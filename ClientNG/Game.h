#pragma once
#include "Renderers/Renderer.h"

#include <map>
#include <SDL_events.h>
#include <string>
#include <vector>

class Game;
class Scene;

struct WindowSettings {
	std::string title;
	int width;
	int height;
	int fullscreen;
	bool resizable;
	RenderSettings renderConfig;
};

class GameWindow final {
public:
	GameWindow(Game* game, const WindowSettings& windowConfig);
	~GameWindow();

	GameWindow(const GameWindow&) = delete;
	GameWindow& operator=(const GameWindow&) = delete;
	GameWindow(GameWindow&&) = default;
	GameWindow& operator=(GameWindow&&) = default;

	void handleEvent(const SDL_Event& event);
	void handleFrame();

	void setScene(std::unique_ptr<Scene> scene);
    [[nodiscard]] RenderTarget* renderTarget() const { return m_renderTarget; }
    [[nodiscard]] Uint32 id() const;

private:
	double m_t = 0.0;
	Game* m_game;
	SDL_Window* m_window;
	RenderTarget* m_renderTarget;
	std::unique_ptr<Scene> m_scene = nullptr;
};

class Game final {
public:
	Game();

	void handleEvent(const SDL_Event& event);
	void handleFrame();

	void iterate();
	void run();

private:
	GameWindow* m_mainWindow;
	GameWindow* m_activeWindow;
	std::map<Uint32, GameWindow*> m_windowById;
	std::vector<GameWindow> m_windows;
	bool m_running = true;
};
