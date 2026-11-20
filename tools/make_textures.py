"""Generates the room textures into textures/.

All textures are made by this script (our own work, no third-party images),
so they need no licence beyond the project's. Run it again to rebuild them:

    python3 tools/make_textures.py

Needs numpy and Pillow (pip install numpy pillow). The PNGs are committed,
so building the C++ program does not need Python.
"""
import os
import numpy as np
from PIL import Image

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "textures")
rng = np.random.default_rng(7)  # fixed seed: same textures every run


def smooth_noise(h, w, cell, wrap=True):
    """Value noise that tiles seamlessly: random grid, bicubic-ish upscaling."""
    gh, gw = max(1, h // cell), max(1, w // cell)
    grid = rng.random((gh, gw))
    ys = np.arange(h) / cell
    xs = np.arange(w) / cell
    y0 = np.floor(ys).astype(int)
    x0 = np.floor(xs).astype(int)
    fy = ys - y0
    fx = xs - x0
    fy = fy * fy * (3 - 2 * fy)
    fx = fx * fx * (3 - 2 * fx)
    y0 %= gh
    x0 %= gw
    y1 = (y0 + 1) % gh
    x1 = (x0 + 1) % gw
    a = grid[np.ix_(y0, x0)]
    b = grid[np.ix_(y0, x1)]
    c = grid[np.ix_(y1, x0)]
    d = grid[np.ix_(y1, x1)]
    top = a + (b - a) * fx[None, :]
    bot = c + (d - c) * fx[None, :]
    return top + (bot - top) * fy[:, None]


def fbm(h, w, cells):
    total = np.zeros((h, w))
    amp, norm = 1.0, 0.0
    for cell in cells:
        total += amp * smooth_noise(h, w, cell)
        norm += amp
        amp *= 0.5
    return total / norm


def save(name, rgb):
    img = Image.fromarray(np.clip(rgb * 255, 0, 255).astype(np.uint8), "RGB")
    img.save(os.path.join(OUT, name), optimize=True)
    print("wrote", name, img.size)


def wood_floor(size=512, planks=5):
    """1 m x 1 m of floor: 5 planks of 20 cm, joints staggered between planks."""
    h = w = size
    x = np.arange(w)[None, :].repeat(h, 0)
    y = np.arange(h)[:, None].repeat(w, 1)
    plank_w = w / planks
    plank = (x // plank_w).astype(int)

    # long stretched noise = grain running along the plank (vertical in the image)
    grain = fbm(h, w * 8, [64, 32, 16, 8])[:, ::8]
    rings = 0.5 + 0.5 * np.sin(grain * 40.0)

    tone = rng.uniform(0.85, 1.12, planks)[plank]          # each plank a bit different
    base = np.array([0.62, 0.40, 0.23])
    dark = np.array([0.45, 0.27, 0.14])
    t = (0.55 * rings + 0.45 * grain)[..., None]
    rgb = (base * (1 - t) + dark * t) * tone[..., None]

    # gaps between planks (sides) and staggered end joints
    gap = (x % plank_w) < 2.0
    offsets = rng.integers(0, h, planks)[plank]
    end_joint = ((y + offsets) % h) < 2
    rgb[gap | end_joint] *= 0.45
    return rgb


def wall_plaster(size=512):
    """Light, slightly uneven plaster with faint vertical wallpaper stripes.
    Nearly white, so the wall colour (uColor) tints it."""
    h = w = size
    n = fbm(h, w, [128, 64, 16, 4])
    x = np.arange(w)[None, :].repeat(h, 0)
    stripes = 0.5 + 0.5 * np.cos(x / w * 2 * np.pi * 8)    # 8 stripes per metre
    v = 0.86 + 0.10 * n + 0.04 * stripes
    return np.stack([v, v, v], -1)


def wood_grain(size=512):
    """Light wood grain for furniture; tinted by each object's colour."""
    h = w = size
    grain = fbm(h, w * 6, [96, 32, 12])[:, ::6]
    rings = 0.5 + 0.5 * np.sin(grain * 30.0)
    v = 0.78 + 0.22 * (0.6 * rings + 0.4 * grain)
    return np.stack([v * 1.05, v, v * 0.92], -1)


def rug(width=448, height=512):
    """A woven rug with a border and diamond pattern. 448 x 512 has the same
    shape as the 3.5 m x 4 m rug, so it is mapped once without stretching."""
    h, w = height, width
    y, x = np.mgrid[0:h, 0:w]
    u = x / (w - 1)
    v = y / (h - 1)
    edge = np.minimum(np.minimum(u, 1 - u) * 3.5, np.minimum(v, 1 - v) * 4.0)  # metres to edge

    cream = np.array([0.93, 0.86, 0.74])
    red = np.array([0.72, 0.26, 0.28])
    deep = np.array([0.42, 0.13, 0.18])
    mustard = np.array([0.86, 0.64, 0.30])

    rgb = np.empty((h, w, 3))
    rgb[:] = red

    # diamonds in the middle field
    du = (u * 3.5) % 0.5 - 0.25
    dv = (v * 4.0) % 0.5 - 0.25
    diamond = np.abs(du) + np.abs(dv)
    rgb[diamond < 0.16] = deep
    rgb[diamond < 0.08] = mustard

    # border bands
    rgb[edge < 0.38] = cream
    rgb[(edge > 0.12) & (edge < 0.22)] = deep
    rgb[edge < 0.06] = red

    # woven look: fine thread lines and a little noise
    weave = 0.93 + 0.07 * ((x + y) % 4 < 2)
    n = 0.92 + 0.12 * fbm(h, w, [64, 8, 2])
    return rgb * (weave * n)[..., None]


def saturn(size=512):
    """Framed picture of Saturn: starry space, a banded planet lit from the
    upper left, and tilted rings that pass behind and in front of it."""
    h = w = size
    y, x = np.mgrid[0:h, 0:w].astype(float)

    # deep space: dark blue gradient with a faint purple haze and stars
    v = y / h
    rgb = np.empty((h, w, 3))
    rgb[:] = np.array([0.02, 0.03, 0.08])
    rgb += (0.06 * (1 - v))[..., None] * np.array([0.3, 0.2, 0.6])
    haze = fbm(h, w, [128, 64, 32])
    rgb += (0.05 * haze)[..., None] * np.array([0.5, 0.3, 0.8])
    stars = rng.random((h, w))
    bright = rng.random((h, w))
    rgb[stars > 0.9975] = 0.6 + 0.4 * bright[stars > 0.9975][:, None]
    rgb[stars > 0.9995] = 1.0

    cx, cy, R = w * 0.5, h * 0.5, w * 0.2
    tilt = np.radians(-22)                       # ring plane tilt
    dx, dy = x - cx, y - cy
    # coordinates in the ring plane: rx along the rings, ry across them
    rx = dx * np.cos(tilt) - dy * np.sin(tilt)
    ry = dx * np.sin(tilt) + dy * np.cos(tilt)
    squash = 0.28                                # how open the rings look
    rr = np.sqrt(rx ** 2 + (ry / squash) ** 2) / R

    # rings: bands of different brightness, with the dark Cassini gap
    ring_alpha = np.zeros((h, w))
    inside = (rr > 1.25) & (rr < 2.25)
    band = 0.55 + 0.45 * np.sin(rr * 38.0) * np.sin(rr * 7.0)
    ring_alpha[inside] = (0.35 + 0.55 * band[inside])
    ring_alpha[(rr > 1.86) & (rr < 1.95)] = 0.08   # Cassini division
    ring_alpha[(rr > 2.15)] *= np.clip((2.25 - rr[(rr > 2.15)]) / 0.1, 0, 1)
    ring_col = np.array([0.86, 0.77, 0.60])

    # planet: latitude bands, soft noise, lit from the upper left
    d = np.sqrt(dx ** 2 + dy ** 2) / R
    planet = d < 1.0
    lat = ry / R                                 # bands follow the ring plane
    bands = 0.5 + 0.5 * np.sin(lat * 9.0 + 0.6 * fbm(h, w, [64, 16]))
    c1 = np.array([0.93, 0.80, 0.55])
    c2 = np.array([0.74, 0.55, 0.34])
    pcol = c1 * (1 - bands[..., None]) + c2 * bands[..., None]
    nz = np.sqrt(np.clip(1 - d ** 2, 0, 1))      # sphere normal (z part)
    light = np.array([-0.55, -0.45, 0.70])
    light /= np.linalg.norm(light)
    lambert = np.clip((dx / R) * light[0] + (dy / R) * light[1] + nz * light[2], 0, 1)
    shade = 0.12 + 0.95 * lambert

    # far half of the rings is behind the planet, near half is in front
    far = ry < 0
    a = ring_alpha * far
    rgb = rgb * (1 - a[..., None]) + ring_col * a[..., None]
    rgb[planet] = (pcol * shade[..., None])[planet]
    a = ring_alpha * ~far
    # the planet's shadow falls on the rings behind it (lower right side)
    rgb = rgb * (1 - a[..., None]) + ring_col * a[..., None]
    behind_shadow = far & (rr > 1.2) & (rr < 2.25) & (rx > 0) & (np.abs(ry) < R * squash * 1.4) & ~planet
    rgb[behind_shadow] *= 0.45
    return np.clip(rgb, 0, 1)


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    save("wood_floor.png", wood_floor())
    save("wall_plaster.png", wall_plaster())
    save("wood_grain.png", wood_grain())
    save("rug.png", rug())
    save("art_saturn.png", saturn())
