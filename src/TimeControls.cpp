#include "TimeControls.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdio>
#include <string>

namespace {

// game hours per real second; 0.2 h/s = one day in 2 minutes
const float SPEEDS[] = {0.05f, 0.1f, 0.2f, 0.5f, 1.0f, 2.0f, 4.0f};
const int SPEED_COUNT = sizeof(SPEEDS) / sizeof(SPEEDS[0]);
const int DEFAULT_SPEED = 2;

const float SCRUB_SPEED = 3.0f;      // hours per second while an arrow key is held
const float TIMELAPSE_START = 7.0f;
const float TIMELAPSE_END = 23.0f;
const float TIMELAPSE_SPEED = 1.0f;  // 07:00 -> 23:00 in 16 seconds

const glm::vec4 PANEL(0.05f, 0.05f, 0.08f, 0.60f);
const glm::vec4 WHITE(1.0f, 1.0f, 1.0f, 1.0f);
const glm::vec4 SOFT(0.85f, 0.87f, 0.92f, 1.0f);
const glm::vec4 ACCENT(1.0f, 0.80f, 0.45f, 1.0f);

std::string formatClock(float hour) {
    int minutes = (int)(hour * 60.0f) % (24 * 60);
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%02d:%02d", minutes / 60, minutes % 60);
    return buf;
}

}  // namespace

TimeControls::TimeControls(DayNight& dayNight) : dayNight(dayNight), speedLevel(DEFAULT_SPEED) {
    dayNight.setSpeed(SPEEDS[speedLevel]);
}

void TimeControls::jumpTo(float hour) {
    dayNight.setHour(hour);
    timeLapse = false;
    jumped = true;
}

void TimeControls::changeSpeed(int step) {
    speedLevel = glm::clamp(speedLevel + step, 0, SPEED_COUNT - 1);
    dayNight.setSpeed(SPEEDS[speedLevel]);
    timeLapse = false;
}

bool TimeControls::takeJump() {
    bool result = jumped;
    jumped = false;
    return result;
}

void TimeControls::onKey(int key, int action) {
    if (action != GLFW_PRESS)
        return;

    switch (key) {
    case GLFW_KEY_SPACE:
        dayNight.setPaused(!dayNight.isPaused());
        break;
    case GLFW_KEY_EQUAL:        // the "+" key without Shift
    case GLFW_KEY_KP_ADD:
        changeSpeed(+1);
        break;
    case GLFW_KEY_MINUS:
    case GLFW_KEY_KP_SUBTRACT:
        changeSpeed(-1);
        break;
    case GLFW_KEY_1: jumpTo(8.0f);  break;
    case GLFW_KEY_2: jumpTo(12.0f); break;
    case GLFW_KEY_3: jumpTo(17.5f); break;
    case GLFW_KEY_4: jumpTo(23.0f); break;
    case GLFW_KEY_T:
        jumpTo(TIMELAPSE_START);
        timeLapse = true;
        dayNight.setPaused(false);
        dayNight.setSpeed(TIMELAPSE_SPEED);
        break;
    case GLFW_KEY_H:
        showHelp = !showHelp;
        break;
    default:
        break;
    }
}

void TimeControls::update(GLFWwindow* window, float deltaTime) {
    float scrub = 0.0f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) scrub += SCRUB_SPEED * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  scrub -= SCRUB_SPEED * deltaTime;
    if (scrub != 0.0f) {
        // forward scrubbing behaves like normal time (the lamp reacts on its own),
        // going backwards is treated as a jump
        dayNight.setHour(dayNight.getHour() + scrub);
        if (scrub < 0.0f)
            jumped = true;
    }

    // the time-lapse stops by itself at night and goes back to the normal speed
    if (timeLapse && dayNight.getHour() >= TIMELAPSE_END) {
        timeLapse = false;
        dayNight.setSpeed(SPEEDS[speedLevel]);
    }
}

TimeControls::Box TimeControls::sliderBox(int windowWidth, int windowHeight) {
    const float margin = 40.0f;
    return {margin, windowHeight - 46.0f, windowWidth - 2.0f * margin, 14.0f};
}

void TimeControls::setHourFromMouse(double x, int windowWidth, int windowHeight) {
    Box box = sliderBox(windowWidth, windowHeight);
    float t = glm::clamp(((float)x - box.x) / box.w, 0.0f, 0.9999f);
    jumpTo(t * 24.0f);
}

bool TimeControls::onMouseButton(int button, int action, double x, double y,
                                 int windowWidth, int windowHeight) {
    if (button != GLFW_MOUSE_BUTTON_LEFT)
        return false;

    if (action == GLFW_RELEASE) {
        bool wasDragging = draggingSlider;
        draggingSlider = false;
        return wasDragging;
    }

    // a slightly bigger area than the bar itself, so it is easy to grab
    Box box = sliderBox(windowWidth, windowHeight);
    bool inside = x >= box.x - 8 && x <= box.x + box.w + 8 && y >= box.y - 10 && y <= box.y + box.h + 10;
    if (inside) {
        draggingSlider = true;
        setHourFromMouse(x, windowWidth, windowHeight);
    }
    return inside;
}

void TimeControls::onMouseMove(double x, double, int windowWidth, int windowHeight) {
    if (draggingSlider)
        setHourFromMouse(x, windowWidth, windowHeight);
}

void TimeControls::draw(Hud& hud, int windowWidth, int windowHeight, float fps, bool lampOn) const {
    const float pad = 12.0f;

    // --- status panel (top left) -------------------------------------------
    std::string clock = formatClock(dayNight.getHour());
    std::string phase = dayNight.getPhaseName();

    char speedLine[64];
    if (dayNight.isPaused())
        std::snprintf(speedLine, sizeof(speedLine), "Time: PAUSED");
    else if (timeLapse)
        std::snprintf(speedLine, sizeof(speedLine), "Time-lapse: 1 h per second");
    else
        std::snprintf(speedLine, sizeof(speedLine), "Speed: 1 day = %.1f min", 24.0f / dayNight.getSpeed() / 60.0f);

    char infoLine[64];
    std::snprintf(infoLine, sizeof(infoLine), "Lamp: %s     FPS: %.0f", lampOn ? "on" : "off", fps);

    float clockScale = 4.0f, textScale = 2.0f;
    float panelW = Hud::textWidth(clock, clockScale) + 14.0f + Hud::textWidth(phase, textScale);
    panelW = glm::max(panelW, Hud::textWidth(speedLine, textScale));
    panelW = glm::max(panelW, Hud::textWidth(infoLine, textScale));
    panelW = glm::max(panelW + pad * 2, 260.0f);
    float panelH = pad * 2 + Hud::lineHeight(clockScale) + Hud::lineHeight(textScale) * 3;
    hud.rect(pad, pad, panelW, panelH, PANEL);

    float x = pad * 2, y = pad * 2;
    hud.textWithShadow(x, y, clock, clockScale, WHITE);
    hud.textWithShadow(x + Hud::textWidth(clock, clockScale) + 14.0f, y + 12.0f, phase, textScale, ACCENT);
    y += Hud::lineHeight(clockScale);
    hud.textWithShadow(x, y, speedLine, textScale, SOFT);
    y += Hud::lineHeight(textScale);
    hud.textWithShadow(x, y, infoLine, textScale, SOFT);
    y += Hud::lineHeight(textScale);
    hud.textWithShadow(x, y, showHelp ? "H - hide help" : "H - show help", textScale, SOFT);

    // --- help panel (top right) ---------------------------------------------
    if (showHelp) {
        // the font is not monospaced, so keys and actions are drawn as two columns
        struct HelpLine { const char* keys; const char* action; };
        const HelpLine lines[] = {
            {"CAMERA", ""},
            {"Right mouse drag", "orbit"},
            {"Scroll, W / S", "zoom"},
            {"A / D, Q / E", "orbit with keys"},
            {"", ""},
            {"TIME OF DAY", ""},
            {"Space", "pause / resume"},
            {"- / +", "slower / faster"},
            {"Left / Right", "scrub time (hold)"},
            {"1  2  3  4", "morning / noon / sunset / night"},
            {"T", "time-lapse: morning to night"},
            {"Left mouse on bar", "drag the time"},
            {"", ""},
            {"SCENE", ""},
            {"L", "desk lamp on / off"},
            {"B", "FPS benchmark (20 s)"},
            {"V", "vsync on / off"},
            {"H", "show / hide this help"},
            {"Esc", "quit"},
        };
        const int lineCount = sizeof(lines) / sizeof(lines[0]);
        const float helpScale = 1.5f;

        float keysW = 0.0f, actionW = 0.0f;
        for (const HelpLine& line : lines) {
            if (line.action[0] == '\0') continue;   // headers do not set the column width
            keysW = glm::max(keysW, Hud::textWidth(line.keys, helpScale));
            actionW = glm::max(actionW, Hud::textWidth(line.action, helpScale));
        }
        float gap = 16.0f;
        float w = pad * 2 + keysW + gap + actionW;
        float h = pad * 2 + Hud::lineHeight(helpScale) * lineCount;
        float hx = windowWidth - w - pad;
        hud.rect(hx, pad, w, h, PANEL);

        float ly = pad * 2;
        for (const HelpLine& line : lines) {
            bool header = line.action[0] == '\0';
            hud.textWithShadow(hx + pad, ly, line.keys, helpScale, header ? ACCENT : WHITE);
            hud.textWithShadow(hx + pad + keysW + gap, ly, line.action, helpScale, SOFT);
            ly += Hud::lineHeight(helpScale);
        }
    }

    // --- time slider (bottom) -----------------------------------------------
    Box box = sliderBox(windowWidth, windowHeight);
    hud.rect(box.x - 10, box.y - 12, box.w + 20, box.h + 40, PANEL);

    // the bar is painted with the sky colour of every hour, so the whole day is visible
    const int segments = 96;
    for (int i = 0; i < segments; ++i) {
        float hour = (i + 0.5f) * 24.0f / segments;
        glm::vec3 sky = DayNight::lightingAt(hour).sky;
        hud.rect(box.x + box.w * i / segments, box.y, box.w / segments + 1.0f, box.h, glm::vec4(sky, 1.0f));
    }

    for (int hour = 0; hour <= 24; hour += 3) {
        float tx = box.x + box.w * hour / 24.0f;
        hud.rect(tx - 1.0f, box.y + box.h, 2.0f, 5.0f, SOFT);
        std::string label = std::to_string(hour);
        hud.text(tx - Hud::textWidth(label, 1.5f) / 2.0f, box.y + box.h + 7.0f, label, 1.5f, SOFT);
    }

    float mx = box.x + box.w * dayNight.getHour() / 24.0f;
    hud.rect(mx - 4.0f, box.y - 6.0f, 8.0f, box.h + 12.0f, glm::vec4(0.0f, 0.0f, 0.0f, 0.8f));
    hud.rect(mx - 2.0f, box.y - 4.0f, 4.0f, box.h + 8.0f, WHITE);
}
