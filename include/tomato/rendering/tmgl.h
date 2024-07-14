#if !defined(TMGL_H)
#define TMGL_H

#include "tomato/util/utils.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

// initialization
TMAPI void tmInit(int width, int height, string name);
TMAPI void tmSwap();
TMAPI void tmPoll();
TMAPI void tmClose();
TMAPI bool tmGetWindowClose();
TMAPI void tmCapture();

// core
struct TMAPI tm_CORE
{
	GLFWwindow* window;
	ImGuiContext* ctx;

	glm::vec2 getWindowPos();
};
TMAPI tm_CORE* tmGetCore();

struct TMAPI tmgl
{
	static unsigned int genBuffer(GLenum target, const void* data, size_t size, GLenum usage);
	static void genVertexBuffer(unsigned int index, GLsizei size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer = (void*)0);
	static unsigned int genVertexArray();
	static unsigned int genShader(const char* vertex, const char* fragment);
	static void freeBuffer(unsigned id);
};



#endif // TMGL_H
