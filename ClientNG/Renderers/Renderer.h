#pragma once
#include "SDL_pixels.h"
#include "SDL_ttf.h"

#include <memory>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct SDL_Window;

namespace PuyoVS {
namespace ClientNG {
    class Font;
}
}

namespace PuyoVS::Renderers {

enum class PolyShader {
	Simple
};

enum class FilterType {
	NearestFilter,
	LinearFilter
};

enum class BlendingMode {
	NoBlending,
	AlphaBlending,
	AdditiveBlending,
	MultiplyBlending
};

enum class DepthFunction {
	NoDepthTest,
	Always,
	LessOrEqual,
	GreaterOrEqual,
	Equal
};

struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 tex;
	glm::vec4 color;
};

class PolyBuffer {
public:
	PolyBuffer() = default;
	virtual ~PolyBuffer() = default;

	PolyBuffer(const PolyBuffer&) = delete;
	PolyBuffer& operator=(const PolyBuffer&) = delete;
	PolyBuffer(PolyBuffer&&) = delete;
	PolyBuffer& operator=(PolyBuffer&&) = delete;

	virtual void uploadVertices(Vertex* data, size_t count) = 0;
	virtual void uploadIndices(unsigned short* data, size_t count) = 0;
	virtual void render(PolyShader shader) = 0;
};

class Texture {
public:
	Texture() = default;
	virtual ~Texture() = default;

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;
	Texture(Texture&&) = delete;
	Texture& operator=(Texture&&) = delete;

	virtual void bind(int slot) = 0;
	virtual void upload(int width, int height, void* data) = 0;
	virtual int width() = 0;
	virtual int height() = 0;
	virtual void setFilter(FilterType filter) = 0;

	void loadPng(const void* data, size_t length);
	void renderText(const ClientNG::Font& font, const char* text, SDL_Color color, unsigned int wrapLength, int& w, int& h);
};

class RenderTarget {
public:
	RenderTarget() = default;
	virtual ~RenderTarget() = default;

	RenderTarget(const RenderTarget&) = delete;
	RenderTarget& operator=(const RenderTarget&) = delete;
	RenderTarget(RenderTarget&&) = delete;
	RenderTarget& operator=(RenderTarget&&) = delete;

	virtual void clear(float r, float g, float b, float a) = 0;
	virtual void clearDepth() = 0;
	virtual void present() = 0;

	virtual std::unique_ptr<PolyBuffer> makeBuffer() = 0;
	virtual std::unique_ptr<Texture> makeTexture() = 0;

	virtual void setViewport(int width, int height) = 0;
	virtual void setProjection(const glm::mat4& projection) = 0;
	virtual void setModelView(const glm::mat4& modelView) = 0;
	virtual void setBlendMode(BlendingMode mode) = 0;
	virtual void setDepthFunction(DepthFunction func) = 0;

	[[nodiscard]] virtual const glm::ivec2& viewport() const = 0;
};

enum class RenderBackend {
	OpenGL,
	Vulkan,
	Metal,
	Software,
};

struct RenderSettings {
	RenderBackend backend = RenderBackend::OpenGL;
	bool debug = false;
};

RenderTarget* createRenderer(SDL_Window* window, const RenderSettings& settings);

}
