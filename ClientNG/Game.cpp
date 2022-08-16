#include "Game.h"
#include "Scenes/Scene.h"
#include "Scenes/Intro/IntroLogo.h"

#include <SDL_timer.h>
#include <stdexcept>

constexpr char kDefaultWindowTitle[] = "Puyo Puyo VS 2";
constexpr int kDefaultWindowWidth = 1280;
constexpr int kDefaultWindowHeight = 720;

GameWindow::GameWindow(Game* game, const WindowSettings& windowConfig)
	: m_game(game)
{
	Uint32 flags = 0;

	if (windowConfig.renderConfig.backend == RenderBackend::OpenGL) {
		flags |= SDL_WINDOW_OPENGL;
	} else if (windowConfig.renderConfig.backend == RenderBackend::Vulkan) {
		flags |= SDL_WINDOW_VULKAN;
	} else if (windowConfig.renderConfig.backend == RenderBackend::Metal) {
		flags |= SDL_WINDOW_METAL;
	}

	if (windowConfig.fullscreen) {
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	} else if (windowConfig.resizable) {
		flags |= SDL_WINDOW_RESIZABLE;
	}

	m_window = SDL_CreateWindow(
		windowConfig.title.c_str(),
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		windowConfig.width,
		windowConfig.height,
		flags);

	if (!m_window) {
		throw std::runtime_error("Failed to create window");
	}

	m_renderTarget = createRenderer(m_window, windowConfig.renderConfig);
}

GameWindow::~GameWindow()
{
	m_scene.reset();
	SDL_DestroyWindow(m_window);
}

void GameWindow::handleEvent(const SDL_Event& event)
{
	if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
		m_renderTarget->setViewport(event.window.data1, event.window.data2);
	}

	if (m_scene) {
		m_scene->handleEvent(event);
	}
}

void GameWindow::handleFrame()
{
	if (m_scene) {
		m_scene->update(m_t);
		m_scene->draw();
	} else {
		m_renderTarget->clear(0, 0, 0, 1);
	}

	// Temporary: it's a lie.
	m_t += 0.016;
}

void GameWindow::setScene(std::unique_ptr<Scene> scene)
{
	m_t = 0.0;
	m_scene = std::move(scene);
}

Uint32 GameWindow::id() const
{
	return SDL_GetWindowID(m_window);
}

Game::Game()
{
	WindowSettings settings;
	settings.title = kDefaultWindowTitle;
	settings.width = kDefaultWindowWidth;
	settings.height = kDefaultWindowHeight;
	settings.fullscreen = false;
	settings.resizable = true;
	settings.renderConfig.backend = RenderBackend::OpenGL;

	auto& window = m_windows.emplace_back(this, settings);
	m_windowById[window.id()] = &window;
	m_mainWindow = &window;
	m_activeWindow = &window;
	m_mainWindow->setScene(std::make_unique<IntroLogo>(*m_mainWindow));
}

void Game::handleEvent(const SDL_Event& event)
{
	switch (event.type) {
	case SDL_QUIT:
		m_running = false;
		break;

	case SDL_WINDOWEVENT:
		if (const auto window = m_windowById.find(event.window.windowID); window != m_windowById.end()) {
			window->second->handleEvent(event);
		}
		break;

	default:
		if (m_activeWindow) {
			m_activeWindow->handleEvent(event);
		}
	}
}

void Game::handleFrame()
{
	for (auto& window : m_windows) {
		window.handleFrame();
	}
}

void Game::iterate()
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		handleEvent(event);
	}
	handleFrame();
}

void Game::run()
{
	while (m_running) {
		iterate();

		// For now :) We'll handle time-step more seriously later...
		SDL_Delay(16);
	}
}
