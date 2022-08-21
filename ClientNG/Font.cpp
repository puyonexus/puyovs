#include "Font.h"

namespace PuyoVS::ClientNG {

Font::Font(const unsigned char* data, size_t length)
{
	m_font = TTF_OpenFontRW(SDL_RWFromMem((void*)data, length), 1, 12);
}

Font::~Font()
{
	TTF_CloseFont(m_font);
}

void Font::setFontSize(const int size)
{
	TTF_SetFontSize(m_font, size);
}

void Font::setFontStyle(const Style renderStyle)
{
	int style = 0;

	switch (renderStyle) {
	case Style::Normal:
		style = TTF_STYLE_NORMAL;
		break;
	case Style::Italic:
		style = TTF_STYLE_ITALIC;
		break;
	case Style::Bold:
		style = TTF_STYLE_BOLD;
		break;
	case Style::Strikethrough:
		style = TTF_STYLE_STRIKETHROUGH;
		break;
	case Style::Underscore:
		style = TTF_STYLE_UNDERLINE;
		break;
	default:
		break;
	}

	TTF_SetFontStyle(m_font, style);
}

void Font::setFontOutline(const int outline)
{
	TTF_SetFontOutline(m_font, outline);
}

void Font::setFontKerning(const int kerning)
{
	TTF_SetFontKerning(m_font, kerning);
}

void Font::setFontHinting(const int hinting)
{
	TTF_SetFontHinting(m_font, hinting);
}

}
