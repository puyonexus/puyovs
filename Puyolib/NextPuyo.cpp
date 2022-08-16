#include "NextPuyo.h"

namespace ppvs
{

NextPuyo::NextPuyo()
{
	// nextPuyo window is 65 pixels wide, 124 pixels high
	m_data = nullptr;
	m_scale = 1;
	m_goNext = 16;
	m_color11 = 0; m_color12 = 0;
	m_color21 = 0; m_color22 = 0;
	m_color31 = 0; m_color32 = 0;
	resetPuyoPos();
	m_offsetX = 0; m_offsetY = 0;
	m_orientation = true; // orientation sets nextPuyo window left or right
	m_sign = 1;
	m_initialize = false;
}

NextPuyo::~NextPuyo() = default;

int NextPuyo::getOrientation() const
{
	return m_sign;
}

void NextPuyo::init(const float x, const float y, const float scale, const bool orientation, GameData* data)
{
	m_data = data;

	initImage();
	m_offsetX = x; m_offsetY = y;
	m_scale = scale;
	m_orientation = orientation;
	m_sign = static_cast<int>(orientation) * 2 - 1;

	if (m_initialize)
		return;

	if (m_data)
	{
		if (orientation)
		{
			m_background.setImage(m_data->imgNextPuyoBackgroundR);
		}
		else
		{
			m_background.setImage(m_data->imgNextPuyoBackgroundL);
		}
	}

	m_background.setSubRect(0, 0, 65, 124);

	m_initialize = true;
	resetPuyoPos();
	m_sprite31.setScale(33.0 / 48.0);
	m_sprite32.setScale(33.0 / 48.0);
}

void NextPuyo::initImage()
{
	if (m_data)
	{
		m_sprite11.setImage(m_data->imgPuyo); m_sprite12.setImage(m_data->imgPuyo);
		m_sprite21.setImage(m_data->imgPuyo); m_sprite22.setImage(m_data->imgPuyo);
		m_sprite31.setImage(m_data->imgPuyo); m_sprite32.setImage(m_data->imgPuyo);
		m_eye11.setImage(m_data->imgPuyo); m_eye12.setImage(m_data->imgPuyo);
		m_eye21.setImage(m_data->imgPuyo); m_eye22.setImage(m_data->imgPuyo);
		m_eye31.setImage(m_data->imgPuyo); m_eye32.setImage(m_data->imgPuyo);
	}

    constexpr MovePuyoType mpt = MovePuyoType::DOUBLET;
	setSprite(m_sprite11, m_sprite12, m_eye11, m_eye12, m_color11, m_color12, mpt);
	setSprite(m_sprite21, m_sprite22, m_eye21, m_eye22, m_color21, m_color22, mpt);
	setSprite(m_sprite31, m_sprite32, m_eye31, m_eye32, m_color31, m_color32, mpt);

	m_cutSprite.setBlendMode(BlendingMode::MultiplyBlending);

}

// Set correct sprites
void NextPuyo::setPuyo()
{
	setSprite(m_sprite11, m_sprite12, m_eye11, m_eye12, m_color11, m_color12, m_type1);
	setSprite(m_sprite21, m_sprite22, m_eye21, m_eye22, m_color21, m_color22, m_type2);
	setSprite(m_sprite31, m_sprite32, m_eye31, m_eye32, m_color31, m_color32, m_type3);
}

// Move nextPuyo
void NextPuyo::play()
{
	if (m_goNext >= 16 && m_pair1Y > 0)
	{
		m_pair1Y -= 4;
	}

	if (m_goNext < 16)
	{
		m_pair1Y -= 6.f;
		m_pair3Y -= 4.f;
		m_pair2X -= 22.0f / 16.0f * static_cast<float>(m_sign);
		m_pair2Y -= 56.0f / 16.0f;

		m_sprite21.setScale((33.0f + static_cast<float>(m_goNext)) / 48.0f);
		m_sprite22.setScale((33.0f + static_cast<float>(m_goNext)) / 48.0f);

		++m_goNext;
	}
}

// Set colors of puyo to player's deque
void NextPuyo::update(const std::deque<int>& dq, const PuyoCharacter p, const int turn)
{
	resetPuyoPos();
	m_goNext = 0;

	// Set colors and call setPuyo
	// The list must be at least 6 big
	if (dq.size() >= 6)
	{
		m_color11 = *(dq.begin()); m_color12 = *(dq.begin() + 1);
		m_color21 = *(dq.begin() + 2); m_color22 = *(dq.begin() + 3);
		m_color31 = *(dq.begin() + 4); m_color32 = *(dq.begin() + 5);
		m_type1 = getFromDropPattern(p, turn);
		m_type2 = getFromDropPattern(p, turn + 1);
		m_type3 = getFromDropPattern(p, turn + 2);
		setPuyo();
	}
	else
	{
		// list wasn't initialized -> set invisible?
	}
}

void NextPuyo::draw()
{
	// Set positions of sprites
	m_sprite11.setPosition(m_pair1X, m_pair1Y); m_sprite12.setPosition(m_pair1X, m_pair1Y);
	m_sprite21.setPosition(m_pair2X, m_pair2Y); m_sprite22.setPosition(m_pair2X, m_pair2Y);
	m_sprite31.setPosition(m_pair3X, m_pair3Y); m_sprite32.setPosition(m_pair3X, m_pair3Y);
	m_eye11.setPosition(m_pair1X, m_pair1Y); m_eye12.setPosition(m_pair1X, m_pair1Y);
	m_eye21.setPosition(m_pair2X, m_pair2Y); m_eye22.setPosition(m_pair2X, m_pair2Y);
	m_eye31.setPosition(m_pair3X, m_pair3Y); m_eye32.setPosition(m_pair3X, m_pair3Y);

	// Copy background (consider copying image without alpha channel)
	if (!m_data)
		return;
	m_data->front->clearDepth();
	m_data->front->setDepthFunction(DepthFunction::LessOrEqual);
	m_data->front->enableAlphaTesting(0.1f);
	m_background.draw(m_data->front);

	// Draw puyos
	m_data->front->setDepthFunction(DepthFunction::Equal);
	m_sprite11.draw(m_data->front);
	m_sprite12.draw(m_data->front);
	m_sprite21.draw(m_data->front);
	m_sprite22.draw(m_data->front);
	m_sprite31.draw(m_data->front);
	m_sprite32.draw(m_data->front);

	// Set eyes
	m_eye21.setScale(m_sprite21.getScaleX());
	m_eye22.setScale(m_sprite22.getScaleX());
	m_eye31.setScale(m_sprite31.getScaleX());
	m_eye32.setScale(m_sprite32.getScaleX());

	m_eye11.draw(m_data->front);
	m_eye12.draw(m_data->front);
	m_eye21.draw(m_data->front);
	m_eye22.draw(m_data->front);
	m_eye31.draw(m_data->front);
	m_eye32.draw(m_data->front);
	m_data->front->setDepthFunction(DepthFunction::Always);
	m_data->front->disableAlphaTesting();
}

// Almost a direct copy from MovePuyo.cpp. Difference in centers.
void NextPuyo::setSprite(Sprite& sprite1, Sprite& sprite2, Sprite& spriteEye1, Sprite& spriteEye2, const int& color1, const int& color2, const MovePuyoType& type) const
{
	int subRectX1, subRectY1, subRectWidth1, subRectHeight1, subRectX2, subRectY2, subRectWidth2, subRectHeight2;
	const int m_bigColor = color1;
	if (type == MovePuyoType::DOUBLET)
	{
		subRectX1 = 0; subRectY1 = kPuyoY * color1;
		subRectWidth1 = kPuyoX; subRectHeight1 = kPuyoY;
		subRectX2 = 0; subRectY2 = kPuyoY * color2;
		subRectWidth2 = kPuyoX; subRectHeight2 = kPuyoY;
		sprite1.setSubRect(subRectX1, subRectY1, subRectWidth1, subRectHeight1);
		sprite2.setSubRect(subRectX2, subRectY2, subRectWidth2, subRectHeight2);
		sprite1.setCenter(kPuyoX / 2, 0);
		sprite2.setCenter(kPuyoX / 2, kPuyoY);
		sprite1.setVisible(true);
		sprite2.setVisible(true);
		spriteEye1.setVisible(false);
		spriteEye2.setVisible(false);
	}
	else if (type == MovePuyoType::TRIPLET && color1 != color2)
	{
		subRectX1 = kPuyoX * color1; subRectY1 = kPuyoY * 5 + 1;
		subRectWidth1 = kPuyoX; subRectHeight1 = kPuyoY * 2 - 1;
		subRectX2 = 0; subRectY2 = kPuyoY * color2;
		subRectWidth2 = kPuyoX; subRectHeight2 = kPuyoY;
		sprite1.setSubRect(subRectX1, subRectY1, subRectWidth1, subRectHeight1);
		sprite2.setSubRect(subRectX2, subRectY2, subRectWidth2, subRectHeight2);
		sprite1.setCenter(kPuyoX, kPuyoY);
		sprite2.setCenter(0, 0);
		sprite1.setVisible(true);
		sprite2.setVisible(true);
		spriteEye1.setSubRect(kPuyoX + 2 * kPuyoX * ((color1 / 2)), 12 * kPuyoY + kPuyoY * (color1 % 2), kPuyoX, kPuyoY);
		spriteEye2.setSubRect(kPuyoX + 2 * kPuyoX * ((color2 / 2)), 12 * kPuyoY + kPuyoY * (color2 % 2), kPuyoX, kPuyoY);
		spriteEye1.setCenter(kPuyoX, kPuyoY);
		spriteEye2.setCenter(0, 0);
		spriteEye1.setVisible(true);
		spriteEye2.setVisible(false);
	}
	else if ((type == MovePuyoType::TRIPLET || type == MovePuyoType::TRIPLET_R) && color1 == color2)
	{
		subRectX1 = 5 * kPuyoX + 2 * kPuyoX * color1; subRectY1 = kPuyoY * 5 + 1;
		subRectWidth1 = kPuyoX * 2; subRectHeight1 = kPuyoY * 2 - 1;
		subRectX2 = 0; subRectY2 = kPuyoY * color2;
		subRectWidth2 = kPuyoX; subRectHeight2 = kPuyoY;
		sprite1.setSubRect(subRectX1, subRectY1, subRectWidth1, subRectHeight1);
		sprite2.setSubRect(subRectX2, subRectY2, subRectWidth2, subRectHeight2);
		sprite1.setCenter(kPuyoX, kPuyoY);
		sprite2.setCenter(0, 0);
		sprite1.setVisible(true);
		sprite2.setVisible(false);
		spriteEye1.setSubRect(kPuyoX + 2 * kPuyoX * ((color1 / 2)), 12 * kPuyoY + kPuyoY * (color1 % 2), kPuyoX, kPuyoY);
		spriteEye2.setSubRect(kPuyoX + 2 * kPuyoX * ((color2 / 2)), 12 * kPuyoY + kPuyoY * (color2 % 2), kPuyoX, kPuyoY);
		spriteEye1.setCenter(kPuyoX, kPuyoY);
		spriteEye2.setCenter(0, 0);
		spriteEye1.setVisible(true);
		spriteEye2.setVisible(false);
	}
	else if (type == MovePuyoType::TRIPLET_R && color1 != color2)
	{
		subRectX1 = kPuyoX * color1; subRectY1 = kPuyoY * 5 + 1;
		subRectWidth1 = kPuyoX; subRectHeight1 = kPuyoY * 2 - 1;
		subRectX2 = 0; subRectY2 = kPuyoY * color2;
		subRectWidth2 = kPuyoX; subRectHeight2 = kPuyoY;
		sprite1.setSubRect(subRectX1, subRectY1, subRectWidth1, subRectHeight1);
		sprite2.setSubRect(subRectX2, subRectY2, subRectWidth2, subRectHeight2);
		sprite1.setCenter(0, kPuyoY);
		sprite2.setCenter(kPuyoX, kPuyoY);
		sprite1.setVisible(true);
		sprite2.setVisible(true);
		spriteEye1.setSubRect(kPuyoX + 2 * kPuyoX * ((color1 / 2)), 12 * kPuyoY + kPuyoY * (color1 % 2), kPuyoX, kPuyoY);
		spriteEye2.setSubRect(kPuyoX + 2 * kPuyoX * ((color2 / 2)), 12 * kPuyoY + kPuyoY * (color2 % 2), kPuyoX, kPuyoY);
		spriteEye1.setCenter(0, 0);
		spriteEye2.setCenter(kPuyoX, kPuyoY);
		spriteEye1.setVisible(true);
		spriteEye2.setVisible(false);

	}
	else if (type == MovePuyoType::QUADRUPLET)
	{
		subRectX1 = static_cast<int>(10.f * static_cast<float>(kPuyoX) + (static_cast<float>(kPuyoX) + static_cast<float>(kPuyoX) / 6.4f) * static_cast<float>(color1)); subRectY1 = kPuyoY * 13;
		subRectWidth1 = static_cast<int>(static_cast<float>(kPuyoX) + static_cast<float>(kPuyoX) / 6.4f); subRectHeight1 = kPuyoY * 2;
		subRectX2 = static_cast<int>(10.f * static_cast<float>(kPuyoX) + (static_cast<float>(kPuyoX) + static_cast<float>(kPuyoX) / 6.4f) * static_cast<float>(color2)); subRectY2 = kPuyoY * 13;
		subRectWidth2 = static_cast<int>(static_cast<float>(kPuyoX) + static_cast<float>(kPuyoX) / 6.4f); subRectHeight2 = kPuyoY * 2;
		sprite2.setSubRect(subRectX1, subRectY1, subRectWidth1, subRectHeight1);
		sprite1.setSubRect(subRectX2, subRectY2, subRectWidth2, subRectHeight2);
		if (m_data)
		{
			sprite2.setCenter(m_data->quadrupletCenter, kPuyoY);
			sprite1.setCenter(m_data->quadrupletCenter, kPuyoY);
		}
		sprite1.setVisible(true);
		sprite2.setVisible(true);
		spriteEye1.setSubRect(kPuyoX + 2 * kPuyoX * ((color1 / 2)), 12 * kPuyoY + kPuyoY * (color1 % 2), kPuyoX, kPuyoY);
		spriteEye2.setSubRect(kPuyoX + 2 * kPuyoX * ((color2 / 2)), 12 * kPuyoY + kPuyoY * (color2 % 2), kPuyoX, kPuyoY);
		spriteEye2.setCenter(kPuyoX - 10, kPuyoY - 4);
		spriteEye1.setCenter(0 + 10, 0 + 4);
		spriteEye1.setVisible(true);
		spriteEye2.setVisible(true);
	}
	else if (type == MovePuyoType::BIG)
	{
		subRectX1 = 2 * kPuyoX + 2 * kPuyoX * m_bigColor; subRectY1 = kPuyoY * 7;
		subRectWidth1 = kPuyoX * 2; subRectHeight1 = kPuyoY * 2;
		subRectX2 = 0; subRectY2 = 0;
		subRectWidth2 = 0; subRectHeight2 = 0;
		sprite1.setSubRect(subRectX1, subRectY1, subRectWidth1, subRectHeight1);
		sprite2.setSubRect(subRectX2, subRectY2, subRectWidth2, subRectHeight2);
		sprite1.setCenter(kPuyoX, kPuyoY);
		sprite2.setCenter(kPuyoX, kPuyoY);
		sprite1.setVisible(true);
		sprite2.setVisible(false);
		spriteEye1.setVisible(false);
		spriteEye2.setVisible(false);
	}
	if (type == MovePuyoType::TRIPLET_R && color1 != color2)
	{
		sprite1.setRotation(-90);
		sprite2.setRotation(0);
	}
	else if (type == MovePuyoType::QUADRUPLET)
	{
		sprite1.setRotation(0);
		sprite2.setRotation(180);
	}
	else
	{
		sprite1.setRotation(0);
		sprite2.setRotation(0);
	}
}

// Reset position when not playing
void NextPuyo::idle()
{
	m_goNext = 0;
	resetPuyoPos();
}

void NextPuyo::test()
{
	m_goNext = 0;
	resetPuyoPos();
}

void NextPuyo::resetPuyoPos()
{
	m_pair1X = static_cast<float>(!m_orientation) * 65 + static_cast<float>(m_sign) * 25; m_pair1Y = 40;
	m_pair2X = static_cast<float>(!m_orientation) * 65 + static_cast<float>(m_sign) * 47; m_pair2Y = 96;
	m_pair3X = static_cast<float>(!m_orientation) * 65 + static_cast<float>(m_sign) * 47; m_pair3Y = 160;
	m_sprite21.setScale(33.0f / 48.0f);
	m_sprite22.setScale(33.0f / 48.0f);
}

}
