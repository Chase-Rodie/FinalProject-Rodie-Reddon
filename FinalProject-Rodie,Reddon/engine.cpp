#include "engine.h"
#include "glm/ext.hpp"

Engine::Engine(const char* name, int width, int height)
{
    m_WINDOW_NAME = name;
    m_WINDOW_WIDTH = width;
    m_WINDOW_HEIGHT = height;

}


Engine::~Engine()
{
    delete m_window;
    delete m_graphics;
    m_window = NULL;
    m_graphics = NULL;
}

bool Engine::Initialize()
{
    // Start a window
    m_window = new Window(m_WINDOW_NAME, &m_WINDOW_WIDTH, &m_WINDOW_HEIGHT);
    if (!m_window->Initialize())
    {
        printf("The window failed to initialize.\n");
        return false;
    }

    // Start the graphics
    m_graphics = new Graphics();
    if (!m_graphics->Initialize(m_WINDOW_WIDTH, m_WINDOW_HEIGHT))
    {
        printf("The graphics failed to initialize.\n");
        return false;
    }

    glfwSetCursorPosCallback(m_window->getWindow(), Engine::cursor_position_callback);
    glfwSetWindowUserPointer(m_window->getWindow(), this); // Enable access to Engine instance

    glfwSetScrollCallback(m_window->getWindow(), [](GLFWwindow* window, double xoffset, double yoffset) {
        Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
        if (engine && engine->getCamera())
            engine->getCamera()->ProcessMouseScroll(yoffset);
        });

    return true;
}

void Engine::Run()
{
    m_running = true;

    while (!glfwWindowShouldClose(m_window->getWindow()))
    {
        ProcessInput();
        Display(m_window->getWindow(), glfwGetTime());
        glfwPollEvents();
    }
    m_running = false;

}

void Engine::ProcessInput()
{
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    GLFWwindow* win = m_window->getWindow();
    Camera* cam = m_graphics->getCamera();

    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(win, true);

    float camSpeed = 5.0f * deltaTime;

    //observation mode
    if (currentMode == GameMode::Observation) {

        if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
            cam->ProcessKeyboard("FORWARD", camSpeed);
        if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
            cam->ProcessKeyboard("BACKWARD", camSpeed);
        if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
            cam->ProcessKeyboard("LEFT", camSpeed);
        if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
            cam->ProcessKeyboard("RIGHT", camSpeed);
    }
    //exploration mode
    else if (currentMode == GameMode::Exploration) {
        Mesh* ship = m_graphics->getMesh();
        float shipSpeed = 65.0f * deltaTime;
        if (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            shipSpeed *= 3.0f;

        // Movement
        if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
            ship->MoveForward(shipSpeed);
        if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
            ship->MoveForward(-shipSpeed);

        // Rotation
        if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
            ship->Rotate(0.0f, 60.0f * deltaTime, 0.0f); // yaw left
        if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
            ship->Rotate(0.0f, -60.0f * deltaTime, 0.0f); // yaw right

        if (glfwGetKey(win, GLFW_KEY_Q) == GLFW_PRESS)
            ship->Rotate(0.0f, 0.0f, 60.0f * deltaTime); // roll left
        if (glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS)
            ship->Rotate(0.0f, 0.0f, -60.0f * deltaTime); // roll right

        if (glfwGetKey(win, GLFW_KEY_UP) == GLFW_PRESS)
            ship->Rotate(-60.0f * deltaTime, 0.0f, 0.0f); // pitch up
        if (glfwGetKey(win, GLFW_KEY_DOWN) == GLFW_PRESS)
            ship->Rotate(60.0f * deltaTime, 0.0f, 0.0f); // pitch down

        if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS)
            ship->Brake(); // brake to halt
    }

    bool tabPressed = glfwGetKey(win, GLFW_KEY_TAB) == GLFW_PRESS;

    if (glfwGetKey(win, GLFW_KEY_TAB) == GLFW_PRESS) {
        if (tabPressed && !tabPressedLastFrame) {
            tabPressedLastFrame = true;

            if (currentMode == GameMode::Exploration) {
                currentMode = GameMode::Observation;
                std::cout << "Switched to Observation Mode\n";
                cachedCamPos = cam->cameraPos;
                cachedCamFront = cam->cameraFront;
                cachedCamUp = cam->cameraUp;

                // Find closest planet to ship
                glm::mat4 shipModel = m_graphics->GetStarshipModelMatrix();
                glm::vec3 shipPos = glm::vec3(shipModel[3]);

                // Find and store index of closest planet
                float minDist = std::numeric_limits<float>::max();
                for (int i = 0; i < planetSpheres.size(); ++i) {
                    glm::vec3 planetPos = glm::vec3(planetSpheres[i]->GetModel()[3]);
                    float dist = glm::distance(shipPos, planetPos);
                    if (dist < minDist) {
                        minDist = dist;
                        observedPlanetIndex = i;
                    }
                }

                orbitDistance = 5.0f; // or scale based on planet

                // Place camera in front of ship, looking at closest planet
                glm::vec3 forward = glm::normalize(glm::vec3(shipModel[2]));
                glm::vec3 cameraPos = shipPos + forward * 2.5f;

                cam->SetPosition(cameraPos);
                cam->FaceDirection(orbitTarget);




                firstMouse = true;

            }
            else {
                currentMode = GameMode::Exploration;
                std::cout << "Switched to Exploration Mode\n";


                Camera* cam = m_graphics->getCamera();
                cam->SetPosition(cachedCamPos);
                cam->cameraFront = cachedCamFront;
                cam->cameraUp = cachedCamUp;
                cam->UpdateView();

                firstMouse = true;
            }
        }
    }
    if (!tabPressed) {
        tabPressedLastFrame = false;
    }



}




unsigned int Engine::getDT()
{
    
    return glfwGetTime();
}

long long Engine::GetCurrentTimeMillis()
{
    
    return (long long)glfwGetTime();
}

void Engine::Display(GLFWwindow* window, double time) {

    m_graphics->SetGameMode(currentMode);
    m_graphics->Render();
    m_window->Swap();
    m_graphics->HierarchicalUpdate2(deltaTime);

    if (currentMode == GameMode::Exploration) {
        glm::mat4 shipModel = m_graphics->GetStarshipModelMatrix();
        glm::vec3 shipPos = glm::vec3(shipModel[3]);
        glm::vec3 forward = glm::normalize(glm::vec3(shipModel[2]));
        glm::vec3 up = glm::normalize(glm::vec3(shipModel[1]));

        float distanceBack = 2.5f;
        float distanceUp = 0.5f;
        glm::vec3 camWorldPos = shipPos - forward * distanceBack + up * distanceUp;

        Camera* cam = m_graphics->getCamera();
        cam->SetPosition(camWorldPos);
        cam->SetTarget(shipPos);
        cam->SetUp(up);

    }
    else {
        if (observedPlanetIndex >= 0 && observedPlanetIndex < planetSpheres.size()) {
            orbitTarget = glm::vec3(planetSpheres[observedPlanetIndex]->GetModel()[3]);
        }

        float yawRad = glm::radians(orbitYaw);
        float pitchRad = glm::radians(orbitPitch);

        glm::vec3 offset;
        offset.x = orbitDistance * cos(pitchRad) * cos(yawRad);
        offset.y = orbitDistance * sin(pitchRad);
        offset.z = orbitDistance * cos(pitchRad) * sin(yawRad);
        Camera* cam = m_graphics->getCamera();
        glm::vec3 camPos = orbitTarget + offset;
        cam->SetLookAt(camPos, orbitTarget, glm::vec3(0, 1, 0));
    }
}

void Engine::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    Camera* cam = engine->m_graphics->getCamera();

    if (engine->firstMouse)
    {
        engine->lastX = xpos;
        engine->lastY = ypos;
        engine->firstMouse = false;
        return;
    }

    float xoffset = xpos - engine->lastX;
    float yoffset = engine->lastY - ypos; // reversed: y-coordinates go from bottom to top

    engine->lastX = xpos;
    engine->lastY = ypos;
    if (engine->currentMode == GameMode::Observation) {
        engine->orbitYaw += xoffset * 0.2f;
        engine->orbitPitch -= yoffset * 0.2f;

        // Clamp pitch to avoid flipping
        if (engine->orbitPitch > 89.0f) engine->orbitPitch = 89.0f;
        if (engine->orbitPitch < -89.0f) engine->orbitPitch = -89.0f;
    }

    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        cam->ProcessMouseMovement(xoffset, yoffset);  // Only rotate while dragging
    }
}
