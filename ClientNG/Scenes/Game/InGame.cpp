#include "InGame.h"

#include "Frontend.h"
#include "../../Game.h"
#include "../../../Puyolib/Game.h"

InGame::InGame(GameWindow& w, std::unique_ptr<ppvs::Game> game)
	: Scene(w.renderTarget())
	, m_window(w)
	, m_game(std::move(game))
	, m_frontend(new GameFrontend(m_target))
{
	m_game->initGame(m_frontend);
}

InGame::~InGame() = default;

bool InGame::handleEvent(const SDL_Event& event)
{
	return false;
}

void InGame::update(double t)
{
	if (t < 1.0) {
		m_fadeVal = 1.0 - t;
	} else {
		m_fadeVal = 0;
	}

	const auto& state = SDL_GetKeyboardState(nullptr);
	m_frontend->setInputState(ppvs::FeInput{
		state[SDL_SCANCODE_UP] != 0,
		state[SDL_SCANCODE_DOWN] != 0,
		state[SDL_SCANCODE_LEFT] != 0,
        state[SDL_SCANCODE_RIGHT] != 0,
        state[SDL_SCANCODE_X] != 0,
        state[SDL_SCANCODE_Z] != 0,
	});
	m_game->playGame();
}

void InGame::draw()
{
	m_game->renderGame();
	if (m_fadeVal != 0) {  // NOLINT(clang-diagnostic-float-equal)
		m_frontend->setColor(0, 0, 0, m_fadeVal * 255);
		m_frontend->drawRect(nullptr, -1000, -1000, 1000, 1000);
	}
	m_target->present();
}
