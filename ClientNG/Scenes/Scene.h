#pragma once
#include "../Renderers/Renderer.h"
#include <SDL_events.h>

namespace PuyoVS::ClientNG::Scenes {

class Scene {
public:
	explicit Scene(Renderers::RenderTarget* target)
		: m_target(target)
	{
	}
	virtual ~Scene() = default;

	virtual bool handleEvent(const SDL_Event& event) = 0;
	virtual void update(double t) = 0;
	virtual void draw() = 0;

protected:
    Renderers::RenderTarget* m_target;
};

}
