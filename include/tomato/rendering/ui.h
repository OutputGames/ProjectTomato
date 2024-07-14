#if !defined(UI_H)
#define UI_H

#include "render.h"

#include "util/utils.h"
#include "ecs/actor.h"
#include <ft2build.h>
#include FT_FREETYPE_H  

struct TMAPI tmUIMgr
{
    tmUIMgr();
};

class tmImage : public Component
{
	CLASS_DECLARATION(tmImage)

public:
	tmImage(std::string&& initialValue) : Component(move(initialValue))
	{
	}

	tmImage() = default;

	void Start() override;
	void Update() override;
	void EngineRender() override;

private:

	int materialIndex;

};

struct tmFontCharacter {
	unsigned int textureId;  // ID handle of the glyph texture
	glm::ivec2   glyphSize;       // Size of glyph
	glm::ivec2   glyphBearing;    // Offset from baseline to left/top of glyph
	unsigned int glyphOffset;    // Offset to advance to next glyph
};

struct tmFont
{

	tmFont(string path, float pixSize);

	float GetPixelSize() { return pixelSize; }

	Dictionary<char, tmFontCharacter> characters;

private:

	float pixelSize;

};

class tmText : public Component
{
	CLASS_DECLARATION(tmText)

public:
	tmText(std::string&& initialValue) : Component(move(initialValue))
	{
	}

	tmText() = default;

	void Update() override;
	void EngineRender() override;
	void Start() override;

	tmFont* font;
	uint materialIndex;

	string text;

	float textScale=1;
};




#endif // UI_H
