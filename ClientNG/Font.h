#pragma once
#include "SDL_ttf.h"

namespace PuyoVS::Renderers {
class Texture;
}

namespace PuyoVS::ClientNG {

class Font final {
	friend class Renderers::Texture;

public:
	enum class Style {
		Normal,
		Italic,
		Bold,
		Strikethrough,
		Underscore,
	};

	explicit Font(const unsigned char* data, size_t length);
	~Font();

	Font(const Font&) = delete;
	Font& operator=(const Font&) = delete;
	Font(Font&&) = default;
	Font& operator=(Font&&) = default;

	void setFontSize(int size);
	void setFontStyle(Style renderStyle);
	void setFontOutline(int outline);
	void setFontKerning(int kerning);
	void setFontHinting(int hinting);

private:
	TTF_Font* m_font;
};

}
