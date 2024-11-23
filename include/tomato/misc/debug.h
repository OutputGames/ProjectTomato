#if !defined(DEBUG_H)
#define DEBUG_H

#include "debug_draw.hpp"
#include "rendering/render.h"

struct TMAPI tmDebug
{
	static void rdoc_init();
	static void rdoc_beginframe();
	static void rdoc_endframe();
	static void tmgl_initdbg();
	static void tmgl_endframe();


};

#define glmtodd(v) ddVec3{v.x,v.y,v.z}
#define glmtoddin(v) ddVec3_In{v.x,v.y,v.z}

struct TMAPI tmglDebugRenderer : dd::RenderInterface
{

	tmglDebugRenderer();

	void drawPointList(const dd::DrawVertex* points, int count, bool depthEnabled) override;
	void drawLineList(const dd::DrawVertex* lines, int count, bool depthEnabled) override;

	uint linePointVAO, linePointVBO;
	tmShader* linePointShader;
	float pointsize = 1;
	
	static void box(vec3 origin, ddVec3_In color, vec3 size);
	static void sphere(vec3 origin, ddVec3_In color, float radius);
	static void line(vec3 start, vec3 end, ddVec3_In color);
	static void frustum(mat4 clipMatrix, ddVec3_In color);
	static void grid(float size, vec3 position, float squareSize, ddVec3_In color);
	static void cone(vec3 origin, vec3 dir, float min, float max, ddVec3_In color);

	static tmglDebugRenderer* renderer;

};



#endif // DEBUG_H
