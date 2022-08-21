#pragma once
#include "../Renderer.h"
#include "SDL_video.h"
#include <memory>
#include <vector>

namespace PuyoVS::Renderers::Software {

class RenderTargetSoft;

class TextureSoft final : public Texture {
public:
	explicit TextureSoft(RenderTargetSoft* target)
		: m_target(target)
	{
	}
	~TextureSoft() override = default;

	void bind(int slot) override;

	void upload(const int width, const int height, void* data) override
	{
		const size_t length = static_cast<size_t>(width) * static_cast<size_t>(height);
		auto pixels = static_cast<uint32_t*>(data);
		m_width = width;
		m_height = height;
		m_data.resize(length);
		for (size_t i = 0; i < length; i++) {
			const auto pixel = *pixels++;
			m_data[i] = {
				(pixel >> 0 & 0xFF) / 255.0f,
				(pixel >> 8 & 0xFF) / 255.0f,
				(pixel >> 16 & 0xFF) / 255.0f,
				(pixel >> 24 & 0xFF) / 255.0f
			};
		}
	}

	int width() override { return m_width; }

	int height() override { return m_height; }

	const std::vector<glm::vec4>& data() { return m_data; }

	glm::vec4 sample(glm::vec2 t)
	{
		return m_data[static_cast<int>(t.y) % m_height * m_width + static_cast<int>(t.x) % m_width];
	}

	void setFilter(const FilterType filter) override { m_filter = filter; }

private:
	RenderTargetSoft* m_target;
	int m_width = 0, m_height = 0;
	std::vector<glm::vec4> m_data;
	FilterType m_filter = FilterType::NearestFilter;
};

class PolyBufferSoft final : public PolyBuffer {
public:
	explicit PolyBufferSoft(RenderTargetSoft* target)
		: m_target(target)
	{
	}
	~PolyBufferSoft() override = default;

	void uploadVertices(Vertex* data, size_t count) override
	{
		m_vertices.assign(data, data + count);
	}

	void uploadIndices(unsigned short* data, size_t count) override
	{
		m_indices.assign(data, data + count);
	}

	void render(PolyShader shader) override;

	RenderTargetSoft* m_target;
	std::vector<Vertex> m_vertices;
	std::vector<unsigned short> m_indices;
};

class RenderTargetSoft final : public RenderTarget {
public:
	explicit RenderTargetSoft(SDL_Window* window, bool enableDebug = false);

	~RenderTargetSoft() override = default;

	void clear(const float r, const float g, const float b, const float a) override
	{
		m_colorBuffer.assign(m_colorBuffer.size(), packRgba(r, g, b, a));
		clearDepth();
	}

	void clearDepth() override
	{
		m_depthBuffer.assign(m_depthBuffer.size(), std::numeric_limits<uint16_t>::max());
	}

	void present() override;

	std::unique_ptr<PolyBuffer> makeBuffer() override;
	std::unique_ptr<Texture> makeTexture() override;

	void setViewport(int width, int height) override;
	void setProjection(const glm::mat4& projection) override { m_projection = projection; }
	void setModelView(const glm::mat4& modelView) override { m_modelView = modelView; }
	void setBlendMode(BlendingMode mode) override { m_blendMode = mode; }
	void setDepthFunction(DepthFunction func) override { m_depthFunction = func; }

	[[nodiscard]] const glm::mat4& projection() const { return m_projection; }
	[[nodiscard]] const glm::mat4& modelView() const { return m_modelView; }
	[[nodiscard]] const glm::ivec2& viewport() const override { return m_scaledViewport; }

	void render(PolyShader shader, const std::vector<Vertex>& vertices, const std::vector<unsigned short>& indices);
	void renderTriangle(PolyShader shader, const glm::mat4& transform, const Vertex& v0, const Vertex& v1, const Vertex& v2);

	void setTexture(int slot, TextureSoft* textureSoft);

	static uint32_t packRgba(const float r, const float g, const float b, const float a)
	{
		return static_cast<uint32_t>(r * 255.0f) << 24
			| static_cast<uint32_t>(g * 255.0f) << 16
			| static_cast<uint32_t>(b * 255.0f) << 8
			| static_cast<uint32_t>(a * 255.0f);
	}

private:
	SDL_Window* m_window;
	bool m_debugMode;
	std::vector<uint32_t> m_colorBuffer;
	std::vector<uint16_t> m_depthBuffer;
	glm::ivec2 m_viewport {};
	glm::ivec2 m_scaledViewport {};
	glm::mat4 m_projection { 1.0f };
	glm::mat4 m_modelView { 1.0f };
	BlendingMode m_blendMode = BlendingMode::AlphaBlending;
	DepthFunction m_depthFunction = DepthFunction::Always;
	TextureSoft* m_currentTexture = nullptr;
};

}
