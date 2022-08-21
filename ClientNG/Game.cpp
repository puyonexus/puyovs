#include "Game.h"
#include "Scenes/Intro/IntroLogo.h"
#include "Scenes/Scene.h"

#include <SDL_timer.h>
#include <stdexcept>

namespace PuyoVS::ClientNG {

namespace {
    
constexpr char kDefaultWindowTitle[] = "Puyo Puyo VS 2";
constexpr int kDefaultWindowWidth = 1280;
constexpr int kDefaultWindowHeight = 720;

}

GameWindow::GameWindow(Game* game, const WindowSettings& windowConfig)
	: m_game(game)
{
	Uint32 flags = 0;

	if (windowConfig.renderConfig.backend == Renderers::RenderBackend::OpenGL) {
		flags |= SDL_WINDOW_OPENGL;
	} else if (windowConfig.renderConfig.backend == Renderers::RenderBackend::Vulkan) {
		flags |= SDL_WINDOW_VULKAN;
	} else if (windowConfig.renderConfig.backend == Renderers::RenderBackend::Metal) {
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
	m_frameSkip = windowConfig.frameSkip;
}

GameWindow::~GameWindow()
{
	m_scene.reset();
	SDL_DestroyWindow(m_window);
}

GameWindow::GameWindow(GameWindow&&) = default;

GameWindow& GameWindow::operator=(GameWindow&&) = default;

void GameWindow::handleEvent(const SDL_Event& event)
{
	if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
		m_renderTarget->setViewport(event.window.data1, event.window.data2);
	}

	if (m_scene) {
		m_scene->handleEvent(event);
	}
}

void GameWindow::update()
{
	if (m_scene) {
		m_scene->update(m_t);
	}

	// Temporary: it's a lie.
	m_t += 0.016;
}

void GameWindow::render()
{
	if (m_frameStep++ % m_frameSkip > 0) {
		return;
	}

	if (m_scene) {
		m_scene->draw();
	} else {
		m_renderTarget->clear(0, 0, 0, 1);
	}
}

void GameWindow::setScene(std::unique_ptr<Scenes::Scene> scene)
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
	settings.renderConfig.backend = Renderers::RenderBackend::OpenGL;

	auto& window = m_windows.emplace_back(this, settings);
	m_windowById[window.id()] = &window;
	m_mainWindow = &window;
	m_activeWindow = &window;
	m_mainWindow->setScene(std::make_unique<Scenes::Intro::IntroLogo>(*m_mainWindow));
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

void Game::update()
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		handleEvent(event);
	}
	for (auto& window : m_windows) {
		window.update();
	}
}

void Game::render()
{
	for (auto& window : m_windows) {
		window.render();
	}
}

void Game::run()
{
	size_t frameStep = 0;
	constexpr Sint64 frameDelays[] = { 16, 17, 17 };
	auto nextFrame = static_cast<Sint64>(SDL_GetTicks64());

	while (m_running) {
		const auto frameDelay = frameDelays[frameStep++ % std::size(frameDelays)];
		nextFrame += frameDelay;

		update();
		render();

		Sint64 drift = nextFrame - static_cast<Sint64>(SDL_GetTicks64());

		// If the drift becomes intolerable, just reset the clock.
		if (drift > 1000 || drift < -1000) {
			nextFrame = static_cast<Sint64>(SDL_GetTicks64());
			continue;
		}

		if (drift > 2) {
			SDL_Delay(static_cast<Uint32>(drift - 1));
		} else {
			while (drift < -frameDelays[0]) {
				const auto frameDelay = frameDelays[frameStep++ % std::size(frameDelays)];

				update();

				drift += frameDelay;
				nextFrame += frameDelay;
			}
		}
	}
}

}
