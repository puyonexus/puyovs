#include "Renderer.h"

#include "../Font.h"
#include "./OpenGL/OpenGL.h"
#include "./Software/Software.h"

#include <SDL_ttf.h>
#include <spng.h>

namespace PuyoVS::Renderers {

void Texture::loadPng(const void* data, size_t length)
{
	spng_ctx* ctx = spng_ctx_new(0);
	spng_set_png_buffer(ctx, data, length);
	spng_ihdr header {};
	spng_get_ihdr(ctx, &header);
	size_t outSize;
	spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &outSize);
	const auto out = new uint8_t[outSize];
	spng_decode_image(ctx, out, outSize, SPNG_FMT_RGBA8, SPNG_DECODE_TRNS);
	spng_ctx_free(ctx);

	upload(static_cast<int>(header.width), static_cast<int>(header.height), out);
}

void Texture::renderText(const ClientNG::Font& font, const char* text, const SDL_Color color, const unsigned wrapLength, int& w, int& h)
{
	SDL_Surface* surface = TTF_RenderUTF8_Blended_Wrapped(font.m_font, text, color, wrapLength);
	w = surface->w;
	h = surface->h;
	SDL_LockSurface(surface);
	upload(surface->pitch / 4, surface->h, surface->pixels);
	SDL_UnlockSurface(surface);
	SDL_FreeSurface(surface);
}

RenderTarget* createRenderer(SDL_Window* window, const RenderSettings& settings)
{
	switch (settings.backend) {
	case RenderBackend::OpenGL:
		return new OpenGL::RenderTargetGL(window, settings.debug);
	case RenderBackend::Vulkan:
	case RenderBackend::Software:
		return new Software::RenderTargetSoft(window, settings.debug);
	case RenderBackend::Metal:
		break;
	}
	return nullptr;
}

}
