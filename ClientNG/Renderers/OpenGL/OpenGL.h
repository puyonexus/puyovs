#pragma once
#include "../Renderer.h"
#include "SDL_video.h"
#include <memory>

namespace PuyoVS::Renderers::OpenGL {

class RenderTargetGL;
struct OpenGLExtensions;

class TextureGL final : public Texture {
public:
	explicit TextureGL(RenderTargetGL* target);
	~TextureGL() override;

	void bind(int slot) override;
	void upload(int width, int height, void* data) override;
	int width() override;
	int height() override;
	void setFilter(FilterType filter) override;

private:
	int m_width = 0, m_height = 0;
	RenderTargetGL* m_target;
	unsigned int m_texture {};
};

class PolyBufferGL final : public PolyBuffer {
public:
	explicit PolyBufferGL(RenderTargetGL* target);
	~PolyBufferGL() override;

	void uploadVertices(Vertex* data, size_t count) override;
	void uploadIndices(unsigned short* data, size_t count) override;
	void render(PolyShader shader) override;

	RenderTargetGL* m_target;
	unsigned int m_vertexArrayObj = 0;
	unsigned int m_indexBufferObj = 0;
	unsigned int m_vertexBufferObj = 0;
	size_t m_indexCount = 0;
};

class RenderTargetGL final : public RenderTarget {
public:
	explicit RenderTargetGL(SDL_Window* window, bool enableDebug = false);
	~RenderTargetGL() override;

	void clear(float r, float g, float b, float a) override;
	void clearDepth() override;
	void present() override;

	std::unique_ptr<PolyBuffer> makeBuffer() override;
	std::unique_ptr<Texture> makeTexture() override;

	void setViewport(int width, int height) override;
	void setProjection(const glm::mat4& projection) override;
	void setModelView(const glm::mat4& modelView) override;
	void setBlendMode(BlendingMode mode) override;
	void setDepthFunction(DepthFunction func) override;

	[[nodiscard]] OpenGLExtensions& extensions() const;
	[[nodiscard]] const glm::mat4& projection() const { return m_projection; }
	[[nodiscard]] const glm::mat4& modelView() const { return m_modelView; }
	[[nodiscard]] const glm::ivec2& viewport() const override { return m_viewport; }

private:
	unsigned int loadShader(const char* vertexSource, const char* fragmentSource) const;

	void makeCurrent()
	{
		if (activeTarget == this) {
			return;
		}
		SDL_GL_MakeCurrent(m_window, m_context);
		activeTarget = this;
	}

	inline static RenderTargetGL* activeTarget = nullptr;

	SDL_Window* m_window;
	SDL_GLContext m_context;
	std::unique_ptr<OpenGLExtensions> m_ext;
	glm::mat4 m_projection { 1.0f };
	glm::mat4 m_modelView { 1.0f };
	glm::ivec2 m_viewport {};

	unsigned int m_simpleShader;
	int m_simpleShaderProj;
	int m_simpleShaderModel;
};

}
