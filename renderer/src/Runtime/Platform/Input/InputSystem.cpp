#include "InputSystem.h"
#include "Function/Global/EngineContext.h"

float InputSystem::mousePositionPreviousX = 0;
float InputSystem::mousePositionPreviousY = 0;
float InputSystem::mousePositionX = 0;
float InputSystem::mousePositionY = 0;
float InputSystem::mouseDeltaX = 0;
float InputSystem::mouseDeltaY = 0;

float InputSystem::scrollDeltaX = 0;
float InputSystem::scrollDeltaY = 0;
bool InputSystem::scrollUpdated = 0;

int InputSystem::keyActions[KEY_TYPE_MAX_ENUM] = { 0 };
int InputSystem::keyMods[KEY_TYPE_MAX_ENUM] = { 0 };
bool InputSystem::keyShouldUpdate[KEY_TYPE_MAX_ENUM] = { false };

int InputSystem::mouseButtonActions[MOUSE_BUTTON_TYPE_MAX_ENUM] = { 0 };
int InputSystem::mouseButtonMods[MOUSE_BUTTON_TYPE_MAX_ENUM] = { 0 };
bool InputSystem::mouseButtonShouldUpdate[MOUSE_BUTTON_TYPE_MAX_ENUM] = { false };

void InputSystem::InitGLFW()
{
    window = EngineContext::Render()->GetWindow();

    // glfwSetWindowUserPointer(window, this);
    // glfwSetWindowSizeCallback(window, nullptr); //TODO
    glfwSetCursorPosCallback(window, MousePosCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetKeyCallback(window, KeyCallback);
}

void InputSystem::Tick() 
{
    ENGINE_TIME_SCOPE(InputSystem::Tick);

    mousePositionPreviousX = mousePositionX;
    mousePositionPreviousY = mousePositionY;

    glfwPollEvents(); // 更新回调函数

    mouseDeltaX = (float)mousePositionX - mousePositionPreviousX;
    mouseDeltaY = (float)mousePositionY - mousePositionPreviousY;

    // 只持续一帧
    if(scrollUpdated)   scrollUpdated = false;      
    else {
        scrollDeltaX = 0;
        scrollDeltaY = 0;
    }

    // 让press只持续一帧，后续全为repeat
    for (int i = 0; i < KEY_TYPE_MAX_ENUM; i++)
    {
        if (keyShouldUpdate[i])
        {
            keyActions[i] = INPUT_STATE_REPEAT;
            keyShouldUpdate[i] = false;
        }
    }
    for (int i = 0; i < KEY_TYPE_MAX_ENUM; i++) 
    {
        if (keyActions[i] == INPUT_STATE_PRESS) keyShouldUpdate[i] = true;
    }

    // 让press只持续一帧，后续全为repeat
    for (int i = 0; i < MOUSE_BUTTON_TYPE_MAX_ENUM; i++)
    {
        if (mouseButtonShouldUpdate[i])
        {
            mouseButtonActions[i] = INPUT_STATE_REPEAT;
            mouseButtonShouldUpdate[i] = false;
        }
    }
    for (int i = 0; i < MOUSE_BUTTON_TYPE_MAX_ENUM; i++) 
    {
        if (mouseButtonActions[i] == INPUT_STATE_PRESS) mouseButtonShouldUpdate[i] = true;
    }
}
