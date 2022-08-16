#pragma once
#include "../Scene.h"
#include <memory>

namespace ppvs {
class Frontend;
class Game;
}

class GameWindow;
class GameFrontend;

class InGame final : public Scene {
public:
	explicit InGame(GameWindow& w, std::unique_ptr<ppvs::Game> game);
	~InGame() override;

	bool handleEvent(const SDL_Event& event) override;
	void update(double t) override;
	void draw() override;

private:
	GameWindow& m_window;
	std::unique_ptr<ppvs::Game> m_game;
	GameFrontend* m_frontend;
	double m_fadeVal = 1.0;
};
