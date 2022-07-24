#include "feverCounter.h"
#include <cmath>

namespace ppvs
{

feverCounter::feverCounter(gameData *gamedata)
{
	m_count = 0;
	m_realCount = m_count;
	m_init = false;
	timer = 0;
	maxSeconds = 30;
	updateTimer = 0;
	seconds = 60 * 15;
	m_positionSeconds.x = 0; m_positionSeconds.y = 0;
	m_visible = false;
	m_sparkle.setImage(gamedata->imgfsparkle);
	m_sparkle.setCenter();
	endless = false;
}

feverCounter::~feverCounter()
{
}

void feverCounter::init(float x, float y, float scale, bool orientation, gameData* g)
{
	data = g;
	m_init = true;
	m_scale = scale;
	m_orientation = orientation;
	// Set images
	for (int i = 0; i < 7; i++)
	{
		m_sprite[i].setImage(data->imgFeverGauge);
		m_glow[i].setImage(data->imgFeverGauge);
	}

	m_sprite[0].setSubRect(120, 0, 40, 40); m_glow[0].setSubRect(120, 40, 40, 40);
	m_sprite[1].setSubRect(120, 0, 40, 40); m_glow[1].setSubRect(120, 40, 40, 40);
	m_sprite[2].setSubRect(80, 0, 40, 40); m_glow[2].setSubRect(80, 40, 40, 40);
	m_sprite[3].setSubRect(80, 0, 40, 40); m_glow[3].setSubRect(80, 40, 40, 40);
	m_sprite[4].setSubRect(80, 0, 40, 40); m_glow[4].setSubRect(80, 40, 40, 40);
	m_sprite[5].setSubRect(40, 0, 40, 40); m_glow[5].setSubRect(40, 40, 40, 40);
	m_sprite[6].setSubRect(0, 0, 40, 40); m_glow[6].setSubRect(0, 40, 40, 40);
	for (int i = 0; i < 7; i++)
	{
		m_sprite[i].setCenter();
		m_glow[i].setCenter();
		m_sprite[i].setScale(m_scale);
		m_glow[i].setScale(m_scale);

	}
	m_sprite[0].setScale(0.8f * m_scale); m_glow[0].setScale(0.8f * m_scale);
	m_sprite[1].setScale(0.8f * m_scale); m_glow[1].setScale(0.8f * m_scale);
	m_sprite[2].setScale(0.8f * m_scale); m_glow[2].setScale(0.8f * m_scale);
	m_sprite[3].setScale(0.9f * m_scale); m_glow[3].setScale(0.9f * m_scale);

	// Align
	const int dir = -1 + 2 * orientation;
	constexpr int sy = 24;
	constexpr int sx = 12;
	m_sprite[0].setPosition(x + static_cast<float>(dir * sx) * 1.2f * scale, y - static_cast<float>(sy) * 1.5f * scale);
	m_sprite[1].setPosition(x + static_cast<float>(dir * sx) * 1.8f * scale, y - static_cast<float>(sy) * 2.5f * scale);
	m_sprite[2].setPosition(x + static_cast<float>(dir * sx) * 3.0f * scale, y - static_cast<float>(sy) * 3.3f * scale);
	m_sprite[3].setPosition(x + static_cast<float>(dir * sx) * 4.5f * scale, y - static_cast<float>(sy) * 4.3f * scale);
	m_sprite[4].setPosition(x + static_cast<float>(dir * sx) * 5.0f * scale, y - static_cast<float>(sy) * 5.5f * scale);
	m_sprite[5].setPosition(x + static_cast<float>(dir * sx) * 4.0f * scale, y - static_cast<float>(sy) * 6.8f * scale);
	m_sprite[6].setPosition(x + static_cast<float>(dir * sx) * 2.0f * scale, y - static_cast<float>(sy) * 8.0f * scale);
	for (int i = 0; i < 7; i++)
	{
		m_glow[i].setPosition(m_sprite[i].getX(), m_sprite[i].getY());
	}

	m_secondsSprite[0].setImage(data->imgSeconds);
	m_secondsSprite[1].setImage(data->imgSeconds);
	setSecondsSprite(static_cast<int>(std::ceil(static_cast<double>(seconds) / 60.)));
	m_positionSeconds.x = x + static_cast<float>(dir * sx) * 2.0f * scale;
	m_positionSeconds.y = y - static_cast<float>(sy) * 5.5f * scale;
	m_secondsSprite[0].setScale(scale);
	m_secondsSprite[1].setScale(scale);
	m_secondsSprite[0].setPosition(m_positionSeconds.x - 10 * scale, m_positionSeconds.y);
	m_secondsSprite[1].setPosition(m_positionSeconds.x + 10 * scale, m_positionSeconds.y);

}

void feverCounter::addTime(int t)
{
	seconds += t;
	if (seconds > maxSeconds * 60)
	{
		seconds = maxSeconds * 60;
	}
}

void feverCounter::update()
{
	// Update seconds
	setSecondsSprite(static_cast<int>(std::ceil(static_cast<float>(seconds) / 60.0f)));

	// Update gauge
	if (m_count != m_realCount)
	{
		updateTimer++;
		if (updateTimer > 44)
		{
			m_count = m_realCount;
			updateTimer = 0;
			data->snd.feverlight.Play(data);
		}
	}
}

void feverCounter::setPositionSeconds(posVectorFloat pv)
{
	float scale = m_secondsSprite[0].getScaleX();
	m_secondsSprite[0].setPosition(pv.x - 10 * scale, pv.y);
	m_secondsSprite[1].setPosition(pv.x + 10 * scale, pv.y);
}
void feverCounter::resetPositionSeconds()
{
	float scale = m_secondsSprite[0].getScaleX();
	m_secondsSprite[0].setPosition(m_positionSeconds.x - 10 * scale, m_positionSeconds.y);
	m_secondsSprite[1].setPosition(m_positionSeconds.x + 10 * scale, m_positionSeconds.y);
}

void feverCounter::setSecondsSprite(int t)
{
	if (t > 99)
		t = 99;
	int num1 = t % 10;
	int num2 = (t / 10) % 10;
	m_secondsSprite[1].setSubRect(20 * num1, 0, 20, 28);
	m_secondsSprite[0].setSubRect(20 * num2, 0, 20, 28);
	m_secondsSprite[0].setCenter();
	m_secondsSprite[1].setCenter();

}

posVectorFloat feverCounter::getPV()
{
	posVectorFloat pv;
	pv.x = m_sprite[m_realCount].getX();
	pv.y = m_sprite[m_realCount].getY();
	return pv;
}

void feverCounter::draw()
{
	if (!m_init || !m_visible)
		return;

	for (int i = 0; i < 7; i++)
	{
		const float col = static_cast<float>(50 * ((1000 + i * -10 + data->globalTimer) % 400 < 10));

		// Colorize
		if (m_count != 7 && i < m_count)
			m_sprite[i].setColor(255 - col, 255 - col, 0 + col);
		else if (m_count != 7 && i >= m_count)
			m_sprite[i].setColor(255 - col, 255 - col, 255 - col);
		else if (m_count >= 7)
			m_sprite[i].setColor(180, 215 + 40 * sin(2 * i + data->globalTimer / 16.f), 0 + 0 * sin(data->globalTimer / 8.f)); // Fever

		m_sparkle.setPosition(m_sprite[i].getX(), m_sprite[i].getY());
		m_sparkle.setRotation(m_sparkle.getAngle() + 1);

		// Flash
		if (data->glowShader)
			data->glowShader->setParameter("color", 0.1f * ((1000 + i * -10 + data->globalTimer) % 400 < 10));
		// Draw
		m_sprite[i].draw(data->front);
		m_glow[i].draw(data->front);
	}
	m_secondsSprite[0].draw(data->front);
	m_secondsSprite[1].draw(data->front);
}

}
