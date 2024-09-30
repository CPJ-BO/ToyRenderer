#pragma once

#include "Core/Math/Math.h"
#include <GLFW/glfw3.h>
#include <iostream>

enum KeyType
{
    KEY_TYPE_SPACE =              32,
    KEY_TYPE_APOSTROPHE =         39,  /* ' */
    KEY_TYPE_COMMA =              44,  /* , */
    KEY_TYPE_MINUS =              45,  /* - */
    KEY_TYPE_PERIOD =             46,  /* . */
    KEY_TYPE_SLASH =              47,  /* / */
    KEY_TYPE_0 =                  48,
    KEY_TYPE_1 =                  49,
    KEY_TYPE_2 =                  50,
    KEY_TYPE_3 =                  51,
    KEY_TYPE_4 =                  52,
    KEY_TYPE_5 =                  53,
    KEY_TYPE_6 =                  54,
    KEY_TYPE_7 =                  55,
    KEY_TYPE_8 =                  56,
    KEY_TYPE_9 =                  57,
    KEY_TYPE_SEMICOLON =          59,  /* ; */
    KEY_TYPE_EQUAL =              61,  /* = */
    KEY_TYPE_A =                  65,
    KEY_TYPE_B =                  66,
    KEY_TYPE_C =                  67,
    KEY_TYPE_D =                  68,
    KEY_TYPE_E =                  69,
    KEY_TYPE_F =                  70,
    KEY_TYPE_G =                  71,
    KEY_TYPE_H =                  72,
    KEY_TYPE_I =                  73,
    KEY_TYPE_J =                  74,
    KEY_TYPE_K =                  75,
    KEY_TYPE_L =                  76,
    KEY_TYPE_M =                  77,
    KEY_TYPE_N =                  78,
    KEY_TYPE_O =                  79,
    KEY_TYPE_P =                  80,
    KEY_TYPE_Q =                  81,
    KEY_TYPE_R =                  82,
    KEY_TYPE_S =                  83,
    KEY_TYPE_T =                  84,
    KEY_TYPE_U =                  85,
    KEY_TYPE_V =                  86,
    KEY_TYPE_W =                  87,
    KEY_TYPE_X =                  88,
    KEY_TYPE_Y =                  89,
    KEY_TYPE_Z =                  90,
    KEY_TYPE_LEFT_BRACKET =       91,  /* [ */
    KEY_TYPE_BACKSLASH =          92,  /* \ */
    KEY_TYPE_RIGHT_BRACKET =      93,  /* ] */
    KEY_TYPE_GRAVE_ACCENT =       96,  /* ` */
    KEY_TYPE_WORLD_1 =            161, /* non-US #1 */
    KEY_TYPE_WORLD_2 =            162, /* non-US #2 */

    KEY_TYPE_ESCAPE =             256,
    KEY_TYPE_ENTER =              257,
    KEY_TYPE_TAB =                258,
    KEY_TYPE_BACKSPACE =          259,
    KEY_TYPE_INSERT =             260,
    KEY_TYPE_DELETE =             261,
    KEY_TYPE_RIGHT =              262,
    KEY_TYPE_LEFT =               263,
    KEY_TYPE_DOWN =               264,
    KEY_TYPE_UP =                 265,
    KEY_TYPE_PAGE_UP =            266,
    KEY_TYPE_PAGE_DOWN =          267,
    KEY_TYPE_HOME =               268,
    KEY_TYPE_END =                269,
    KEY_TYPE_CAPS_LOCK =          280,
    KEY_TYPE_SCROLL_LOCK =        281,
    KEY_TYPE_NUM_LOCK =           282,
    KEY_TYPE_PRINT_SCREEN =       283,
    KEY_TYPE_PAUSE =              284,
    KEY_TYPE_F1 =                 290,
    KEY_TYPE_F2 =                 291,
    KEY_TYPE_F3 =                 292,
    KEY_TYPE_F4 =                 293,
    KEY_TYPE_F5 =                 294,
    KEY_TYPE_F6 =                 295,
    KEY_TYPE_F7 =                 296,
    KEY_TYPE_F8 =                 297,
    KEY_TYPE_F9 =                 298,
    KEY_TYPE_F10 =                299,
    KEY_TYPE_F11 =                300,
    KEY_TYPE_F12 =                301,
    KEY_TYPE_F13 =                302,
    KEY_TYPE_F14 =                303,
    KEY_TYPE_F15 =                304,
    KEY_TYPE_F16 =                305,
    KEY_TYPE_F17 =                306,
    KEY_TYPE_F18 =                307,
    KEY_TYPE_F19 =                308,
    KEY_TYPE_F20 =                309,
    KEY_TYPE_F21 =                310,
    KEY_TYPE_F22 =                311,
    KEY_TYPE_F23 =                312,
    KEY_TYPE_F24 =                313,
    KEY_TYPE_F25 =                314,
    KEY_TYPE_KP_0 =               320,
    KEY_TYPE_KP_1 =               321,
    KEY_TYPE_KP_2 =               322,
    KEY_TYPE_KP_3 =               323,
    KEY_TYPE_KP_4 =               324,
    KEY_TYPE_KP_5 =               325,
    KEY_TYPE_KP_6 =               326,
    KEY_TYPE_KP_7 =               327,
    KEY_TYPE_KP_8 =               328,
    KEY_TYPE_KP_9 =               329,
    KEY_TYPE_KP_DECIMAL =         330,
    KEY_TYPE_KP_DIVIDE =          331,
    KEY_TYPE_KP_MULTIPLY =        332,
    KEY_TYPE_KP_SUBTRACT =        333,
    KEY_TYPE_KP_ADD =             334,
    KEY_TYPE_KP_ENTER =           335,
    KEY_TYPE_KP_EQUAL =           336,
    KEY_TYPE_LEFT_SHIFT =         340,
    KEY_TYPE_LEFT_CONTROL =       341,
    KEY_TYPE_LEFT_ALT =           342,
    KEY_TYPE_LEFT_SUPER =         343,
    KEY_TYPE_RIGHT_SHIFT =        344,
    KEY_TYPE_RIGHT_CONTROL =      345,
    KEY_TYPE_RIGHT_ALT =          346,
    KEY_TYPE_RIGHT_SUPER =        347,
    KEY_TYPE_MENU =               348,

    KEY_TYPE_MAX_ENUM =           349 //
};

enum MouseButtonType
{
    MOUSE_BUTTON_TYPE_1 =         0,
    MOUSE_BUTTON_TYPE_2 =         1,
    MOUSE_BUTTON_TYPE_3 =         2,
    MOUSE_BUTTON_TYPE_4 =         3,
    MOUSE_BUTTON_TYPE_5 =         4,
    MOUSE_BUTTON_TYPE_6 =         5,
    MOUSE_BUTTON_TYPE_7 =         6,
    MOUSE_BUTTON_TYPE_8 =         7,
    MOUSE_BUTTON_TYPE_MAX_ENUM,     //

    MOUSE_BUTTON_TYPE_LEFT =      MOUSE_BUTTON_TYPE_1,
    MOUSE_BUTTON_TYPE_RIGHT =     MOUSE_BUTTON_TYPE_2,
    MOUSE_BUTTON_TYPE_MIDDLE =    MOUSE_BUTTON_TYPE_3, 
};

enum InputState
{
    INPUT_STATE_RELEASE = 0,
    INPUT_STATE_PRESS,
    INPUT_STATE_REPEAT,

    INPUT_STATE_MAX_ENUM,   //
};

class InputSystem
{
public:
    InputSystem() {};
    ~InputSystem() {};

    void Init() {};
    void InitGLFW();

    void Tick();

    inline Vec2 GetMousePosition()                      { return { mousePositionX, mousePositionY}; }  // 
    inline Vec2 GetMouseDeltaPosition()                 { return { mouseDeltaX, mouseDeltaY}; }        // 鼠标与上一帧的变化
    inline Vec2 GetScrollDeltaPosition()                { return { scrollDeltaX, scrollDeltaY }; }     // 滚轮与上一帧变化

    bool OnKeyPress(KeyType key)                        { return keyActions[key] == INPUT_STATE_PRESS; }    // 仅当按钮按下的单帧触发
    bool KeyIsPressed(KeyType key)                      { return keyActions[key] != INPUT_STATE_RELEASE; }  // 当按钮按下后，未释放前持续
    bool KeyIsReleased(KeyType key)                     { return keyActions[key] == INPUT_STATE_RELEASE; }  // 按钮未按下

    bool OnMouseButtonPress(MouseButtonType button)     { return mouseButtonActions[button] == INPUT_STATE_PRESS; }    
    bool MouseButtonIsPressed(MouseButtonType button)   { return mouseButtonActions[button] != INPUT_STATE_RELEASE; }  
    bool MouseButtonIsReleased(MouseButtonType button)  { return mouseButtonActions[button] == INPUT_STATE_RELEASE; }    

    // int GetKeyAction(KeyType key);       { keyActions[key]; }
    // int GetKeyMods(KeyType key);         { return keyMods[key]; }    // 是否带了ctrl之类的键

private:
    GLFWwindow* window;

    static float mousePositionPreviousX;
    static float mousePositionPreviousY;
    static float mousePositionX;
    static float mousePositionY;
    static float mouseDeltaX;
    static float mouseDeltaY;

    static float scrollDeltaX;
    static float scrollDeltaY;
    static bool scrollUpdated;

    static int keyActions[KEY_TYPE_MAX_ENUM];
    static int keyMods[KEY_TYPE_MAX_ENUM];
    static bool keyShouldUpdate[KEY_TYPE_MAX_ENUM];

    static int mouseButtonActions[MOUSE_BUTTON_TYPE_MAX_ENUM];
    static int mouseButtonMods[MOUSE_BUTTON_TYPE_MAX_ENUM];
    static bool mouseButtonShouldUpdate[MOUSE_BUTTON_TYPE_MAX_ENUM];

    static void MousePosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        mousePositionX = xpos;
        mousePositionY = ypos;
        //std::cout << "Mouse Pos: " << mousePositionX << " " << mousePositionY << std::endl;
    }

    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        if(button < MOUSE_BUTTON_TYPE_MAX_ENUM)
        {
            mouseButtonActions[button] = action;
            mouseButtonMods[button] = mods;
        }
        //std::cout << "Mouse Button: " << button << " " << action << " " << mods << std::endl;
    }

    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        scrollDeltaX = xoffset;
        scrollDeltaY = yoffset;
        scrollUpdated = true;
        //std::cout << "Scroll : " << scrollDeltaX << " " << scrollDeltaY << std::endl;
    }

    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        //这个回调函数的PRESS和REPEAT之间有延迟，需要手动处理
        if (key < KEY_TYPE_MAX_ENUM)
        {
            keyActions[key] = action;
            keyMods[key] = mods;
        }
        //std::cout << "Key: " << key << " " << action << " " << mods << std::endl;
    }
};

