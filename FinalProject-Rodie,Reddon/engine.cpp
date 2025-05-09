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

    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
        cam->ProcessKeyboard("FORWARD", camSpeed);
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
        cam->ProcessKeyboard("BACKWARD", camSpeed);
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
        cam->ProcessKeyboard("LEFT", camSpeed);
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
        cam->ProcessKeyboard("RIGHT", camSpeed);
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

    m_graphics->Render();
    m_window->Swap();
    m_graphics->HierarchicalUpdate2(time);
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
    }

    float xoffset = xpos - engine->lastX;
    float yoffset = engine->lastY - ypos; // reversed: y-coordinates go from bottom to top

    engine->lastX = xpos;
    engine->lastY = ypos;

    cam->ProcessMouseMovement(xoffset, yoffset);
}

