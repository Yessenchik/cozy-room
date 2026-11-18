#include "Texture.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Texture::Texture(const std::string& path) {
    int width, height, channels;
    // OpenGL expects the first row at the bottom of the image
    stbi_set_flip_vertically_on_load(true);
    unsigned char* pixels = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!pixels) {
        std::cerr << "Failed to load texture " << path << ": " << stbi_failure_reason() << "\n";
        return;
    }

    GLenum format = channels == 4 ? GL_RGBA : channels == 1 ? GL_RED : GL_RGB;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    // rows of an RGB image are not always 4-byte aligned
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // trilinear filtering: smooth between mip levels, no shimmering at grazing angles
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(pixels);
}

Texture::~Texture() {
    if (id)
        glDeleteTextures(1, &id);
}

void Texture::bind(int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id);
    glActiveTexture(GL_TEXTURE0);
}
