#include "IntroLogo.h"

#include "../../Game.h"
#include "../../PTSans.h"
#include "IntroNotes.h"
#include "Logo.h"

namespace PuyoVS::ClientNG::Scenes::Intro {

namespace {

	double easeOut(double x)
	{
		return x == 1 ? 1 : 1 - pow(2, -10 * x);
	}

	double easeInOutQuart(double x)
	{
		return x < 0.5 ? 8.0 * x * x * x * x : 1.0 - pow(-2.0 * x + 2.0, 4.0) / 2.0;
	}

}

IntroLogo::IntroLogo(GameWindow& w)
	: Scene(w.renderTarget())
	, m_window(w)
{
	m_quadBuffer = m_target->makeBuffer();
	m_logoTexture = m_target->makeTexture();
	m_logoTexture->loadPng(kLogoPng, sizeof kLogoPng);

	for (int i = 0; i < 9; i++) {
		const auto left = static_cast<double>(kLetterTexCoords[i][0]) / static_cast<double>(kLetterTexSize[0]);
		const auto right = left + static_cast<double>(kLetterTexCoords[i][2]) / static_cast<double>(kLetterTexSize[0]);
		const auto top = static_cast<double>(kLetterTexCoords[i][1]) / static_cast<double>(kLetterTexSize[1]);
		const auto bottom = top + static_cast<double>(kLetterTexCoords[i][3] / static_cast<double>(kLetterTexSize[1]));

		m_quadVertices[i * 4 + 0].tex = { left, top };
		m_quadVertices[i * 4 + 1].tex = { right, top };
		m_quadVertices[i * 4 + 2].tex = { right, bottom };
		m_quadVertices[i * 4 + 3].tex = { left, bottom };

		for (int j = 0; j < 4; j++) {
			m_quadVertices[i * 4 + j].color = { 1, 1, 1, 1 };
		}
	}

	m_quadBuffer->uploadIndices(m_quadIndices, std::size(m_quadIndices));
}

IntroLogo::~IntroLogo() = default;

bool IntroLogo::handleEvent(const SDL_Event& event)
{
	// TODO: handle input properly; also, centralize 'skippable' scene logic
	if (event.type == SDL_KEYUP && (event.key.keysym.sym == SDLK_SPACE || event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER || event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_x)) {
		finish();
		return true;
	}
	return false;
}

void IntroLogo::update(double t)
{
	const auto keyState = SDL_GetKeyboardState(nullptr);
	if (t > 4) {
		finish();
		return;
	}
	t /= 2;
	if (t < 1.5) {
		for (int i = 0; i < 9; i++) {
			auto tLocalTop = t - 0.025 * i;
			if (tLocalTop < 0.)
				tLocalTop = 0.;
			if (tLocalTop > 1.)
				tLocalTop = 1.;
			auto tLocalBottom = t - 0.030 * i;
			if (tLocalBottom < 0.)
				tLocalBottom = 0.;
			if (tLocalBottom > 1.)
				tLocalBottom = 1.;
			const auto th = easeOut(tLocalTop);
			const auto tb = easeOut(tLocalBottom);

			const auto endLeft = static_cast<double>(kLetterPositions[i][0]) / 800. - 1.;
			const auto endRight = endLeft + static_cast<double>(kLetterTexCoords[i][2]) / 800.;
			const auto top = 1. - static_cast<double>(kLetterPositions[i][1]) / 600.;
			const auto bottom = top - static_cast<double>(kLetterTexCoords[i][3]) / 600.;

			const auto startLeft = endLeft - 2.;
			const auto startRight = endRight - 2.;

			const auto leftTop = th * (endLeft - startLeft) + startLeft;
			const auto rightTop = th * (endRight - startRight) + startRight;
			const auto leftBottom = tb * (endLeft - startLeft) + startLeft;
			const auto rightBottom = tb * (endRight - startRight) + startRight;

			m_quadVertices[i * 4 + 0].pos.x = leftTop;
			m_quadVertices[i * 4 + 0].pos.y = top;
			m_quadVertices[i * 4 + 1].pos.x = rightTop;
			m_quadVertices[i * 4 + 1].pos.y = top;
			m_quadVertices[i * 4 + 2].pos.x = rightBottom;
			m_quadVertices[i * 4 + 2].pos.y = bottom;
			m_quadVertices[i * 4 + 3].pos.x = leftBottom;
			m_quadVertices[i * 4 + 3].pos.y = bottom;
		}
	}

	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 4; j++) {
			const auto localT = t * 2 - i * 0.05 - j * 0.01;
			if (localT < 0.)
				m_quadVertices[i * 4 + j].color.a = 0.;
			else if (localT < 1.)
				m_quadVertices[i * 4 + j].color.a = localT * localT;
			else if (localT < 2.)
				m_quadVertices[i * 4 + j].color.a = 1.;
			else if (localT >= 2. && localT < 2.5)
				m_quadVertices[i * 4 + j].color.a = easeInOutQuart((2.5 - localT) * 2.);
			else
				m_quadVertices[i * 4 + j].color.a = 0.;
		}
	}
	m_quadBuffer->uploadVertices(m_quadVertices, std::size(m_quadVertices));

	if (t < 1.25) {
		m_clearColor = { 0.15, 0.18, 0.20, 1.0 };
	} else if (t >= 1.25 && t < 1.75) {
		constexpr auto startRed = 0.15;
		constexpr auto startGreen = 0.18;
		constexpr auto startBlue = 0.20;
		constexpr auto endColor = 0.0;
		const auto localT = easeInOutQuart((t - 1.25) * 2.);
		m_clearColor = {
			localT * (endColor - startRed) + startRed,
			localT * (endColor - startGreen) + startGreen,
			localT * (endColor - startBlue) + startBlue,
			1.0
		};
	} else {
		m_clearColor = { 0.0, 0.0, 0.0, 1.0 };
	}
}

void IntroLogo::draw()
{
	m_target->clear(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
	m_logoTexture->bind(0);
	m_quadBuffer->render(Renderers::PolyShader::Simple);
	m_target->present();
}

void IntroLogo::finish()
{
	m_window.setScene(std::make_unique<IntroNotes>(m_window));
}

}
