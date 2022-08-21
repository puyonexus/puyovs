#include "OpenGL.h"

#include "SDL_log.h"

#include <SDL_opengl.h>
#include <SDL_video.h>
#include <glm/ext/matrix_clip_space.hpp>

namespace PuyoVS::Renderers::OpenGL {

namespace {

	void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg, const void* data)
	{
		SDL_Log("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, msg);
	}

}

struct OpenGLExtensions {
	PFNGLACTIVETEXTUREPROC glActiveTexture = reinterpret_cast<PFNGLACTIVETEXTUREPROC>(SDL_GL_GetProcAddress("glActiveTexture"));
	PFNGLGENBUFFERSPROC glGenBuffers = reinterpret_cast<PFNGLGENBUFFERSPROC>(SDL_GL_GetProcAddress("glGenBuffers"));
	PFNGLDELETEBUFFERSPROC glDeleteBuffers = reinterpret_cast<PFNGLDELETEBUFFERSPROC>(SDL_GL_GetProcAddress("glDeleteBuffers"));
	PFNGLBINDBUFFERPROC glBindBuffer = reinterpret_cast<PFNGLBINDBUFFERPROC>(SDL_GL_GetProcAddress("glBindBuffer"));
	PFNGLBUFFERDATAPROC glBufferData = reinterpret_cast<PFNGLBUFFERDATAPROC>(SDL_GL_GetProcAddress("glBufferData"));
	PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = reinterpret_cast<PFNGLGENVERTEXARRAYSPROC>(SDL_GL_GetProcAddress("glGenVertexArrays"));
	PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = reinterpret_cast<PFNGLDELETEVERTEXARRAYSPROC>(SDL_GL_GetProcAddress("glDeleteVertexArrays"));
	PFNGLBINDVERTEXARRAYPROC glBindVertexArray = reinterpret_cast<PFNGLBINDVERTEXARRAYPROC>(SDL_GL_GetProcAddress("glBindVertexArray"));
	PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = reinterpret_cast<PFNGLENABLEVERTEXATTRIBARRAYPROC>(SDL_GL_GetProcAddress("glEnableVertexAttribArray"));
	PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = reinterpret_cast<PFNGLVERTEXATTRIBPOINTERPROC>(SDL_GL_GetProcAddress("glVertexAttribPointer"));
	PFNGLCREATEPROGRAMPROC glCreateProgram = reinterpret_cast<PFNGLCREATEPROGRAMPROC>(SDL_GL_GetProcAddress("glCreateProgram"));
	PFNGLCREATESHADERPROC glCreateShader = reinterpret_cast<PFNGLCREATESHADERPROC>(SDL_GL_GetProcAddress("glCreateShader"));
	PFNGLSHADERSOURCEPROC glShaderSource = reinterpret_cast<PFNGLSHADERSOURCEPROC>(SDL_GL_GetProcAddress("glShaderSource"));
	PFNGLLINKPROGRAMPROC glLinkProgram = reinterpret_cast<PFNGLLINKPROGRAMPROC>(SDL_GL_GetProcAddress("glLinkProgram"));
	PFNGLCOMPILESHADERPROC glCompileShader = reinterpret_cast<PFNGLCOMPILESHADERPROC>(SDL_GL_GetProcAddress("glCompileShader"));
	PFNGLATTACHSHADERPROC glAttachShader = reinterpret_cast<PFNGLATTACHSHADERPROC>(SDL_GL_GetProcAddress("glAttachShader"));
	PFNGLUSEPROGRAMPROC glUseProgram = reinterpret_cast<PFNGLUSEPROGRAMPROC>(SDL_GL_GetProcAddress("glUseProgram"));
	PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation = reinterpret_cast<PFNGLBINDATTRIBLOCATIONPROC>(SDL_GL_GetProcAddress("glBindAttribLocation"));
	PFNGLGETSHADERIVPROC glGetShaderiv = reinterpret_cast<PFNGLGETSHADERIVPROC>(SDL_GL_GetProcAddress("glGetShaderiv"));
	PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = reinterpret_cast<PFNGLGETSHADERINFOLOGPROC>(SDL_GL_GetProcAddress("glGetShaderInfoLog"));
	PFNGLGETPROGRAMIVPROC glGetProgramiv = reinterpret_cast<PFNGLGETPROGRAMIVPROC>(SDL_GL_GetProcAddress("glGetProgramiv"));
	PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = reinterpret_cast<PFNGLGETPROGRAMINFOLOGPROC>(SDL_GL_GetProcAddress("glGetProgramInfoLog"));
	PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback = reinterpret_cast<PFNGLDEBUGMESSAGECALLBACKPROC>(SDL_GL_GetProcAddress("glDebugMessageCallback"));
	PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = reinterpret_cast<PFNGLGETUNIFORMLOCATIONPROC>(SDL_GL_GetProcAddress("glGetUniformLocation"));
	PFNGLUNIFORM1IPROC glUniform1i = reinterpret_cast<PFNGLUNIFORM1IPROC>(SDL_GL_GetProcAddress("glUniform1i"));
	PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = reinterpret_cast<PFNGLUNIFORMMATRIX4FVPROC>(SDL_GL_GetProcAddress("glUniformMatrix4fv"));
};

const char* kSimpleVertexShader = R"GLSL(
#version 150

in vec3 position;
in vec3 normal;
in vec2 texCoord;
in vec4 color;

out vec2 vTexCoord;
out vec4 vColor;

uniform mat4 projection;
uniform mat4 modelView;

void main()
{
    gl_Position = projection * modelView * vec4(position, 1.0);
    vTexCoord = texCoord;
    vColor = color;
}
)GLSL";

const char* kSimpleFragmentShader = R"GLSL(
#version 150

in vec2 vTexCoord;
in vec4 vColor;

out vec4 outColor;

uniform sampler2D tex;

void main()
{
    outColor = texture(tex, vTexCoord) * vColor;
}
)GLSL";

TextureGL::TextureGL(RenderTargetGL* target)
	: m_target(target)
{
	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

TextureGL::~TextureGL()
{
	glDeleteTextures(1, &m_texture);
}

void TextureGL::bind(int slot)
{
	m_target->extensions().glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_texture);
}

void TextureGL::upload(int width, int height, void* data)
{
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	m_width = width;
	m_height = height;
}

int TextureGL::width()
{
	return m_width;
}

int TextureGL::height()
{
	return m_height;
}

void TextureGL::setFilter(const FilterType filter)
{
	glBindTexture(GL_TEXTURE_2D, m_texture);
	switch (filter) {
	case FilterType::NearestFilter:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
	case FilterType::LinearFilter:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	}
}

PolyBufferGL::PolyBufferGL(RenderTargetGL* target)
	: m_target(target)
{
	const auto& ext = m_target->extensions();

	// Create objects.
	ext.glGenVertexArrays(1, &m_vertexArrayObj);
	ext.glGenBuffers(1, &m_indexBufferObj);
	ext.glGenBuffers(1, &m_vertexBufferObj);

	// Bind VAO and set VBOs.
	ext.glBindVertexArray(m_vertexArrayObj);
	ext.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObj);
	ext.glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObj);

	// Setup vertex attrib pointers for VAO.
	ext.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, pos))); // NOLINT(performance-no-int-to-ptr)
	ext.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal))); // NOLINT(performance-no-int-to-ptr)
	ext.glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, tex))); // NOLINT(performance-no-int-to-ptr)
	ext.glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color))); // NOLINT(performance-no-int-to-ptr)
	ext.glEnableVertexAttribArray(0);
	ext.glEnableVertexAttribArray(1);
	ext.glEnableVertexAttribArray(2);
	ext.glEnableVertexAttribArray(3);
}

PolyBufferGL::~PolyBufferGL()
{
	const auto& ext = m_target->extensions();
	ext.glDeleteBuffers(1, &m_indexBufferObj);
	ext.glDeleteBuffers(1, &m_vertexBufferObj);
	ext.glDeleteVertexArrays(1, &m_vertexArrayObj);
}

void PolyBufferGL::uploadVertices(Vertex* data, size_t count)
{
	const auto& ext = m_target->extensions();
	ext.glBindVertexArray(m_vertexArrayObj);
	ext.glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObj);
	ext.glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Vertex) * count), data, GL_STREAM_DRAW);
}

void PolyBufferGL::uploadIndices(unsigned short* data, const size_t count)
{
	const auto& ext = m_target->extensions();
	ext.glBindVertexArray(m_vertexArrayObj);
	ext.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferObj);
	ext.glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(unsigned short) * count), data, GL_STREAM_DRAW);
	m_indexCount = count;
}

void PolyBufferGL::render(PolyShader shader)
{
	const auto& ext = m_target->extensions();
	ext.glBindVertexArray(m_vertexArrayObj);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexCount), GL_UNSIGNED_SHORT, nullptr);
}

RenderTargetGL::RenderTargetGL(SDL_Window* window, const bool enableDebug)
	: RenderTarget()
	, m_window(window)
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	if (enableDebug) {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	}

	m_context = SDL_GL_CreateContext(m_window);
	m_ext = std::make_unique<OpenGLExtensions>();

	if (enableDebug) {
		m_ext->glDebugMessageCallback(debugCallback, nullptr);
	}

	m_simpleShader = loadShader(kSimpleVertexShader, kSimpleFragmentShader);
	m_simpleShaderProj = m_ext->glGetUniformLocation(m_simpleShader, "projection");
	m_simpleShaderModel = m_ext->glGetUniformLocation(m_simpleShader, "modelView");
	m_ext->glUniformMatrix4fv(m_simpleShaderProj, 1, GL_FALSE, &m_projection[0][0]);
	m_ext->glUniformMatrix4fv(m_simpleShaderModel, 1, GL_FALSE, &m_modelView[0][0]);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int w, h;
	SDL_GetWindowSize(m_window, &w, &h);
	setViewport(w, h);
}

RenderTargetGL::~RenderTargetGL()
{
	if (activeTarget == this) {
		activeTarget = nullptr;
	}

	SDL_GL_DeleteContext(m_context);
}

void RenderTargetGL::clear(const float r, const float g, const float b, const float a)
{
	makeCurrent();
	SDL_GL_MakeCurrent(m_window, m_context);
	glClearColor(
		static_cast<GLclampf>(r),
		static_cast<GLclampf>(g),
		static_cast<GLclampf>(b),
		static_cast<GLclampf>(a));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderTargetGL::clearDepth()
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

void RenderTargetGL::present()
{
	SDL_GL_SwapWindow(m_window);
}

std::unique_ptr<PolyBuffer> RenderTargetGL::makeBuffer()
{
	return std::make_unique<PolyBufferGL>(this);
}

std::unique_ptr<Texture> RenderTargetGL::makeTexture()
{
	return std::make_unique<TextureGL>(this);
}

void RenderTargetGL::setViewport(const int width, const int height)
{
	glViewport(0, 0, width, height);
	const float offset = ((static_cast<float>(width) / static_cast<float>(height)) - (4.0f / 3.0f));
	setProjection(glm::ortho(-1.0f - offset, 1.0f + offset, -1.0f, 1.0f, -1.0f, 1.0f));
	m_viewport = glm::ivec2(width, height);
}

void RenderTargetGL::setProjection(const glm::mat4& projection)
{
	m_projection = projection;
	m_ext->glUniformMatrix4fv(m_simpleShaderProj, 1, GL_FALSE, &m_projection[0][0]);
}

void RenderTargetGL::setModelView(const glm::mat4& modelView)
{
	m_modelView = modelView;
	m_ext->glUniformMatrix4fv(m_simpleShaderModel, 1, GL_FALSE, &m_modelView[0][0]);
}

void RenderTargetGL::setBlendMode(BlendingMode mode)
{
	switch (mode) {
	case BlendingMode::NoBlending:
		glDisable(GL_BLEND);
		break;
	case BlendingMode::AlphaBlending:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case BlendingMode::AdditiveBlending:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case BlendingMode::MultiplyBlending:
		glEnable(GL_BLEND);
		glBlendFunc(GL_DST_COLOR, GL_ZERO);
		break;
	}
}

void RenderTargetGL::setDepthFunction(DepthFunction func)
{
	switch (func) {
	case DepthFunction::NoDepthTest:
		glDisable(GL_DEPTH_TEST);
		break;
	case DepthFunction::Always:
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);
		break;
	case DepthFunction::LessOrEqual:
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		break;
	case DepthFunction::GreaterOrEqual:
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_GEQUAL);
		break;
	case DepthFunction::Equal:
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_EQUAL);
		break;
	}
}

OpenGLExtensions& RenderTargetGL::extensions() const
{
	return *m_ext;
}

unsigned RenderTargetGL::loadShader(const char* vertexSource, const char* fragmentSource) const
{
	const auto& ext = extensions();

	const GLuint program = ext.glCreateProgram();
	const GLuint vertexShader = ext.glCreateShader(GL_VERTEX_SHADER);
	ext.glShaderSource(vertexShader, 1, &vertexSource, nullptr);
	ext.glCompileShader(vertexShader);

	GLint success;
	ext.glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLint logLength;
		ext.glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLength);
		const auto log = new char[logLength];
		ext.glGetShaderInfoLog(vertexShader, logLength, nullptr, log);
		SDL_Log("Vertex shader compile error: %s", log);
		delete[] log;
	}

	const GLuint fragmentShader = ext.glCreateShader(GL_FRAGMENT_SHADER);
	ext.glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
	ext.glCompileShader(fragmentShader);

	ext.glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLint logLength;
		ext.glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLength);
		const auto log = new char[logLength];
		ext.glGetShaderInfoLog(fragmentShader, logLength, nullptr, log);
		SDL_Log("Fragment shader compile error: %s", log);
		delete[] log;
	}

	ext.glAttachShader(program, vertexShader);
	ext.glAttachShader(program, fragmentShader);
	ext.glBindAttribLocation(program, 0, "position");
	ext.glBindAttribLocation(program, 1, "normal");
	ext.glBindAttribLocation(program, 2, "texCoord");
	ext.glBindAttribLocation(program, 3, "color");
	ext.glLinkProgram(program);

	ext.glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		GLint logLength;
		ext.glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		const auto log = new char[logLength];
		ext.glGetProgramInfoLog(program, logLength, nullptr, log);
		SDL_Log("Program link error: %s", log);
		delete[] log;
	}

	ext.glUseProgram(program);

	const GLint texUniform = ext.glGetUniformLocation(program, "tex");
	ext.glUniform1i(texUniform, 0);

	return program;
}

}
