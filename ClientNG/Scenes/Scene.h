#pragma once
#include "../Renderers/Renderer.h"
#include <SDL_events.h>

class Scene {
public:
	explicit Scene(RenderTarget* target)
		: m_target(target)
	{
	}
	virtual ~Scene() = default;

	virtual bool handleEvent(const SDL_Event& event) = 0;
	virtual void update(double t) = 0;
	virtual void draw() = 0;

protected:
	RenderTarget* m_target;
};
