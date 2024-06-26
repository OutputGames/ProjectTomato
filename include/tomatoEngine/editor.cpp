#include "editor.h"

#include "engine.hpp"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "icons/IconsFontAwesome6.h"
#include "scripting/script.h"

#define RRES_IMPLEMENTATION
#include "rres.h"
#include "util/filesystem_tm.h"

teEditorMgr::teEditorMgr()
{
    camera_->framebuffer = new tmFramebuffer(tmFramebuffer::Deferred, {1,1});
    sceneFramebuffer = new tmFramebuffer(tmFramebuffer::Color, { 1,1 });

    var io = ImGui::GetIO();

    io.Fonts->AddFontDefault();
    float baseFontSize = 16; // 13.0f is the size of the default font. Change to the font size you use.
    float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

    // merge in icons from Font Awesome
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = iconFontSize;
    io.Fonts->AddFontFromFileTTF((std::string("resources/fonts/") + std::string(FONT_ICON_FILE_NAME_FAS)).c_str(), iconFontSize, &icons_config, icons_ranges);

    //ImGui::OpenPopup("FolderPopup");
}

void teEditorMgr::_drawDockspace()
{
    bool p_open = true;
    static bool opt_fullscreen = true; // Is the Dockspace full-screen?
    static bool opt_padding = false; // Is there padding (a blank space) between the window edge and the Dockspace?
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None; // Config flags for the Dockspace

    // In this example, we're embedding the Dockspace into an invisible parent window to make it more configurable.
    // We set ImGuiWindowFlags_NoDocking to make sure the parent isn't dockable into because this is handled by the Dockspace.
    //
    // ImGuiWindowFlags_MenuBar is to show a menu bar with config options. This isn't necessary to the functionality of a
    // Dockspace, but it is here to provide a way to change the configuration flags interactively.
    // You can remove the MenuBar flag if you don't want it in your app, but also remember to remove the code which actually
    // renders the menu bar, found at the end of this function.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    // Is the example in Fullscreen mode?
    if (opt_fullscreen)
    {
        // If so, get the main viewport:
        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        // Set the parent window's position, size, and viewport to match that of the main viewport. This is so the parent window
        // completely covers the main viewport, giving it a "full-screen" feel.
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        // Set the parent window's styles to match that of the main viewport:
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f); // No corner rounding on the window
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f); // No border around the window

        // Manipulate the window flags to make it inaccessible to the user (no titlebar, resize/move, or navigation)
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    else
    {
        // The example is not in Fullscreen mode (the parent window can be dragged around and resized), disable the
        // ImGuiDockNodeFlags_PassthruCentralNode flag.
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so the parent window should not have its own background:
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // If the padding option is disabled, set the parent window's padding size to 0 to effectively hide said padding.
    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    ImGui::Begin("DockSpace", &p_open, window_flags);

    // Remove the padding configuration - we pushed it, now we pop it:
    if (!opt_padding)
        ImGui::PopStyleVar();

    // Pop the two style rules set in Fullscreen mode - the corner rounding and the border size.
    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // Check if Docking is enabled:
    ImGuiIO& io = ImGui::GetIO();

    // If it is, draw the Dockspace with the DockSpace() function.
    // The GetID() function is to give a unique identifier to the Dockspace - here, it's "MyDockSpace".
    ImGuiID dockspace_id = ImGui::GetID("EngineDS");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
}

void AlignForWidth(float width, float alignment = 0.5f)
{
    ImGuiStyle& style = ImGui::GetStyle();
    float avail = ImGui::GetContentRegionAvail().x;
    float off = (avail - width) * alignment;
    if (off > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
}

void teEditorMgr::_drawMenuBar()
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New", "Ctrl+N"))
            {
	            //ImguiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Create new project", nullptr, ".");
	            std::cout << "made new project" << std::endl;
            }

            if (ImGui::MenuItem("Open", "Ctrl+O"))
            {
                std::cout << "opened new project" << std::endl;
            }

            if (ImGui::MenuItem("Save", "Ctrl+S"))
            {
                std::cout << "saved new project" << std::endl;
            }

            ImGui::EndMenu();
        }
    }
    ImGui::EndMainMenuBar();

    if (currentProject) {
        if (ImGui::BeginMenuBar())
        {

            AlignForWidth(ImGui::CalcTextSize(ICON_FA_PLAY "").x);
            bool p = ImGui::Button(ICON_FA_PLAY"");

            if (p)
            {
                tmeGetCore()->scriptMgr->CompileFull();
            }
        }
        ImGui::EndMenuBar();
    }
}

void teEditorMgr::_drawSceneView()
{
    camera_->position = { 0,0,-10 };
    camera_->front = { 0,0,1 };
    camera_->up = { 0,1,0 };
    camera_->Update();
    sceneFramebuffer->use();
    camera_->framebuffer->draw();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ImGui::Begin("Scene");
    {

        if (checkResize(0) == false)
        {
            ImGui::End();
            return;
        }

        ImGui::Image((ImTextureID)(sceneFramebuffer->gAlb),_windowSizes[0], ImVec2(0, 1), ImVec2(1, 0));

    }ImGui::End();

}

void teEditorMgr::_drawGameView()
{
    gameFramebuffer->use();
    tmCamera::GetMainCamera()->framebuffer->draw();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ImGui::Begin("Game");
    {

        if (checkResize(1) == false)
        {
            ImGui::End();
            return;
        }

        ImGui::Image((ImTextureID)(gameFramebuffer->gAlb), _windowSizes[1], ImVec2(0, 1), ImVec2(1, 0));

    }ImGui::End();
}

void teEditorMgr::_drawDebug()
{

    ImGui::Begin("Debug Menu");

    ImGui::Text("Texture Debugger");

    static int selectedTex = 0;

    if (ImGui::Button("<"))
    {
        selectedTex--;
    }

    ImGui::SameLine();

    ImGui::Text(std::to_string(selectedTex).c_str());

    ImGui::SameLine();

    if (ImGui::Button(">"))
    {
        selectedTex++;
    }

    //selectedTex = ImClamp(selectedTex, 0, 100);

    ImGui::Image((ImTextureID)selectedTex, { 256,256 });

    ImGui::End();

}

void teEditorMgr::_drawLog()
{
    ImGui::Begin(ICON_FA_MESSAGE " Log");

    std::map<string, Logger::Level> loggedents;

    for (std::pair<std::pair<int, std::string>, Logger::Level> logged_entry : Logger::loggedEntries)
    {
        if (!loggedents.count(logged_entry.first.second)) {
            ImVec4 col = { 1,1,1,1 };

            switch (logged_entry.second)
            {
            case Logger::INFO:
                break;
            case Logger::DBG:
                col = { 0,.5,1,1 };
                break;
            case Logger::WARN:
                col = { 1,1,0,1 };
                break;
            case Logger::LOG_ERROR:
                col = { 1,.25,.25,1 };
                break;
            }

            ImGui::TextColored(col, logged_entry.first.second.c_str());

            ImGui::NextColumn();

            loggedents.insert({ logged_entry.first.second, logged_entry.second });
        }
    }

    ImGui::End();
}

bool teEditorMgr::checkResize(int index)
{
    ImVec2 view = ImGui::GetContentRegionAvail();
    ImVec2 _windowSize = _windowSizes[index];
 
    if (view.x != _windowSize.x || view.y != _windowSize.y)
    {
        if (view.x == 0 || view.y == 0)
        {
            return false;
        }

        _windowSize = view;

        camera_->framebuffer->recreate({ view.x, view.y });
        sceneFramebuffer->recreate({ view.x,view.y });

        return true;
    }

    return true;
}

// Load a continuous data buffer from rresResourceChunkData struct
static unsigned char* LoadDataBuffer(rresResourceChunkData data, unsigned int rawSize)
{
    unsigned char* buffer = (unsigned char*)RRES_CALLOC((data.propCount + 1) * sizeof(unsigned int) + rawSize, 1);

    memcpy(buffer, &data.propCount, sizeof(unsigned int));
    for (int i = 0; i < data.propCount; i++) memcpy(buffer + (i + 1) * sizeof(unsigned int), &data.props[i], sizeof(unsigned int));
    memcpy(buffer + (data.propCount + 1) * sizeof(unsigned int), data.raw, rawSize);

    return buffer;
}

// Unload data buffer
static void UnloadDataBuffer(unsigned char* buffer)
{
    RRES_FREE(buffer);
}

// Load data chunk: RRES_DATA_RAW
// NOTE: This chunk can be used raw files embedding or other binary blobs
static void* LoadDataFromResourceChunk(rresResourceChunk chunk, unsigned int* size)
{
    void* rawData = NULL;

    if ((chunk.info.compType == RRES_COMP_NONE) && (chunk.info.cipherType == RRES_CIPHER_NONE))
    {
        rawData = RRES_CALLOC(chunk.data.props[0], 1);
        if (rawData != NULL) memcpy(rawData, chunk.data.raw, chunk.data.props[0]);
        *size = chunk.data.props[0];
    }
    else RRES_LOG("RRES: %c%c%c%c: WARNING: Data must be decompressed/decrypted\n", chunk.info.type[0], chunk.info.type[1], chunk.info.type[2], chunk.info.type[3]);

    return rawData;
}

// Load raw data from rres resource
void* LoadDataFromResource(rresResourceChunk chunk, unsigned int* size)
{
    void* rawData = NULL;

    // Data can be provided in the resource or linked to an external file
    if (rresGetDataType(chunk.info.type) == RRES_DATA_RAW)       // Raw data
    {
        rawData = LoadDataFromResourceChunk(chunk, size);
    }
    /*
    else if (rresGetDataType(chunk.info.type) == RRES_DATA_LINK) // Link to external file
    {
        // Get raw data from external linked file
        unsigned int dataSize = 0;
        void* data = LoadDataFromResourceLink(chunk, &dataSize);

        rawData = data;
        *size = dataSize;
    }
    */

    return rawData;
}

void AddTextToRRES(string path, FILE* rresFile, rresFileHeader& header)
{

    rresResourceChunkInfo chunkInfo = { 0 };    // Chunk info
    rresResourceChunkData chunkData = { 0 };    // Chunk data
    unsigned char* buffer = NULL;

    //Logger::logger << tmfs::loadFileString(path) << std::endl;

    const char* text = tmfs::loadFileString(path).c_str();
    unsigned rawSize = strlen(text);

    chunkInfo.type[0] = 'T';
    chunkInfo.type[1] = 'E';
    chunkInfo.type[2] = 'X';
    chunkInfo.type[3] = 'T';

    chunkInfo.id = rresComputeCRC32((unsigned char*)path.c_str(), strlen(path.c_str()));

    chunkInfo.compType = RRES_COMP_NONE,     // Data compression algorithm
        chunkInfo.cipherType = RRES_CIPHER_NONE, // Data encription algorithm
        chunkInfo.flags = 0,             // Data flags (if required)
        chunkInfo.baseSize = 5 * sizeof(unsigned int) + rawSize;   // Data base size (uncompressed/unencrypted)
    chunkInfo.packedSize = chunkInfo.baseSize; // Data chunk size (compressed/encrypted + custom data appended)
    chunkInfo.nextOffset = 0,        // Next resource chunk global offset (if resource has multiple chunks)
        chunkInfo.reserved = 0,          // <reserved>

        chunkData.propCount = 4;
    chunkData.props = (unsigned int*)RRES_CALLOC(chunkData.propCount, sizeof(unsigned int));
    chunkData.props[0] = rawSize;    // props[0]:size (bytes)
    chunkData.props[1] = RRES_TEXT_ENCODING_UNDEFINED;  // props[1]:rresTextEncoding
    chunkData.props[2] = RRES_CODE_LANG_UNDEFINED;      // props[2]:rresCodeLang
    chunkData.props[3] = 0x0409;     // props[3]:cultureCode: en-US: English - United States
    chunkData.raw = (char*)text;

    // Get a continuous data buffer from chunkData
    buffer = LoadDataBuffer(chunkData, rawSize);

    // Compute data chunk CRC32 (propCount + props[] + data)
    chunkInfo.crc32 = rresComputeCRC32(buffer, chunkInfo.packedSize);

    // Write resource chunk into rres file
    fwrite(&chunkInfo, sizeof(rresResourceChunkInfo), 1, rresFile);
    fwrite(buffer, 1, chunkInfo.packedSize, rresFile);

    // Free required memory
    memset(&chunkInfo, 0, sizeof(rresResourceChunkInfo));
    RRES_FREE(chunkData.props);
    UnloadDataBuffer(buffer);

    header.chunkCount += 1;
}

void ResourceManager::PackResources(string path, string pack_path)
{

    FILE* rresFile = fopen(path.c_str(), "wb");

    rresFileHeader header = {};
    header.id[0] = 'r', // File identifier: rres
    header.id[1] = 'r', // File identifier: rres
    header.id[2] = 'e', // File identifier: rres
    header.id[3] = 's', // File identifier: rres
    header.version = 100, // File version: 100 for version 1.0
    header.chunkCount = 0, // Number of resource chunks in the file (MAX: 65535)
    header.cdOffset = 0, // Central Directory offset in file (0 if not available)
    header.reserved = 0; // <reserved>

    fwrite(&header, sizeof(rresFileHeader), 1, rresFile);

    // testing chunk writing

    AddTextToRRES("resources/text.txt", rresFile, header);
    AddTextToRRES("resources/text2.txt", rresFile, header);

    fclose(rresFile);

    rresCentralDir dir = rresLoadCentralDirectory(path.c_str());
    var chunk = rresLoadResourceChunk(path.c_str(), rresGetResourceId(dir, "resources/text.txt"));
    var result = 0;              // Decompres/decipher resource data (if required)

    if (result == 0)    // Data decompressed/decrypted successfully
    {
        unsigned int dataSize = 0;
        var data = LoadDataFromResource(chunk, &dataSize);  // Load raw data, must be freed at the end

        std::cout << data;

        if ((data != NULL) && (dataSize > 0))
        {
            FILE* rawFile = fopen("export_data.raw", "wb");
            fwrite(data, 1, dataSize, rawFile);
            fclose(rawFile);
        }
    }

    rresUnloadResourceChunk(chunk);

}

void CenteredText(const char* text) {
    // Calculate the width of the text
    ImVec2 textSize = ImGui::CalcTextSize(text);

    // Get the available width
    float windowWidth = ImGui::GetContentRegionAvail().x;

    // Calculate the offset
    float textOffsetX = (windowWidth - textSize.x) * 0.5f;

    // Ensure the offset is not negative (i.e., the text is wider than the window)
    if (textOffsetX > 0.0f) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffsetX);
    }

    // Render the text
    ImGui::Text("%s", text);
}

void teEditorMgr::teCamera::Update()
{
    view = glm::lookAt(position, position + front, up);

    var engine = tmeGetCore();

    float aspect = framebuffer->size.x / framebuffer->size.y;

    switch (Projection)
    {
    case Perspective:
        proj = glm::perspective(glm::radians(FieldOfView), aspect, NearPlane, FarPlane);
        break;
    case Orthographic:
        //proj = glm::ortho
        break;
    }

    Use();

    for (auto material : tmeGetCore()->renderMgr->materials)
        UpdateShader(material->shader);

    var renderMgr = tmeGetCore()->renderMgr;
    renderMgr->Draw();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void teEditorMgr::DrawUI()
{

    // Draw dockspace

    _drawDockspace();

    _drawMenuBar();

    if (currentProject) {
        _drawSceneView();
        _drawDebug();
        _drawLog();
    } else
    {
        if (tmeGetCore()->applicationTime == 1.f)
        {
            ImGui::OpenPopup("FolderPopup");
        }

        ImVec2 wsiz = { tmeGetCore()->ScreenWidth, tmeGetCore()->ScreenHeight };

        static float div = 2.5;

        ImVec2 winSize = { tmeGetCore()->ScreenWidth / div,tmeGetCore()->ScreenHeight / div };

        ImGui::SetNextWindowSize(winSize);
        //ImGui::SetNextWindowPos({ wsiz.x * 0.5f, wsiz.y * 0.5f });

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, 0, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("FolderPopup", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
        {

            div = std::clamp(div, 1.5f, 5.0f);

            CenteredText("Tomato Engine");

            static bool new_project_open = false;

            if (ImGui::Button("New"))
            {
                new_project_open = true;
            }

            if (new_project_open)
            {
                static char buffer[256] = "";

                if (ImGui::InputText("Input Text", buffer, 256))
                {
	                std::cout << "project" << std::endl;
                }
            }

            ImGui::EndPopup();
        }
    }

    ImGui::End();

    var engine = tmeGetCore();

    engine->renderMgr->Clear();

}
