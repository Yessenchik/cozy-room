#pragma once
#include <glad/glad.h>
#include <string>

// A 2D colour texture loaded from an image file with stb_image.
// Mipmaps are generated on load and the texture repeats (GL_REPEAT),
// so a small tile can cover a whole floor without stretching.
class Texture {
public:
    explicit Texture(const std::string& path);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    void bind(int unit) const;
    bool isLoaded() const { return id != 0; }

private:
    GLuint id = 0;
};
