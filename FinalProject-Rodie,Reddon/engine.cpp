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



    // No errors
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

        if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
            ship->MoveForward(shipSpeed);
        if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
            ship->MoveForward(-shipSpeed);
        if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
            ship->Rotate(0.0f, 60.0f * deltaTime, 0.0f); // turn left
        if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
            ship->Rotate(0.0f, -60.0f * deltaTime, 0.0f); // turn right

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

                glm::mat4 shipModel = m_graphics->GetStarshipModelMatrix();
                glm::vec3 shipPos = glm::vec3(shipModel[3]);
                glm::vec3 shipForward = glm::normalize(glm::vec3(shipModel[2]));

                glm::vec3 cameraPos = shipPos + shipForward * 2.5f; // move 0.5 units in front of ship
                glm::vec3 planetPos = glm::vec3(5.0f, 0.0f, 0.0f);

                cam->SetPosition(cameraPos);
                cam->FaceDirection(planetPos);



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


//void Engine::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
//{

  //  glfwGetCursorPos(window, &xpos, &ypos);
    //std::cout << "Position: (" << xpos << ":" << ypos << ")";
//}

unsigned int Engine::getDT()
{
    //long long TimeNowMillis = GetCurrentTimeMillis();
    //assert(TimeNowMillis >= m_currentTimeMillis);
    //unsigned int DeltaTimeMillis = (unsigned int)(TimeNowMillis - m_currentTimeMillis);
    //m_currentTimeMillis = TimeNowMillis;
    //return DeltaTimeMillis;
    return glfwGetTime();
}

long long Engine::GetCurrentTimeMillis()
{
    //timeval t;
    //gettimeofday(&t, NULL);
    //long long ret = t.tv_sec * 1000 + t.tv_usec / 1000;
    //return ret;
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
        cam->ProcessMouseMovement(xoffset, yoffset);  // Always rotate with mouse in this mode
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        cam->ProcessMouseMovement(xoffset, yoffset);  // Only rotate while dragging
    }
}
