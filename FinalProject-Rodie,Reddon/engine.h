#ifndef ENGINE_H
#define ENGINE_H


#include <assert.h>
#include "window.h"
#include "graphics.h"
#include "gamemode.h"



class Engine
{
public:
    Engine(const char* name, int width, int height);

    ~Engine();
    bool Initialize();
    void Run();
    void ProcessInput();
    unsigned int getDT();
    long long GetCurrentTimeMillis();
    void Display(GLFWwindow*, double);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    Camera* getCamera() { return m_graphics->getCamera(); }

    glm::vec3 cachedCamPos;
    glm::vec3 cachedCamFront;
    glm::vec3 cachedCamUp;
    bool tabPressedLastFrame = false;

    GameMode currentMode = GameMode::Exploration;
    float orbitYaw = 90.0f;    // horizontal angle
    float orbitPitch = 0.0f;   // vertical angle
    float orbitDistance = 5.0f; // fixed zoom distance
    glm::vec3 orbitTarget = glm::vec3(0.0f); // planet center
    int observedPlanetIndex = -1;



private:
    // Window related variables
    Window* m_window;
    const char* m_WINDOW_NAME;
    int m_WINDOW_WIDTH;
    int m_WINDOW_HEIGHT;
    bool m_FULLSCREEN;

    double lastX = 400, lastY = 300;
    bool firstMouse = true;

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;




    Graphics* m_graphics;

    bool m_running;
};

#endif // ENGINE_H
