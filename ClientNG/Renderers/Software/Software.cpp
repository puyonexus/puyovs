#include "Software.h"

#include <glm/ext/matrix_clip_space.hpp>

#include <algorithm>

namespace PuyoVS::Renderers::Software {

void TextureSoft::bind(const int slot)
{
	m_target->setTexture(slot, this);
}

void PolyBufferSoft::render(const PolyShader shader)
{
	m_target->render(shader, m_vertices, m_indices);
}

RenderTargetSoft::RenderTargetSoft(SDL_Window* window, bool enableDebug)
	: m_window(window)
	, m_debugMode(enableDebug)
{
	int w, h;
	SDL_GetWindowSize(m_window, &w, &h);
	setViewport(w, h);
}

void RenderTargetSoft::present()
{
	SDL_Surface* backBuffer = SDL_CreateRGBSurfaceFrom(
		m_colorBuffer.data(),
		m_scaledViewport.x,
		m_scaledViewport.y,
		32,
		m_scaledViewport.x * static_cast<int>(sizeof(uint32_t)),
		0xff000000,
		0x00ff0000,
		0x0000ff00,
		0x000000ff);
	SDL_SetSurfaceBlendMode(backBuffer, SDL_BLENDMODE_NONE);
	SDL_Surface* windowSurface = SDL_GetWindowSurface(m_window);
	SDL_BlitScaled(backBuffer, nullptr, windowSurface, nullptr);
	SDL_FreeSurface(backBuffer);
	SDL_UpdateWindowSurface(m_window);
}

std::unique_ptr<PolyBuffer> RenderTargetSoft::makeBuffer()
{
	return std::make_unique<PolyBufferSoft>(this);
}

std::unique_ptr<Texture> RenderTargetSoft::makeTexture()
{
	return std::make_unique<TextureSoft>(this);
}

void RenderTargetSoft::setViewport(int width, int height)
{
	m_viewport = { width, height };
	m_scaledViewport = { width, height };

	m_colorBuffer.resize(static_cast<size_t>(m_scaledViewport.x) * static_cast<size_t>(m_scaledViewport.y));
	m_depthBuffer.resize(static_cast<size_t>(m_scaledViewport.x) * static_cast<size_t>(m_scaledViewport.y));

	const float offset = ((static_cast<float>(width) / static_cast<float>(height)) - (4.0f / 3.0f));
	setProjection(glm::ortho(-1.0f - offset, 1.0f + offset, -1.0f, 1.0f, -1.0f, 1.0f));
}

void RenderTargetSoft::render(PolyShader shader, const std::vector<Vertex>& vertices, const std::vector<unsigned short>& indices)
{
	const auto transform = m_projection * m_modelView;

	if (shader != PolyShader::Simple) {
		return;
	}

	for (size_t i = 0; i < indices.size(); i += 3) {
		if (indices[i + 0] >= vertices.size() || indices[i + 1] >= vertices.size() || indices[i + 2] >= vertices.size()) {
			continue;
		}
		renderTriangle(shader, transform, vertices[indices[i + 0]], vertices[indices[i + 1]], vertices[indices[i + 2]]);
	}
}

void RenderTargetSoft::setTexture(int slot, TextureSoft* textureSoft)
{
	// Only support slot 0 for now.
	if (slot != 0) {
		return;
	}

	m_currentTexture = textureSoft;
}

// Begin software rasterization code.

struct renderVertex {
	glm::vec4 pos;
	glm::vec4 screen;
	glm::vec2 tex;
	glm::vec4 color;
};

static renderVertex computeRenderVertex(const glm::mat4& transform, const glm::ivec2& viewport, const glm::vec2& textureSize, const Vertex& v)
{
	const glm::vec4 pos = transform * glm::vec4(v.pos, 1);
	const renderVertex p {
		pos,
		{
			(pos.x + 1.0f) / 2.0f * static_cast<float>(viewport.x),
			(-pos.y + 1.0f) / 2.0f * static_cast<float>(viewport.y),
			0,
			0,
		},
		v.tex * textureSize,
		v.color,
	};
	return p;
}

inline float edgeFunction(const glm::vec4& a, const glm::vec4& b, const glm::vec4& c)
{
	return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

template <bool uniformColor, bool haveTexture, DepthFunction depthFunction>
void renderTriangleAlgorithm(PolyShader shader, const glm::mat4& transform, const glm::ivec2 scaledViewport, TextureSoft* currentTexture, const Vertex& v0, const Vertex& v1, const Vertex& v2, std::vector<uint32_t>& colorBuffer, std::vector<uint16_t>& depthBuffer)
{
	// TODO: This is a pretty horrifically inefficient rasterizer.

	if (scaledViewport.x < 1 || scaledViewport.y < 1) {
		return;
	}

	const glm::vec4* textureData = nullptr;
	int textureWidth = 0, textureHeight = 0;
	if (haveTexture) {
		textureData = currentTexture->data().data();
		textureWidth = currentTexture->width();
		textureHeight = currentTexture->height();
	}

	const glm::ivec2 textureSize = { textureWidth, textureHeight };

	renderVertex p0 = computeRenderVertex(transform, scaledViewport, textureSize, v0);
	renderVertex p1 = computeRenderVertex(transform, scaledViewport, textureSize, v1);
	renderVertex p2 = computeRenderVertex(transform, scaledViewport, textureSize, v2);

	if (glm::cross(glm::vec3(p1.screen - p0.screen), glm::vec3(p2.screen - p0.screen)).z > 0) {
		std::swap(p1, p2);
	}

	const int left = std::clamp(std::min({ static_cast<int>(std::floor(p0.screen.x)), static_cast<int>(std::floor(p1.screen.x)), static_cast<int>(std::floor(p2.screen.x)) }), 0, scaledViewport.x);
	const int right = std::clamp(std::max({ static_cast<int>(std::ceil(p0.screen.x)), static_cast<int>(std::ceil(p1.screen.x)), static_cast<int>(std::ceil(p2.screen.x)) }), 0, scaledViewport.x);
	const int top = std::clamp(std::min({ static_cast<int>(std::floor(p0.screen.y)), static_cast<int>(std::floor(p1.screen.y)), static_cast<int>(std::floor(p2.screen.y)) }), 0, scaledViewport.y);
	const int bottom = std::clamp(std::max({ static_cast<int>(std::ceil(p0.screen.y)), static_cast<int>(std::ceil(p1.screen.y)), static_cast<int>(std::ceil(p2.screen.y)) }), 0, scaledViewport.y);

	float area = edgeFunction(p0.screen, p1.screen, p2.screen);
	for (int y = top; y < bottom; y++) {
		int index = y * scaledViewport.x + left;
		for (int x = left; x < right; x++, index++) {
			glm::vec4 p = { static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f, 0, 0 };
			float w0 = edgeFunction(p1.screen, p2.screen, p);
			float w1 = edgeFunction(p2.screen, p0.screen, p);
			float w2 = edgeFunction(p0.screen, p1.screen, p);
			if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
				w0 /= area;
				w1 /= area;
				w2 /= area;

				uint16_t depthValue = std::numeric_limits<uint16_t>::max();
				if (depthFunction != DepthFunction::NoDepthTest) {
					depthValue = (w0 * p0.pos.z + w1 * p1.pos.z + w2 * p2.pos.z + 1.0) * (std::numeric_limits<uint16_t>::max() / 2);
				}

				switch (depthFunction) {
				case DepthFunction::NoDepthTest:
				case DepthFunction::Always:
					// Always pass
					break;
				case DepthFunction::LessOrEqual:
					if (depthValue > depthBuffer[index]) {
						continue;
					}
					break;
				case DepthFunction::GreaterOrEqual:
					if (depthValue < depthBuffer[index]) {
						continue;
					}
					break;
				case DepthFunction::Equal:
					if (depthValue != depthBuffer[index]) {
						continue;
					}
					break;
				}

				glm::vec4 color;
				if (uniformColor) {
					color = p0.color;
				} else {
					color = {
						w0 * p0.color.r + w1 * p1.color.r + w2 * p2.color.r,
						w0 * p0.color.g + w1 * p1.color.g + w2 * p2.color.g,
						w0 * p0.color.b + w1 * p1.color.b + w2 * p2.color.b,
						w0 * p0.color.a + w1 * p1.color.a + w2 * p2.color.a,
					};
				}
				if (haveTexture) {
					color *= textureData[static_cast<int>(w0 * p0.tex.x + w1 * p1.tex.x + w2 * p2.tex.x) % textureWidth
						+ static_cast<int>(w0 * p0.tex.y + w1 * p1.tex.y + w2 * p2.tex.y) % textureHeight * textureWidth];
				}

				color.r *= 255.f;
				color.g *= 255.f;
				color.b *= 255.f;

				if (color.a >= 1.0f) {
					const auto r = static_cast<uint32_t>(color.r) << 24;
					const auto g = static_cast<uint32_t>(color.g) << 16;
					const auto b = static_cast<uint32_t>(color.b) << 8;
					colorBuffer[index] = r | g | b | 0xff;
				} else {
					uint32_t src = colorBuffer[index];
					const auto r = static_cast<uint32_t>((1.0f - color.a) * static_cast<float>(src >> 24 & 0xff) + color.a * color.r) << 24;
					const auto g = static_cast<uint32_t>((1.0f - color.a) * static_cast<float>(src >> 16 & 0xff) + color.a * color.g) << 16;
					const auto b = static_cast<uint32_t>((1.0f - color.a) * static_cast<float>(src >> 8 & 0xff) + color.a * color.b) << 8;
					colorBuffer[index] = r | g | b | 0xff;
				}
				depthBuffer[index] = depthValue;
			}
		}
	}
}

void RenderTargetSoft::renderTriangle(const PolyShader shader, const glm::mat4& transform, const Vertex& v0, const Vertex& v1, const Vertex& v2)
{
#define CALL_RENDER_TRIANGLE_ALGORITHM_0(uniformColor, haveTexture, depthFunction) \
	renderTriangleAlgorithm<uniformColor, haveTexture, depthFunction>(shader, transform, m_scaledViewport, m_currentTexture, v0, v1, v2, m_colorBuffer, m_depthBuffer)

	const bool uniformColor = v0.color == v1.color && v1.color == v2.color;
	const bool haveTexture = m_currentTexture && m_currentTexture->width() > 0 && m_currentTexture->height() > 0;

#define CALL_RENDER_TRIANGLE_ALGORITHM_1(uniformColor, haveTexture)                                 \
	switch (m_depthFunction) {                                                                      \
	case DepthFunction::NoDepthTest:                                                                \
		CALL_RENDER_TRIANGLE_ALGORITHM_0(uniformColor, haveTexture, DepthFunction::NoDepthTest);    \
		break;                                                                                      \
	case DepthFunction::Always:                                                                     \
		CALL_RENDER_TRIANGLE_ALGORITHM_0(uniformColor, haveTexture, DepthFunction::Always);         \
		break;                                                                                      \
	case DepthFunction::LessOrEqual:                                                                \
		CALL_RENDER_TRIANGLE_ALGORITHM_0(uniformColor, haveTexture, DepthFunction::LessOrEqual);    \
		break;                                                                                      \
	case DepthFunction::GreaterOrEqual:                                                             \
		CALL_RENDER_TRIANGLE_ALGORITHM_0(uniformColor, haveTexture, DepthFunction::GreaterOrEqual); \
		break;                                                                                      \
	case DepthFunction::Equal:                                                                      \
		CALL_RENDER_TRIANGLE_ALGORITHM_0(uniformColor, haveTexture, DepthFunction::Equal);          \
		break;                                                                                      \
	}

#define CALL_RENDER_TRIANGLE_ALGORITHM_2(uniformColor)         \
	if (haveTexture) {                                         \
		CALL_RENDER_TRIANGLE_ALGORITHM_1(uniformColor, true);  \
	} else {                                                   \
		CALL_RENDER_TRIANGLE_ALGORITHM_1(uniformColor, false); \
	}

#define CALL_RENDER_TRIANGLE_ALGORITHM_3()       \
	if (uniformColor) {                          \
		CALL_RENDER_TRIANGLE_ALGORITHM_2(true);  \
	} else {                                     \
		CALL_RENDER_TRIANGLE_ALGORITHM_2(false); \
	}

	CALL_RENDER_TRIANGLE_ALGORITHM_3()

#undef CALL_RENDER_TRIANGLE_ALGORITHM_2
#undef CALL_RENDER_TRIANGLE_ALGORITHM_1
#undef CALL_RENDER_TRIANGLE_ALGORITHM_0
}

}
