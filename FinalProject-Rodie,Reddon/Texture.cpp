#include "Texture.h"

Texture::Texture(const char* fileName) {
    glGenTextures(1, &m_TextureID);
    glBindTexture(GL_TEXTURE_2D, m_TextureID);

    if (!loadTexture(fileName)) {
        printf("Texture loading failed: %s\n", fileName);
    }
    else {
        initializeTexture();
    }

    glBindTexture(GL_TEXTURE_2D, 0); // Unbind
}

Texture::Texture() {
    m_TextureID = 0;
    printf("No Texture Data Provided.\n");
}

bool Texture::loadTexture(const char* texFile) {
    m_TextureID = SOIL_load_OGL_texture(texFile, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);

    if (!m_TextureID) {
        printf("Failed to load texture: %s\n", texFile);
        return false;
    }

    return true;
}

bool Texture::initializeTexture() {
    glBindTexture(GL_TEXTURE_2D, m_TextureID);

    // Mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);

    // Texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Wrapping (for repeating textures)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return true;
}



