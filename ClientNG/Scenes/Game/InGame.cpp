#include "InGame.h"

#include "../../Game.h"
#include "Frontend.h"

namespace PuyoVS::ClientNG {

using ppvs::DebugMessageType;

// TODO: Move to the right place. Maybe our current handler is too smart but some current clients are just too dumb.
/* Handles printing debug information to the terminal */
void handlePuyolibDebugLog(std::string text, DebugMessageType sev)
{
	switch (sev) {
	case DebugMessageType::Debug:
		SDL_LogDebug(0, "Puyolib: %s", text.c_str());
		return;
	case DebugMessageType::Info:
		SDL_LogInfo(0, "Puyolib: %s", text.c_str());
		return;
	case DebugMessageType::Warning:
		SDL_LogWarn(0, "Puyolib: %s", text.c_str());
		return;
	case DebugMessageType::Error:
		SDL_LogError(0, "Puyolib: %s", text.c_str());
		return;
	case DebugMessageType::None:
	default:
		return;
	}
}

}

namespace PuyoVS::ClientNG::Scenes::Game {

InGame::InGame(GameWindow& w, std::unique_ptr<ppvs::Game> game)
	: Scene(w.renderTarget())
	, m_window(w)
	, m_game(std::move(game))
	, m_frontend(new GameFrontend(m_target))
{
	m_game->initGame(m_frontend, createAssetManager());
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
	m_frontend->setInputState(ppvs::FeInput {
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
	if (m_fadeVal != 0) { // NOLINT(clang-diagnostic-float-equal)
		m_frontend->setColor(0, 0, 0, m_fadeVal * 255);
		m_frontend->drawRect(nullptr, -1000, -1000, 1000, 1000);
	}
	m_target->present();
}

ppvs::AssetManagerPriv* InGame::createAssetManager()
{
	ppvs::AssetManager* manager = new ppvs::AssetManager(m_game->m_debug);
	// TODO: add bundles automatically

	auto* defaultBundle = new ppvs::FolderAssetBundle(new ppvs::GameAssetSettings(m_game->m_settings));
	manager->loadBundle(defaultBundle);

	auto* priv = manager->generate_priv(m_frontend, nullptr);
	priv->activate(m_frontend,nullptr);
	return priv;
}

}
