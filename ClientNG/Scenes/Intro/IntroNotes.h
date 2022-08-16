#pragma once
#include "../Scene.h"
#include "../../Font.h"

class GameWindow;

class IntroNotes final : public Scene {
public:
	explicit IntroNotes(GameWindow& w);
	~IntroNotes() override;

	bool handleEvent(const SDL_Event& event) override;
	void update(double t) override;
	void draw() override;

	void finish();

private:
	glm::vec4 m_clearColor { 0, 0, 0, 1.0 };
	Vertex m_quadVertices[4] {};
	unsigned short m_quadIndices[6] = {
		0, 1, 2, 2, 3, 0
	};

	GameWindow& m_window;
	std::unique_ptr<PolyBuffer> m_quadBuffer;
	std::unique_ptr<Font> m_font;
	int m_textWidth = 0, m_textHeight = 0;
	std::unique_ptr<Texture> m_text;
};
