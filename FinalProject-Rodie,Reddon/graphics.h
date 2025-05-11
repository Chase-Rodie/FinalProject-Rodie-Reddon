#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <iostream>
#include <stack>
#include <vector>
using namespace std;

#include "graphics_headers.h"
#include "camera.h"
#include "shader.h"
#include "object.h"
#include "sphere.h"
#include "mesh.h"

#define numVBOs 2;
#define numIBs 2;

struct Comet {
    Sphere* body;
    float orbitRadiusA;   
    float orbitRadiusB;   
    float speed;
    float scale;
    float rotation;       
};


struct CelestialBody {
    std::string name;
    float orbitRadius;
    float orbitSpeed;
    float rotationSpeed;
    float scale;
    float axialTilt;
    std::string texturePath;
};

extern std::vector<CelestialBody> planets;
extern std::vector<Sphere*> planetSpheres;

struct Moon {
    int parentPlanetIndex;
    Sphere* sphere;
    float orbitRadius;
    float orbitSpeed;
    float scale;
    float tilt;
    std::string texturePath;
};

extern std::vector<Moon> moons;
extern Comet halleysComet;



class Graphics
{
public:
    Graphics();
    ~Graphics();
    bool Initialize(int width, int height);
    void HierarchicalUpdate2(double dt);
    void Render();
    void GenerateAsteroidBelts();
    glm::mat4 GetStarshipModelMatrix() const;
    void SetupAsteroidInstancing();
  

    Camera* getCamera() { return m_camera; }
    Mesh* getMesh() { return m_mesh; }
    void RenderCometTail(const glm::vec3& cometPos, const glm::vec3& sunPos);


private:
    std::string ErrorString(GLenum error);

    bool collectShPrLocs();
    void ComputeTransforms(double dt, std::vector<float> speed, std::vector<float> dist,
        std::vector<float> rotSpeed, glm::vec3 rotVector, std::vector<float> scale,
        glm::mat4& tmat, glm::mat4& rmat, glm::mat4& smat);
    GLuint loadCubemap(std::vector<std::string> faces);

    stack<glm::mat4> modelStack;

    Camera* m_camera;
    Shader* m_shader;
    Mesh* m_mesh;
    Mesh* m_asteroid;

    std::vector<glm::mat4> innerAsteroidTransforms;
    std::vector<glm::mat4> outerAsteroidTransforms;


    GLint m_projectionMatrix;
    GLint m_viewMatrix;
    GLint m_modelMatrix;
    GLint m_positionAttrib;
    GLint m_colorAttrib;
    GLint m_tcAttrib;
    GLint m_hasTexture;
    GLuint innerAsteroidVBO, outerAsteroidVBO;

    double totalTime = 0.0; 


    GLint overrideColorLoc;


    glm::vec3 currentCometPosition = glm::vec3(0.0f);
    glm::vec3 previousCometPosition = glm::vec3(0.0f);
    glm::vec3 cometVelocity = glm::vec3(0.0f);

    std::deque<glm::vec3> cometTrailPositions;  
    const size_t maxTrailLength = 50;           




    Sphere* m_sphere;
    Sphere* m_sphere2;
   
    //Sphere* m_sphere3;

   



    // Skybox members
    GLuint skyboxVAO, skyboxVBO;
    GLuint cubemapTexture;
    Shader* skyboxShader;
};

#endif /* GRAPHICS_H */


