#if !defined(TMGL_H)
#define TMGL_H

#include "tomato/util/utils.h"

// initialization
TMAPI void tmInit(int width, int height, string name);
TMAPI void tmSwap();
TMAPI void tmClose();
TMAPI bool tmGetWindowClose();

// core
struct tm_CORE
{
	GLFWwindow* window;
};
TMAPI tm_CORE* tmGetCore();

struct tmgl
{
	static unsigned int genBuffer(GLenum target, const void* data, size_t size, GLenum usage);
	static void genVertexBuffer(unsigned int index, GLsizei size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer = (void*)0);
	static unsigned int genVertexArray();
	static unsigned int genShader(const char* vertex, const char* fragment);
};



#endif // TMGL_H
