#pragma once
#include "../Scene.h"

class GameWindow;

class IntroLogo final : public Scene {
public:
    IntroLogo(GameWindow &w);
	~IntroLogo() override;

	bool handleEvent(const SDL_Event& event) override;
	void update(double t) override;
	void draw() override;

	void finish();

private:
	GameWindow& m_window;
	std::unique_ptr<PolyBuffer> m_quadBuffer;
	std::unique_ptr<Texture> m_logoTexture;

    glm::vec4 m_clearColor { 0.15, 0.18, 0.20, 1.0 };
	Vertex m_quadVertices[4 * 9] {};
	unsigned short m_quadIndices[6 * 9] = {
		0, 1, 2, 2, 3, 0, // Quad 1
		4, 5, 6, 6, 7, 4, // Quad 2
		8, 9, 10, 10, 11, 8, // Quad 3
		12, 13, 14, 14, 15, 12, // Quad 4
		16, 17, 18, 18, 19, 16, // Quad 5
		20, 21, 22, 22, 23, 20, // Quad 6
		24, 25, 26, 26, 27, 24, // Quad 7
		28, 29, 30, 30, 31, 28, // Quad 8
		32, 33, 34, 34, 35, 32, // Quad 9
	};
};
