/**
 * @file input.cpp
 * @brief Implementation of mouse, keyboard, and gamepad input handling
 */

#include "input.hpp"
#include "globals.hpp"
#include "dear-imgui/imgui.h"
//#include "imgui.h"


using namespace tmt::input;


// Current active input method (keyboard/mouse or gamepad)
static InputState currentInputState = KeyboardMouse;
// Whether input state has been manually forced (disables auto-switching)
static bool forcedInputState = false;

glm::vec2 Mouse::GetMousePosition() { return mousep; }

glm::vec2 Mouse::GetMouseDelta() { return mousedelta; }

glm::vec2 Mouse::GetMouseScroll() { return mousescrl; }

/**
 * @brief Convert screen coordinates to a world-space ray direction
 * 
 * This function performs the reverse of the rendering pipeline:
 * 1. Converts mouse position to normalized device coordinates (NDC) [-1, 1]
 * 2. Transforms NDC to clip space
 * 3. Transforms clip space to eye/camera space
 * 4. Transforms eye space to world space
 * 
 * @param mouseX Mouse X position in screen coordinates
 * @param mouseY Mouse Y position in screen coordinates
 * @param screenWidth Screen width in pixels
 * @param screenHeight Screen height in pixels
 * @param viewMatrix Camera view matrix
 * @param projectionMatrix Camera projection matrix
 * @return glm::vec3 Normalized ray direction in world space
 */
glm::vec3 ScreenToWorldRay(float mouseX, float mouseY, int screenWidth, int screenHeight, const glm::mat4& viewMatrix,
                           const glm::mat4& projectionMatrix)
{
    // Convert mouse position to normalized device coordinates (NDC)
    // NDC range is [-1, 1] where (0,0) is center, (-1,-1) is bottom-left, (1,1) is top-right
    float x = 1.0f - (2.0f * mouseX) / static_cast<float>(screenWidth);
    float y = 1.0f - (2.0f * mouseY) / static_cast<float>(screenHeight);
    float z = 1.0f;  // Far plane


    glm::vec3 rayNDC(x, y, z);

    // Convert NDC to clip coordinates (add w component)
    glm::vec4 rayClip(rayNDC, 1.0f);

    // Convert clip coordinates to eye/camera coordinates
    glm::vec4 rayEye = glm::inverse(projectionMatrix) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);  // Direction vector (w=0)

    // Convert eye coordinates to world coordinates
    auto rayWorld = glm::vec3(glm::inverse(viewMatrix) * rayEye);
    rayWorld = normalize(rayWorld);

    return rayWorld;
}

/**
 * @brief Cast a ray from the camera through the mouse cursor into the scene
 * 
 * Helper function that combines screen-to-world conversion with raycasting.
 * 
 * @param mouseX Mouse X position in screen coordinates
 * @param mouseY Mouse Y position in screen coordinates  
 * @param screenWidth Screen width in pixels
 * @param screenHeight Screen height in pixels
 * @param viewMatrix Camera view matrix
 * @param projectionMatrix Camera projection matrix
 * @return glm::vec3 Ray direction in world space
 */
glm::vec3 RaycastFromMouse(float mouseX, float mouseY, int screenWidth, int screenHeight, const glm::mat4& viewMatrix,
                           const glm::mat4& projectionMatrix)
{
    glm::vec3 rayDirection = ScreenToWorldRay(mouseX, mouseY, screenWidth, screenHeight, viewMatrix, projectionMatrix);


    // Now you can use rayDirection to perform raycasting in your scene
    // For example, intersecting with objects in the scene

    return rayDirection;
}


/**
 * @brief Get mouse position in world space via raycasting
 * 
 * Casts a ray from the camera through the mouse cursor and returns the
 * first intersection point with the physics world (up to 1000 units away).
 * 
 * Note: Returns (0,0,0) if no hit, which could be ambiguous since that's
 * a valid world coordinate. Consider checking the return value carefully.
 * 
 * @param camera Camera to use for raycasting
 * @return glm::vec3 World position where ray hits, or (0,0,0) if no hit
 */
glm::vec3 Mouse::GetWorldMousePosition(render::Camera* camera)
{

    // Get ray direction from camera through mouse cursor
    var dir = RaycastFromMouse(mousep.x, mousep.y, renderer->windowWidth, renderer->windowHeight, camera->GetView_m4(),
                               camera->GetProjection_m4());

    // Debug visualization (uncomment to see the ray)
    // debug::Gizmos::DrawSphere(camera->position + dir, 0.1f);

    var pos = camera->position;

    // Create physics ray starting at camera position
    var ray = physics::Ray{pos, dir, 1000};

    // Cast ray and check for hits
    var hit = ray.Cast();

    if (hit)
    {
        return hit->point;
    }

    // No hit, return origin (note: this is ambiguous with actual (0,0,0) hits)
    return glm::vec3{0};
}

/**
 * @brief Get mouse button state with press/hold detection
 * 
 * Tracks button state over frames to distinguish between:
 * - Press: Button just pressed this frame
 * - Hold: Button held down for multiple frames
 * - Release: Button not pressed
 * 
 * @param i Mouse button to query
 * @param real If true, ignores UI hover state (for non-game interactions)
 * @return MouseButtonState Current button state
 */
Mouse::MouseButtonState Mouse::GetMouseButton(MouseButton i, bool real)
{
    // Get raw button state from GLFW
    int state = glfwGetMouseButton(renderer->window, i);

    if (!real)
    {
        // Optional: Filter input when hovering over UI elements
        /*
        if (ImGui::IsAnyItemHovered() || ImGui::IsAnyItemFocused() || ImGui::IsAnyItemActive())
            state = Release;
            */
    }

    // Convert Press to Hold if button was already pressed last frame
    if (state == Press && (mstates[i] == Press || mstates[i] == Hold))
    {
        state = Hold;
    }

    // Update state tracking for next frame
    if (real)
        mstates[i] = state;

    return static_cast<MouseButtonState>(state);
}

/**
 * @brief Get keyboard key state with press/hold detection
 * 
 * Similar to mouse button tracking, distinguishes between Press and Hold.
 * Returns Release if window is not focused.
 * 
 * @param key GLFW key code (e.g., GLFW_KEY_W, GLFW_KEY_SPACE)
 * @return KeyState Current key state
 */
Keyboard::KeyState Keyboard::GetKey(int key)
{
    // Get raw key state from GLFW
    int state = glfwGetKey(renderer->window, key);
    
    // Don't register input if window is not focused
    if (glfwGetWindowAttrib(renderer->window, GLFW_FOCUSED) == GLFW_FALSE)
    {
        state = Release;
    }

    // Resize state tracking vector if needed (dynamic size based on highest key code)
    if (kstates.size() <= key)
    {
        kstates.resize(key + 1, Release);
        kstates[key] = state;
    }
    else
    {
        // Convert Press to Hold if key was already pressed last frame
        if (state == Press && (kstates[key] == Press || kstates[key] == Hold))
        {
            state = Hold;
        }
        kstates[key] = state;
    }

    return static_cast<KeyState>(state);
}

/**
 * @brief Get gamepad button state with press/hold detection
 * 
 * Queries the first connected gamepad (GLFW_JOYSTICK_1) for button state.
 * 
 * @param button GLFW gamepad button ID
 * @return PadState Current button state
 */
Gamepad::PadState Gamepad::GetButton(int button)
{
    GLFWgamepadstate s;
    glfwGetGamepadState(GLFW_JOYSTICK_1, &s);

    int state = s.buttons[button];

    // Resize state tracking vector if needed
    if (gstates.size() <= button)
    {
        gstates.resize(button + 1, Release);
        gstates[button] = state;
    }
    else
    {
        // Convert Press to Hold if button was already pressed last frame
        if (state == Press && (gstates[button] == Press || gstates[button] == Hold))
        {
            state = Hold;
        }
        gstates[button] = state;
    }

    return static_cast<PadState>(state);
}

/**
 * @brief Get gamepad axis value with deadzone
 * 
 * Queries a gamepad analog stick axis. Applies a small deadzone (0.1)
 * to prevent drift from slightly off-center sticks.
 * 
 * @param axis GLFW gamepad axis ID (e.g., GLFW_GAMEPAD_AXIS_LEFT_X)
 * @return float Axis value in range [-1, 1], or 0 if within deadzone
 */
float Gamepad::GetAxis(int axis)
{
    GLFWgamepadstate s;
    glfwGetGamepadState(GLFW_JOYSTICK_1, &s);

    float a = s.axes[axis];

    // Apply deadzone to prevent stick drift
    if (glm::abs(a) <= 0.1)
    {
        a = 0;
    }
    return a;
}

InputState tmt::input::GetInputState() { return currentInputState; }

void tmt::input::ForceInputState(InputState state)
{
    forcedInputState = true;
    currentInputState = state;
}

/**
 * @brief Get virtual axis value by name
 * 
 * Provides consistent input across keyboard/mouse and gamepad by mapping
 * different input methods to the same virtual axis name.
 * 
 * Supported axes:
 * - "Horizontal": Arrow keys or A/D, or left stick X (-1 = left, 1 = right)
 * - "Vertical": Arrow keys or W/S, or left stick Y (-1 = down, 1 = up)
 * - "LookHorizontal": Mouse X delta or right stick X
 * - "LookVertical": Mouse Y delta or right stick Y
 * 
 * @param axis Name of the virtual axis
 * @return float Axis value typically in range [-1, 1]
 */
float tmt::input::GetAxis(string axis)
{
    var a = 0.0f;

    if (axis == "Horizontal")
    {
        if (currentInputState == KeyboardMouse)
        {
            // Check for left arrow or A key
            var l =
                (Keyboard::GetKey(GLFW_KEY_LEFT) == Keyboard::Hold) || (Keyboard::GetKey(GLFW_KEY_A) == Keyboard::Hold);

            // Check for right arrow or D key
            var r = (Keyboard::GetKey(GLFW_KEY_RIGHT) == Keyboard::Hold) ||
                (Keyboard::GetKey(GLFW_KEY_D) == Keyboard::Hold);


            if (l)
            {
                a = -1;  // Left
            }
            else if (r)
            {
                a = 1;   // Right
            }
        }
        else if (currentInputState == Gamepad)
        {
            // Use left stick X axis
            a = Gamepad::GetAxis(GLFW_GAMEPAD_AXIS_LEFT_X);
        }
    }
    else if (axis == "Vertical")
    {
        if (currentInputState == KeyboardMouse)
        {
            // Check for up arrow or W key
            var u =
                (Keyboard::GetKey(GLFW_KEY_UP) == Keyboard::Hold) || (Keyboard::GetKey(GLFW_KEY_W) == Keyboard::Hold);

            var d =
                (Keyboard::GetKey(GLFW_KEY_DOWN) == Keyboard::Hold) || (Keyboard::GetKey(GLFW_KEY_A) == Keyboard::Hold);

            if (u)
            {
                a = 1;
            }
            else if (d)
            {
                a = -1;
            }
        }
        else if (currentInputState == Gamepad)
        {
            a = -Gamepad::GetAxis(GLFW_GAMEPAD_AXIS_LEFT_Y);
        }
    }
    else if (axis == "LookHorizontal")
    {
        if (currentInputState == KeyboardMouse)
        {
            a = Mouse::GetMouseDelta().x;
        }
        else if (currentInputState == Gamepad)
        {
            a = -Gamepad::GetAxis(GLFW_GAMEPAD_AXIS_RIGHT_X);
        }
    }
    else if (axis == "LookVertical")
    {
        if (currentInputState == KeyboardMouse)
        {
            a = Mouse::GetMouseDelta().y;
        }
        else if (currentInputState == Gamepad)
        {
            a = Gamepad::GetAxis(GLFW_GAMEPAD_AXIS_RIGHT_Y);
        }
    }

    if (glm::abs(a) <= 0.1)
    {
        a = 0;
    }

    return a;
}


glm::vec2 tmt::input::GetAxis2(string axis)
{
    glm::vec2 a(0, 0);

    if (axis == "Move")
    {
        a.x = GetAxis("Horizontal");
        a.y = GetAxis("Vertical");
    }
    else if (axis == "Look")
    {
        a.x = -GetAxis("LookHorizontal");
        a.y = GetAxis("LookVertical");
    }

    return a;
}

string tmt::input::GetGamepadName() { return glfwGetJoystickName(0); }

int tmt::input::GetLastKey() { return lastKey; }

static void joystick_cb(int jid, int event)
{
    if (!forcedInputState)
    {
        if (event == GLFW_CONNECTED)
        {
            currentInputState = Gamepad;
            std::cout << "Connected gamepad" << std::endl;
        }
        else if (event == GLFW_DISCONNECTED)
        {
            currentInputState = KeyboardMouse;
            std::cout << "Disconnected gamepad" << std::endl;
        }
    }
}


static void char_cb(GLFWwindow* window, uint c)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharacter(c);
}

ImGuiKey ImGui_ImplGlfw_KeyToImGuiKey(int keycode, int scancode)
{
    IM_UNUSED(scancode);
    switch (keycode)
    {
        case GLFW_KEY_TAB:
            return ImGuiKey_Tab;
        case GLFW_KEY_LEFT:
            return ImGuiKey_LeftArrow;
        case GLFW_KEY_RIGHT:
            return ImGuiKey_RightArrow;
        case GLFW_KEY_UP:
            return ImGuiKey_UpArrow;
        case GLFW_KEY_DOWN:
            return ImGuiKey_DownArrow;
        case GLFW_KEY_PAGE_UP:
            return ImGuiKey_PageUp;
        case GLFW_KEY_PAGE_DOWN:
            return ImGuiKey_PageDown;
        case GLFW_KEY_HOME:
            return ImGuiKey_Home;
        case GLFW_KEY_END:
            return ImGuiKey_End;
        case GLFW_KEY_INSERT:
            return ImGuiKey_Insert;
        case GLFW_KEY_DELETE:
            return ImGuiKey_Delete;
        case GLFW_KEY_BACKSPACE:
            return ImGuiKey_Backspace;
        case GLFW_KEY_SPACE:
            return ImGuiKey_Space;
        case GLFW_KEY_ENTER:
            return ImGuiKey_Enter;
        case GLFW_KEY_ESCAPE:
            return ImGuiKey_Escape;
        case GLFW_KEY_APOSTROPHE:
            return ImGuiKey_Apostrophe;
        case GLFW_KEY_COMMA:
            return ImGuiKey_Comma;
        case GLFW_KEY_MINUS:
            return ImGuiKey_Minus;
        case GLFW_KEY_PERIOD:
            return ImGuiKey_Period;
        case GLFW_KEY_SLASH:
            return ImGuiKey_Slash;
        case GLFW_KEY_SEMICOLON:
            return ImGuiKey_Semicolon;
        case GLFW_KEY_EQUAL:
            return ImGuiKey_Equal;
        case GLFW_KEY_LEFT_BRACKET:
            return ImGuiKey_LeftBracket;
        case GLFW_KEY_BACKSLASH:
            return ImGuiKey_Backslash;
        case GLFW_KEY_RIGHT_BRACKET:
            return ImGuiKey_RightBracket;
        case GLFW_KEY_GRAVE_ACCENT:
            return ImGuiKey_GraveAccent;
        case GLFW_KEY_CAPS_LOCK:
            return ImGuiKey_CapsLock;
        case GLFW_KEY_SCROLL_LOCK:
            return ImGuiKey_ScrollLock;
        case GLFW_KEY_NUM_LOCK:
            return ImGuiKey_NumLock;
        case GLFW_KEY_PRINT_SCREEN:
            return ImGuiKey_PrintScreen;
        case GLFW_KEY_PAUSE:
            return ImGuiKey_Pause;
        case GLFW_KEY_KP_0:
            return ImGuiKey_Keypad0;
        case GLFW_KEY_KP_1:
            return ImGuiKey_Keypad1;
        case GLFW_KEY_KP_2:
            return ImGuiKey_Keypad2;
        case GLFW_KEY_KP_3:
            return ImGuiKey_Keypad3;
        case GLFW_KEY_KP_4:
            return ImGuiKey_Keypad4;
        case GLFW_KEY_KP_5:
            return ImGuiKey_Keypad5;
        case GLFW_KEY_KP_6:
            return ImGuiKey_Keypad6;
        case GLFW_KEY_KP_7:
            return ImGuiKey_Keypad7;
        case GLFW_KEY_KP_8:
            return ImGuiKey_Keypad8;
        case GLFW_KEY_KP_9:
            return ImGuiKey_Keypad9;
        case GLFW_KEY_KP_DECIMAL:
            return ImGuiKey_KeypadDecimal;
        case GLFW_KEY_KP_DIVIDE:
            return ImGuiKey_KeypadDivide;
        case GLFW_KEY_KP_MULTIPLY:
            return ImGuiKey_KeypadMultiply;
        case GLFW_KEY_KP_SUBTRACT:
            return ImGuiKey_KeypadSubtract;
        case GLFW_KEY_KP_ADD:
            return ImGuiKey_KeypadAdd;
        case GLFW_KEY_KP_ENTER:
            return ImGuiKey_KeypadEnter;
        case GLFW_KEY_KP_EQUAL:
            return ImGuiKey_KeypadEqual;
        case GLFW_KEY_LEFT_SHIFT:
            return ImGuiKey_LeftShift;
        case GLFW_KEY_LEFT_CONTROL:
            return ImGuiKey_LeftCtrl;
        case GLFW_KEY_LEFT_ALT:
            return ImGuiKey_LeftAlt;
        case GLFW_KEY_LEFT_SUPER:
            return ImGuiKey_LeftSuper;
        case GLFW_KEY_RIGHT_SHIFT:
            return ImGuiKey_RightShift;
        case GLFW_KEY_RIGHT_CONTROL:
            return ImGuiKey_RightCtrl;
        case GLFW_KEY_RIGHT_ALT:
            return ImGuiKey_RightAlt;
        case GLFW_KEY_RIGHT_SUPER:
            return ImGuiKey_RightSuper;
        case GLFW_KEY_MENU:
            return ImGuiKey_Menu;
        case GLFW_KEY_0:
            return ImGuiKey_0;
        case GLFW_KEY_1:
            return ImGuiKey_1;
        case GLFW_KEY_2:
            return ImGuiKey_2;
        case GLFW_KEY_3:
            return ImGuiKey_3;
        case GLFW_KEY_4:
            return ImGuiKey_4;
        case GLFW_KEY_5:
            return ImGuiKey_5;
        case GLFW_KEY_6:
            return ImGuiKey_6;
        case GLFW_KEY_7:
            return ImGuiKey_7;
        case GLFW_KEY_8:
            return ImGuiKey_8;
        case GLFW_KEY_9:
            return ImGuiKey_9;
        case GLFW_KEY_A:
            return ImGuiKey_A;
        case GLFW_KEY_B:
            return ImGuiKey_B;
        case GLFW_KEY_C:
            return ImGuiKey_C;
        case GLFW_KEY_D:
            return ImGuiKey_D;
        case GLFW_KEY_E:
            return ImGuiKey_E;
        case GLFW_KEY_F:
            return ImGuiKey_F;
        case GLFW_KEY_G:
            return ImGuiKey_G;
        case GLFW_KEY_H:
            return ImGuiKey_H;
        case GLFW_KEY_I:
            return ImGuiKey_I;
        case GLFW_KEY_J:
            return ImGuiKey_J;
        case GLFW_KEY_K:
            return ImGuiKey_K;
        case GLFW_KEY_L:
            return ImGuiKey_L;
        case GLFW_KEY_M:
            return ImGuiKey_M;
        case GLFW_KEY_N:
            return ImGuiKey_N;
        case GLFW_KEY_O:
            return ImGuiKey_O;
        case GLFW_KEY_P:
            return ImGuiKey_P;
        case GLFW_KEY_Q:
            return ImGuiKey_Q;
        case GLFW_KEY_R:
            return ImGuiKey_R;
        case GLFW_KEY_S:
            return ImGuiKey_S;
        case GLFW_KEY_T:
            return ImGuiKey_T;
        case GLFW_KEY_U:
            return ImGuiKey_U;
        case GLFW_KEY_V:
            return ImGuiKey_V;
        case GLFW_KEY_W:
            return ImGuiKey_W;
        case GLFW_KEY_X:
            return ImGuiKey_X;
        case GLFW_KEY_Y:
            return ImGuiKey_Y;
        case GLFW_KEY_Z:
            return ImGuiKey_Z;
        case GLFW_KEY_F1:
            return ImGuiKey_F1;
        case GLFW_KEY_F2:
            return ImGuiKey_F2;
        case GLFW_KEY_F3:
            return ImGuiKey_F3;
        case GLFW_KEY_F4:
            return ImGuiKey_F4;
        case GLFW_KEY_F5:
            return ImGuiKey_F5;
        case GLFW_KEY_F6:
            return ImGuiKey_F6;
        case GLFW_KEY_F7:
            return ImGuiKey_F7;
        case GLFW_KEY_F8:
            return ImGuiKey_F8;
        case GLFW_KEY_F9:
            return ImGuiKey_F9;
        case GLFW_KEY_F10:
            return ImGuiKey_F10;
        case GLFW_KEY_F11:
            return ImGuiKey_F11;
        case GLFW_KEY_F12:
            return ImGuiKey_F12;
        case GLFW_KEY_F13:
            return ImGuiKey_F13;
        case GLFW_KEY_F14:
            return ImGuiKey_F14;
        case GLFW_KEY_F15:
            return ImGuiKey_F15;
        case GLFW_KEY_F16:
            return ImGuiKey_F16;
        case GLFW_KEY_F17:
            return ImGuiKey_F17;
        case GLFW_KEY_F18:
            return ImGuiKey_F18;
        case GLFW_KEY_F19:
            return ImGuiKey_F19;
        case GLFW_KEY_F20:
            return ImGuiKey_F20;
        case GLFW_KEY_F21:
            return ImGuiKey_F21;
        case GLFW_KEY_F22:
            return ImGuiKey_F22;
        case GLFW_KEY_F23:
            return ImGuiKey_F23;
        case GLFW_KEY_F24:
            return ImGuiKey_F24;
        default:
            return ImGuiKey_None;
    }
}

// FIXME: should this be baked into ImGui_ImplGlfw_KeyToImGuiKey()? then what about the values passed to
// io.SetKeyEventNativeData()?
static int ImGui_ImplGlfw_TranslateUntranslatedKey(int key, int scancode)
{
#if GLFW_HAS_GETKEYNAME && !defined(EMSCRIPTEN_USE_EMBEDDED_GLFW3)
    // GLFW 3.1+ attempts to "untranslate" keys, which goes the opposite of what every other framework does, making
    // using lettered shortcuts difficult. (It had reasons to do so: namely GLFW is/was more likely to be used for
    // WASD-type game controls rather than lettered shortcuts, but IHMO the 3.1 change could have been done differently)
    // See https://github.com/glfw/glfw/issues/1502 for details.
    // Adding a workaround to undo this (so our keys are translated->untranslated->translated, likely a lossy process).
    // This won't cover edge cases but this is at least going to cover common cases.
    if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_EQUAL)
        return key;
    GLFWerrorfun prev_error_callback = glfwSetErrorCallback(nullptr);
    const char* key_name = glfwGetKeyName(key, scancode);
    glfwSetErrorCallback(prev_error_callback);
#if GLFW_HAS_GETERROR && !defined(EMSCRIPTEN_USE_EMBEDDED_GLFW3) // Eat errors (see #5908)
    (void)glfwGetError(nullptr);
#endif
    if (key_name && key_name[0] != 0 && key_name[1] == 0)
    {
        const char char_names[] = "`-=[]\\,;\'./";
        const int char_keys[] = {GLFW_KEY_GRAVE_ACCENT,  GLFW_KEY_MINUS,     GLFW_KEY_EQUAL, GLFW_KEY_LEFT_BRACKET,
                                 GLFW_KEY_RIGHT_BRACKET, GLFW_KEY_BACKSLASH, GLFW_KEY_COMMA, GLFW_KEY_SEMICOLON,
                                 GLFW_KEY_APOSTROPHE,    GLFW_KEY_PERIOD,    GLFW_KEY_SLASH, 0};
        IM_ASSERT(IM_ARRAYSIZE(char_names) == IM_ARRAYSIZE(char_keys));
        if (key_name[0] >= '0' && key_name[0] <= '9')
        {
            key = GLFW_KEY_0 + (key_name[0] - '0');
        }
        else if (key_name[0] >= 'A' && key_name[0] <= 'Z')
        {
            key = GLFW_KEY_A + (key_name[0] - 'A');
        }
        else if (key_name[0] >= 'a' && key_name[0] <= 'z')
        {
            key = GLFW_KEY_A + (key_name[0] - 'a');
        }
        else if (const char* p = strchr(char_names, key_name[0]))
        {
            key = char_keys[p - char_names];
        }
    }
    // if (action == GLFW_PRESS) printf("key %d scancode %d name '%s'\n", key, scancode, key_name);
#else
    IM_UNUSED(scancode);
#endif
    return key;
}

// X11 does not include current pressed/released modifier key in 'mods' flags submitted by GLFW
// See https://github.com/ocornut/imgui/issues/6034 and https://github.com/glfw/glfw/issues/1630
static void ImGui_ImplGlfw_UpdateKeyModifiers(GLFWwindow* window)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiMod_Ctrl,
                   (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) ||
                   (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS));
    io.AddKeyEvent(ImGuiMod_Shift,
                   (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ||
                   (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS));
    io.AddKeyEvent(ImGuiMod_Alt,
                   (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) ||
                   (glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS));
    io.AddKeyEvent(ImGuiMod_Super,
                   (glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS) ||
                   (glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS));
}

static void key_cb(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_RELEASE)
        return;

    ImGui_ImplGlfw_UpdateKeyModifiers(window);

    keycode = ImGui_ImplGlfw_TranslateUntranslatedKey(keycode, scancode);

    ImGuiIO& io = ImGui::GetIO();
    ImGuiKey imgui_key = ImGui_ImplGlfw_KeyToImGuiKey(keycode, scancode);
    io.AddKeyEvent(imgui_key, (action == GLFW_PRESS));
    io.SetKeyEventNativeData(imgui_key, keycode, scancode); // To support legacy indexing (<1.87 user code)
}

static void scrl_cb(GLFWwindow* window, double xoffset, double yoffset)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseWheelEvent(xoffset, yoffset);
    mousescrl = {xoffset, yoffset};
}

void tmt::input::init()
{
    glfwSetJoystickCallback(joystick_cb);
    glfwSetCharCallback(renderer->window, char_cb);
    glfwSetKeyCallback(renderer->window, key_cb);
    glfwSetScrollCallback(renderer->window, scrl_cb);
}


void tmt::input::Update()
{
    if (!forcedInputState)
    {

        var present = glfwJoystickPresent(GLFW_JOYSTICK_1);
        if (present)
        {
            currentInputState = Gamepad;
        }
        else
        {
            currentInputState = KeyboardMouse;
        }
    }
}
