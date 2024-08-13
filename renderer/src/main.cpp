

#include "Test/TestAsset.h"
#include "Test/TestComponent.h"
#include "Test/TestHAL.h"
#include "Test/TestMath.h"
#include "Test/TestRDG.h"
#include "Test/TestRHI.h"
#include "Test/TestSystem.h"

Extent2D windowsExtent = { 2048, 1152 };
Offset2D windowsOffset = { 2048, 1152 };
TextureFormat colorFormat = FORMAT_R8G8B8A8_UNORM;
TextureFormat depthFormat = FORMAT_D24_UNORM_S8_UINT;
uint32_t framesInFlight = 3;
std::string shaderPath = "resource/build_in/shader/";

GLFWwindow* InitWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(windowsExtent.width, windowsExtent.height, "Toy", nullptr, nullptr); 

    // glfwSetWindowUserPointer(m_window, this);
    // glfwSetWindowSizeCallback(m_window, nullptr); //TODO
    // glfwSetCursorPosCallback(m_window, MousePosCallback);
    // glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
    // glfwSetScrollCallback(m_window, ScrollCallback);
    // glfwSetKeyCallback(m_window, KeyCallback);

    return window;
}

int main() 
{
    GLFWwindow* window = InitWindow();
    EngineContext::Init();

    // TestMath();
    // TestSystem();
    // TestAsset();
    // TestComponent();
    // TestHAL();
    // TestRHI(window);
    TestRDG(window);

    EngineContext::Destroy();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}



