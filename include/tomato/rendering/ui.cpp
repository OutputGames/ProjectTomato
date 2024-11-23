#include "ui.h"

#include "engine.hpp"
#include "imgui_ext.hpp"
#include <imgui/misc/cpp/imgui_stdlib.h>

CLASS_DEFINITION(Component, tmText)

bool tmImage::IsClassType(const std::size_t classType) const
{
    if (classType == tmImage::Type) {
        return true;
    }
	return Component::IsClassType(classType);
}

const char* imgvert = R"(

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec4 ScreenPosition;

uniform mat4 model;
uniform mat4 projection;

void main()
{


    TexCoords = vec2(1.0) - aTexCoords;    
    ScreenPosition =  projection * model * vec4(aPos.xy,0.0,1.0);

    gl_Position =  ScreenPosition;
}

)";

const char* imgfrag = R"(

#version 420 core
layout (location = 0) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gShading;
layout (depth_greater) out float gl_FragDepth;

in vec2 TexCoords;
in vec4 ScreenPosition;

uniform sampler2D mainTexture;
uniform vec3 color_col;

void main()
{
    gAlbedoSpec = texture(mainTexture, TexCoords) * vec4(color_col, 1.0);

	

    gShading = vec4(0.0);
	gl_FragDepth = 0;
}

)";

tmVertexBuffer* quad_buffer;


tmUIMgr::tmUIMgr()
{
}

void tmImage::Start()
{
    if (!quad_buffer)
    {
        float vertices[] = {
            // positions          // colors           // texture coords
             1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
             1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
            -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
            -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
        };
        unsigned int indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
        };


        // Number of vertices
        const int numVertices = 4;

        // Create an array of tmVertex structures
        tmVertex tmVertices[numVertices];

        // Loop through the vertices array and fill the tmVertex structures
        for (int i = 0; i < numVertices; ++i)
        {
            tmVertices[i] = {};
            int offset = i * 8;
            tmVertices[i].position = glm::vec3(vertices[offset], vertices[offset + 1], vertices[offset + 2]);
            tmVertices[i].normal = glm::vec3(vertices[offset + 3], vertices[offset + 4], vertices[offset + 5]);
            tmVertices[i].texcoords = glm::vec2(vertices[offset + 6], vertices[offset + 7]);

            // Initialize bone IDs and weights to zero
            for (int j = 0; j < MAX_BONE_INFLUENCE; ++j)
            {
                tmVertices[i].m_BoneIDs[j] = 0;
                tmVertices[i].m_Weights[j] = 0.0f;
            }
        }


	    quad_buffer = new tmVertexBuffer(tmVertices, numVertices, sizeof(indices) / sizeof(unsigned int), indices);
    }

    var shader = new tmShader(imgvert, imgfrag, false);

	var material_ = new tmMaterial(shader);
    materialIndex = material_->materialIndex;
}

void tmImage::Update()
{
    var material = tmeGetCore()->renderMgr->materials[materialIndex];
    material->settings.useOrtho = true;

    std::vector<mat4> boneMatrix;

    for (int i = 0; i < MAX_BONES; ++i)
    {
        boneMatrix.push_back(mat4(1.0));
    }

    var drawCall = tmeGetCore()->renderMgr->InsertCall(quad_buffer, materialIndex, boneMatrix, transform()->GetMatrix(), actor());
}

void tmImage::EngineRender()
{

    var material = tmeGetCore()->renderMgr->materials[materialIndex];

    ImGui::Text(("Material Index: " + std::to_string(materialIndex)).c_str());

    {

        static string file_path = "";
        static bool found = false;
        static int actorN;

        if (ImGui::Button("Select.."))
            ImGui::OpenPopup("mat_popup");
        ImGui::SameLine();
        ImGui::TextUnformatted(materialIndex == -1 ? "<None>" : material->name.c_str());
        if (ImGui::BeginPopup("mat_popup"))
        {
            ImGui::SeparatorText("Material");
            for (int i = 0; i < tmeGetCore()->renderMgr->materials.size(); i++)
                if (ImGui::Selectable(tmeGetCore()->renderMgr->materials[i]->name.c_str()))
                    materialIndex = i;

            ImGui::Separator();

            if (ImGui::Selectable("Add new Material")) {

                var mat = new tmMaterial(material->shader);

                materialIndex = mat->materialIndex;

                material = mat;
            }
            ImGui::EndPopup();
        }
    }

    material->EngineRender();


}

FT_Library library = NULL;

tmFont::tmFont(string path, float pixSize)
{
    if (library == NULL) {
        if (FT_Init_FreeType(&library))
        {
            std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            return;
        }
    }
    FT_Face face;
    if (FT_New_Face(library, path.c_str(), 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }
    FT_Set_Pixel_Sizes(face, 0, pixSize);


    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    for (unsigned char c = 0; c < 128; c++)
    {
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
		tmFontCharacter character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        characters.Add(std::pair<char, tmFontCharacter>(c, character));
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    FT_Done_Face(face);
}

tmVertexBuffer* textbuffer;

void tmText::Update()
{

    if (textbuffer == nullptr)
    {
        unsigned int VAO, VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        textbuffer = new tmVertexBuffer(VAO, VBO);
        textbuffer->vertexCount = 6;
        textbuffer->mode = GL_TRIANGLES;
    }

    var material = tmeGetCore()->renderMgr->materials[materialIndex];
    material->settings.useOrtho = true;

    std::vector<mat4> boneMatrix;

    for (int i = 0; i < MAX_BONES; ++i)
    {
        boneMatrix.push_back(mat4(1.0));
    }

    std::string::const_iterator c;

    vec3 globPos = transform()->GetGlobalPosition();

    float x = globPos.x;

    for (c = text.begin(); c != text.end(); c++)
    {
        tmFontCharacter ch = font->characters[*c];

        float xpos = x + ch.glyphBearing.x * textScale;
        float ypos = globPos.y - (ch.glyphSize.y - ch.glyphBearing.y) * textScale;

        float w = ch.glyphSize.x * textScale;
        float h = ch.glyphSize.y * textScale;
        // update VBO for each character
        const float vertices[] = {
             xpos,     ypos + h,   0.0f, 0.0f,
        	xpos,     ypos,       0.0f, 1.0f,
             xpos + w, ypos,       1.0f, 1.0f,

             xpos,     ypos + h,   0.0f, 0.0f,
             xpos + w, ypos,       1.0f, 1.0f,
             xpos + w, ypos + h,   1.0f, 0.0f
        };

        const float* verticesPtr = vertices;

        var draw_call = tmeGetCore()->renderMgr->InsertCall(textbuffer, materialIndex, boneMatrix, transform()->GetMatrix(), actor());
        draw_call->subData.insert(draw_call->subData.end(), vertices, vertices + std::size(vertices));
        draw_call->subDataSize = sizeof(vertices);

        var texOverride = material->GetField("text_NOREN");

        texOverride.data.int_0 = ch.textureId;

        draw_call->overrideFields.Add(texOverride.name,texOverride);

        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.glyphOffset >> 6) * textScale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }

}

void tmText::EngineRender()
{
    ImGui::InputText("Text", &text);

    ImGui::DragFloat("Text Scale", &textScale, 0.01);

    var material = tmeGetCore()->renderMgr->materials[materialIndex];

    material->EngineRender();
}

void tmText::Start()
{

    string textvert = R"(
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";
    string textFrag = R"(


#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text_NOREN;
uniform vec3 textColor_col;

void main()
{    
    vec4 sampled = vec4(textColor_col, texture(text_NOREN, TexCoords).r);
    color = sampled;
}  

)";

    var shader = new tmShader(textvert, textFrag, false);

    var material_ = new tmMaterial(shader);
    materialIndex = material_->materialIndex;

    font = new tmFont("resources/fonts/BlitzBold.ttf", 32.f);
}
