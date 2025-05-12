#ifndef MESH_H
#define MESH_H

#include <vector>
#include "graphics_headers.h"
#include "Texture.h"

class Mesh
{
public:
    Mesh();
    Mesh(glm::vec3 pivot, const char* fname);
    Mesh(glm::vec3 pivot, const char* fname, const char* tname);

    ~Mesh();
    void Update(glm::mat4 model);
    void Render(GLint posAttrib, GLint colAttrib);
    void Render(GLint positionAttribLoc, GLint colorAttribLoc, GLint tcAttribLoc, GLint hasTex);
    void Rotate(float 
        , float yaw, float roll);
    void MoveForward(float amount);
    void Brake();
    glm::mat4 GetModel();

    bool InitBuffers();
    bool loadModelFromFile(const char* path);

    bool hasTex;
    GLuint getIBO() const { return IB; }
    GLuint getTextureID() { return m_texture->getTextureID(); }
    GLuint getVAO() const { return vao; }
    int GetIndexCount() const { return Indices.size(); }


private:
    glm::vec3 pivotLocation;
    glm::mat4 model;
    std::vector<Vertex> Vertices;
    std::vector<unsigned int> Indices;
    GLuint VB;
    GLuint IB;

    Texture* m_texture;

    GLuint vao;

    float angle;
};

#endif