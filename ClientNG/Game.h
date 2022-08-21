#pragma once
#include "Renderers/Renderer.h"

#include <map>
#include <SDL_events.h>
#include <string>
#include <vector>

namespace PuyoVS::ClientNG {

class Game;

namespace Scenes {
    class Scene;
}

struct WindowSettings {
	std::string title;
	int width = 1024;
	int height = 768;
	int fullscreen = false;
	bool resizable = true;
	int frameSkip = 1;
    Renderers::RenderSettings renderConfig;
};

class GameWindow final {
public:
	GameWindow(Game* game, const WindowSettings& windowConfig);
	~GameWindow();

	GameWindow(const GameWindow&) = delete;
	GameWindow& operator=(const GameWindow&) = delete;
	GameWindow(GameWindow&&);
	GameWindow& operator=(GameWindow&&);

	void handleEvent(const SDL_Event& event);

    void update();
	void render();

	void setScene(std::unique_ptr<Scenes::Scene> scene);
    [[nodiscard]] Renderers::RenderTarget* renderTarget() const { return m_renderTarget; }
    [[nodiscard]] Uint32 id() const;

private:
	double m_t = 0.0;
	Game* m_game;
	SDL_Window* m_window;
    Renderers::RenderTarget* m_renderTarget;
	std::unique_ptr<Scenes::Scene> m_scene;
	int m_frameStep = 0;
	int m_frameSkip = 1;
};

class Game final {
public:
	Game();

	void handleEvent(const SDL_Event& event);

	void update();
	void render();
	void run();

private:
	GameWindow* m_mainWindow;
	GameWindow* m_activeWindow;
	std::map<Uint32, GameWindow*> m_windowById;
	std::vector<GameWindow> m_windows;
	bool m_running = true;
};

}
