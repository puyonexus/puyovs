#include "IntroNotes.h"
#include "../../../Puyolib/Game.h"
#include "../../../Puyolib/GameSettings.h"
#include "../Game/InGame.h"

#include "../../Game.h"
#include "../../PTSans.h"
#include "../../Paths.h"

namespace PuyoVS::ClientNG::Scenes::Intro {

namespace {

	constexpr char kIntroNotes[] = u8"Puyo VS 2 is open source software licensed under the GNU General Public "
								   u8"License v3.0. For more information about your rights, please consult the "
								   u8"included LICENSE.txt file. \u201cPuyo Puyo\u201d is a registered U.S. "
								   u8"trademark of SEGA CORPORATION. Puyo VS is not endorsed or related to SEGA "
								   u8"CORPORATION in any way.";

}

IntroNotes::IntroNotes(GameWindow& w)
	: Scene(w.renderTarget())
	, m_window(w)
{
	m_quadBuffer = m_target->makeBuffer();
	m_font = std::make_unique<Font>(kPTSansTTF, sizeof kPTSansTTF);
	m_font->setFontSize(36);
	m_text = m_target->makeTexture();
	m_text->renderText(*m_font, kIntroNotes, { 0xDD, 0xDD, 0xDD, 0xFF }, 750 * 2, m_textWidth, m_textHeight);

	m_quadVertices[0].tex = { 0, 1 };
	m_quadVertices[1].tex = { 1, 1 };
	m_quadVertices[2].tex = { 1, 0 };
	m_quadVertices[3].tex = { 0, 0 };
	m_quadVertices[0].pos = { -m_textWidth / 1600.0, -550.0 / 600.0, 0 };
	m_quadVertices[1].pos = { m_textWidth / 1600.0, -550.0 / 600.0, 0 };
	m_quadVertices[2].pos = { m_textWidth / 1600.0, (-550.0 + m_textHeight) / 600.0, 0 };
	m_quadVertices[3].pos = { -m_textWidth / 1600.0, (-550.0 + m_textHeight) / 600.0, 0 };

	for (auto& vertex : m_quadVertices) {
		vertex.color = { 1, 1, 1, 1 };
	}

	m_quadBuffer->uploadIndices(m_quadIndices, std::size(m_quadIndices));
}

IntroNotes::~IntroNotes() = default;

bool IntroNotes::handleEvent(const SDL_Event& event)
{
	// TODO: handle input properly; also, centralize 'skippable' scene logic
	if (event.type == SDL_KEYUP && (event.key.keysym.sym == SDLK_SPACE || event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER || event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_x)) {
		finish();
		return true;
	}
	return false;
}

void IntroNotes::update(double t)
{
	if (t > 6.0) {
		finish();
		return;
	}
	for (auto& vertex : m_quadVertices) {
		if (t < 1.0) {
			vertex.color.a = static_cast<float>(t);
		} else if (t < 4.0) {
			vertex.color.a = 1.0f;
		} else if (t < 5.0) {
			vertex.color.a = 5.0f - float(t);
		} else {
			vertex.color.a = 0.0f;
		}
	}
	m_quadBuffer->uploadVertices(m_quadVertices, std::size(m_quadVertices));
}

void IntroNotes::draw()
{
	m_target->clear(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
	m_text->bind(0);
	m_quadBuffer->render(Renderers::PolyShader::Simple);
	m_target->present();
}

void IntroNotes::finish()
{
	auto gs = new ppvs::GameSettings();
	gs->useCpuPlayers = true;
	gs->baseAssetDir = std::string(defaultAssetPath) + "/";
	auto dbg = new ppvs::DebugLog();
	dbg->setLogHandler([this](std::string text, ppvs::DebugMessageType sev) { PuyoVS::ClientNG::handlePuyolibDebugLog(text, sev); });
	m_window.setScene(std::make_unique<Game::InGame>(m_window, std::make_unique<ppvs::Game>(gs,dbg)));
}

}
