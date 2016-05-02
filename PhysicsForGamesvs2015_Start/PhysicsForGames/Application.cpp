#include "Application.h"
#include "gl_core_4_4.h"
#include "GLFW/glfw3.h"
#include <cstdio>


void FormatDebugOutputARB(char outStr[], size_t outStrSize, GLenum source, GLenum type,
    GLuint id, GLenum severity, const char *msg);

void __stdcall DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar *message, GLvoid *userParam);

Application::Application(){}
Application::~Application(){}

bool Application::startup()
{
	if (glfwInit() == false)
	{
		return false;
	}

#if _DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

    window_width = 1280;
    window_height = 720;
    this->m_window = glfwCreateWindow(window_width, window_height, "Computer Graphics", nullptr, nullptr);
	if (this->m_window == nullptr)
	{
		return false;
	}

	glfwMakeContextCurrent(this->m_window);

	if (ogl_LoadFunctions() == ogl_LOAD_FAILED)
	{
		glfwDestroyWindow(this->m_window);
		glfwTerminate();
		return false;
	}

	int major_version = ogl_GetMajorVersion();
	int minor_version = ogl_GetMinorVersion();
	printf("Successfully loaded OpenGL version %d.%d\n",
		major_version, minor_version);


#ifdef _DEBUG
    glDebugMessageCallback((GLDEBUGPROC)DebugCallback, stderr); // print debug output to stderr
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

	glfwSwapInterval(1);


	return true;
}

void Application::shutdown()
{
	glfwDestroyWindow(this->m_window);
	glfwTerminate();
}

bool Application::update() 
{ 
	if ( glfwWindowShouldClose(m_window) )
	{
		return false;
	}

    int w, h;
    glfwGetWindowSize(m_window, &w, &h);
    if (w != window_width || h != window_height)
    {
        glViewport(0, 0, w, h);
        window_width = w;
        window_height = h;
    }
	return true; 
}

void Application::draw() {}

#if _DEBUG

void FormatDebugOutputARB(char outStr[], size_t outStrSize, GLenum source, GLenum type,
    GLuint id, GLenum severity, const char *msg)
{
    char sourceStr[512];
    const char *sourceFmt = "UNDEFINED(0x%04X)";
    switch (source)

    {
    case GL_DEBUG_SOURCE_API:             sourceFmt = "API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sourceFmt = "WINDOW_SYSTEM"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceFmt = "SHADER_COMPILER"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     sourceFmt = "THIRD_PARTY"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     sourceFmt = "APPLICATION"; break;
    case GL_DEBUG_SOURCE_OTHER:           sourceFmt = "OTHER"; break;
    }

    _snprintf_s(sourceStr, 256, sourceFmt, source);

    char typeStr[512];
    const char *typeFmt = "UNDEFINED(0x%04X)";
    switch (type)
    {

    case GL_DEBUG_TYPE_ERROR:               typeFmt = "ERROR"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeFmt = "DEPRECATED_BEHAVIOR"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeFmt = "UNDEFINED_BEHAVIOR"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         typeFmt = "PORTABILITY"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         typeFmt = "PERFORMANCE"; break;
    case GL_DEBUG_TYPE_OTHER:               typeFmt = "OTHER"; break;
    }
    _snprintf(typeStr, 256, typeFmt, type);


    char severityStr[512];
    const char *severityFmt = "UNDEFINED";
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:   severityFmt = "HIGH";   break;
    case GL_DEBUG_SEVERITY_MEDIUM: severityFmt = "MEDIUM"; break;
    case GL_DEBUG_SEVERITY_LOW:    severityFmt = "LOW"; break;
    }

    _snprintf(severityStr, 256, severityFmt, severity);

    _snprintf(outStr, outStrSize, "OpenGL: %s [source=%s type=%s severity=%s id=%d]",
        msg, sourceStr, typeStr, severityStr, id);
}

void __stdcall DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar *message, GLvoid *userParam)
{
    (void)length;
    FILE *outFile = (FILE*)userParam;
    char finalMessage[512];
    FormatDebugOutputARB(finalMessage, 256, source, type, id, severity, message);

    if (type != GL_DEBUG_TYPE_OTHER)
    {
        fprintf(outFile, "%s\n", finalMessage);
    }
}

#endif