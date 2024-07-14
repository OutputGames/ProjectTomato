#include "tmgl.h"

#include "engine.hpp"
#include "ImGuizmo.h"

#include "misc/debug.h"


static tm_CORE* core;
bool captureFrame, capturingFrame;

TMAPI void tmInit(int width, int height, string windowName)
{

    // glfw: initialize and configure
// ------------------------------

#ifdef DEBUG
    tmDebug::rdoc_init();
#endif

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    core = new tm_CORE;


    // glfw window creation
    // --------------------
    core->window = glfwCreateWindow(width, height, windowName.c_str(), NULL, NULL);
    if (core->window == nullptr)
    {
        Logger::logger << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(core->window);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    glewExperimental = true; // Needed in core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        exit(-1);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    core->ctx = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(core->window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init("#version 130");
}

void tmSwap()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    GLFWwindow* backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);

    glfwSwapBuffers(core->window);

#ifdef DEBUG

    if (capturingFrame)
    {
        tmDebug::rdoc_endframe();
        capturingFrame = false;
    }

    if (captureFrame) {
        tmDebug::rdoc_beginframe();
        captureFrame = false;
        capturingFrame = true;
    }
#endif


}

void tmPoll()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    tmInput::update();

    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}

void tmClose()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
}

bool tmGetWindowClose()
{
    return glfwWindowShouldClose(core->window);
}

void tmCapture()
{
    captureFrame = true;
}

glm::vec2 tm_CORE::getWindowPos()
{
    int window_x, window_y;
    glfwGetWindowPos(tmGetCore()->window, &window_x, &window_y);

    return glm::vec2(window_x, window_y);
}

tm_CORE* tmGetCore()
{
    return core;
}

void tmgl::genVertexBuffer(unsigned index, GLsizei size,
	GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
{


    if (type != GL_INT)
		glVertexAttribPointer(index, size, type, normalized, stride, pointer);
    else
        glVertexAttribIPointer(index, size, type, stride, pointer);
    glEnableVertexAttribArray(index);

}

unsigned tmgl::genVertexArray()
{
    unsigned i = -1;

    glGenVertexArrays(1, &i);
    glBindVertexArray(i);

    return i;
}

unsigned tmgl::genShader(const char* vertex, const char* fragment)
{
	std::cout << "Generating shader.." << std::endl;
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &(vertex), NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        Logger::logger << "Shader compilation failed at vertex stage. \n" << vertex << "\n\n" << infoLog << std::endl;
        return 0;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &(fragment), NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        Logger::logger << "Shader compilation failed at fragment stage. \n" << fragment << "\n\n" << infoLog << std::endl;
        return 0;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        Logger::logger << "Shader linking failed. \n" << infoLog << std::endl;
        return 0;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

	std::cout << "Successfully compiled shader!" << std::endl;

    return shaderProgram;
}

void tmgl::freeBuffer(unsigned id)
{
    glDeleteBuffers(1, &id);
}

unsigned tmgl::genBuffer(GLenum target, const void* data, size_t size, GLenum usage)
{

    unsigned int i = -1;

    glGenBuffers(1, &i);
    glBindBuffer(target, i);
    glBufferData(target, size, data, usage);
    //glBindBuffer(target,0);

    return i;
}
