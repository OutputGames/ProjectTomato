#if !defined(INPUT_H)
#define INPUT_H

#include "utils.h"


#define TM_KEY_UNKNOWN            -1

/* Printable keys */
#define TM_KEY_SPACE              32
#define TM_KEY_APOSTROPHE         39  /* ' */
#define TM_KEY_COMMA              44  /* , */
#define TM_KEY_MINUS              45  /* - */
#define TM_KEY_PERIOD             46  /* . */
#define TM_KEY_SLASH              47  /* / */
#define TM_KEY_0                  48
#define TM_KEY_1                  49
#define TM_KEY_2                  50
#define TM_KEY_3                  51
#define TM_KEY_4                  52
#define TM_KEY_5                  53
#define TM_KEY_6                  54
#define TM_KEY_7                  55
#define TM_KEY_8                  56
#define TM_KEY_9                  57
#define TM_KEY_SEMICOLON          59  /* ; */
#define TM_KEY_EQUAL              61  /* = */
#define TM_KEY_A                  65
#define TM_KEY_B                  66
#define TM_KEY_C                  67
#define TM_KEY_D                  68
#define TM_KEY_E                  69
#define TM_KEY_F                  70
#define TM_KEY_G                  71
#define TM_KEY_H                  72
#define TM_KEY_I                  73
#define TM_KEY_J                  74
#define TM_KEY_K                  75
#define TM_KEY_L                  76
#define TM_KEY_M                  77
#define TM_KEY_N                  78
#define TM_KEY_O                  79
#define TM_KEY_P                  80
#define TM_KEY_Q                  81
#define TM_KEY_R                  82
#define TM_KEY_S                  83
#define TM_KEY_T                  84
#define TM_KEY_U                  85
#define TM_KEY_V                  86
#define TM_KEY_W                  87
#define TM_KEY_X                  88
#define TM_KEY_Y                  89
#define TM_KEY_Z                  90
#define TM_KEY_LEFT_BRACKET       91  /* [ */
#define TM_KEY_BACKSLASH          92  /* \ */
#define TM_KEY_RIGHT_BRACKET      93  /* ] */
#define TM_KEY_GRAVE_ACCENT       96  /* ` */
#define TM_KEY_WORLD_1            161 /* non-US #1 */
#define TM_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define TM_KEY_ESCAPE             256
#define TM_KEY_ENTER              257
#define TM_KEY_TAB                258
#define TM_KEY_BACKSPACE          259
#define TM_KEY_INSERT             260
#define TM_KEY_DELETE             261
#define TM_KEY_RIGHT              262
#define TM_KEY_LEFT               263
#define TM_KEY_DOWN               264
#define TM_KEY_UP                 265
#define TM_KEY_PAGE_UP            266
#define TM_KEY_PAGE_DOWN          267
#define TM_KEY_HOME               268
#define TM_KEY_END                269
#define TM_KEY_CAPS_LOCK          280
#define TM_KEY_SCROLL_LOCK        281
#define TM_KEY_NUM_LOCK           282
#define TM_KEY_PRINT_SCREEN       283
#define TM_KEY_PAUSE              284
#define TM_KEY_F1                 290
#define TM_KEY_F2                 291
#define TM_KEY_F3                 292
#define TM_KEY_F4                 293
#define TM_KEY_F5                 294
#define TM_KEY_F6                 295
#define TM_KEY_F7                 296
#define TM_KEY_F8                 297
#define TM_KEY_F9                 298
#define TM_KEY_F10                299
#define TM_KEY_F11                300
#define TM_KEY_F12                301
#define TM_KEY_F13                302
#define TM_KEY_F14                303
#define TM_KEY_F15                304
#define TM_KEY_F16                305
#define TM_KEY_F17                306
#define TM_KEY_F18                307
#define TM_KEY_F19                308
#define TM_KEY_F20                309
#define TM_KEY_F21                310
#define TM_KEY_F22                311
#define TM_KEY_F23                312
#define TM_KEY_F24                313
#define TM_KEY_F25                314
#define TM_KEY_KP_0               320
#define TM_KEY_KP_1               321
#define TM_KEY_KP_2               322
#define TM_KEY_KP_3               323
#define TM_KEY_KP_4               324
#define TM_KEY_KP_5               325
#define TM_KEY_KP_6               326
#define TM_KEY_KP_7               327
#define TM_KEY_KP_8               328
#define TM_KEY_KP_9               329
#define TM_KEY_KP_DECIMAL         330
#define TM_KEY_KP_DIVIDE          331
#define TM_KEY_KP_MULTIPLY        332
#define TM_KEY_KP_SUBTRACT        333
#define TM_KEY_KP_ADD             334
#define TM_KEY_KP_ENTER           335
#define TM_KEY_KP_EQUAL           336
#define TM_KEY_LEFT_SHIFT         340
#define TM_KEY_LEFT_CONTROL       341
#define TM_KEY_LEFT_ALT           342
#define TM_KEY_LEFT_SUPER         343
#define TM_KEY_RIGHT_SHIFT        344
#define TM_KEY_RIGHT_CONTROL      345
#define TM_KEY_RIGHT_ALT          346
#define TM_KEY_RIGHT_SUPER        347
#define TM_KEY_MENU               348

#define TM_KEY_LAST               TM_KEY_MENU

#define TM_PRESS 1
#define TM_REPEAT 2
#define TM_RELEASE 0

#define TM_MOUSE_BUTTON_1         0
#define TM_MOUSE_BUTTON_2         1
#define TM_MOUSE_BUTTON_3         2
#define TM_MOUSE_BUTTON_4         3
#define TM_MOUSE_BUTTON_5         4
#define TM_MOUSE_BUTTON_6         5
#define TM_MOUSE_BUTTON_7         6
#define TM_MOUSE_BUTTON_8         7
#define TM_MOUSE_BUTTON_LAST      TM_MOUSE_BUTTON_8
#define TM_MOUSE_BUTTON_LEFT      TM_MOUSE_BUTTON_1
#define TM_MOUSE_BUTTON_RIGHT     TM_MOUSE_BUTTON_2
#define TM_MOUSE_BUTTON_MIDDLE    TM_MOUSE_BUTTON_3



struct TMAPI tmInput
{
	static void pollEvents();
	static void update();
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void cursor_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

	static glm::vec2 getMousePosition();
	static glm::vec2 getMouseDelta();
	static glm::vec2 getScrollOffset();

	static int getKey(int key);
	static int getMouseButton(int button);

private:

	inline static List<int> keys_pressed = List<int>();
	inline static List<int> keys_repeated = List<int>();
	inline static List<int> keys_released = List<int>();
	inline static bool _attached;
	inline static glm::vec2 mousePosition;
	inline static glm::vec2 mouseDelta;

	inline static glm::vec2 scrollOffset;

};

#define TM_KEY_PRESSED(KEY) tmInput::getKey(KEY) == TM_PRESS

#endif // INPUT_H
