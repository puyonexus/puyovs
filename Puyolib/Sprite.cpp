#include "Sprite.h"
#include "Frontend.h"
#include "global.h"
#include <algorithm>

using namespace std;

namespace ppvs {

Sprite::Sprite()
	: m_visible(true)
	, m_flipX(false)
	, m_flipY(false)
	, m_posX(0)
	, m_posY(0)
	, m_scaleX(1)
	, m_scaleY(1)
	, m_subRectX(0)
	, m_subRectY(0)
	, m_subRectW(0)
	, m_subRectH(0)
	, m_centerX(0)
	, m_centerY(0)
	, m_colorA(255)
	, m_colorR(255)
	, m_colorG(255)
	, m_colorB(255)
	, m_rotation(0)
	, m_blendMode(AlphaBlending)
	, m_image(nullptr)
	, m_colorRInternal(255)
	, m_colorGInternal(255)
	, m_colorBInternal(255)
	, m_colorAInternal(255)
{
}

Sprite::~Sprite() = default;

void Sprite::setImage(FeImage* img)
{
	m_image = img;
	m_subRectX = 0;
	m_subRectY = 0;
	if (m_image) {
		m_subRectW = m_image->width();
		m_subRectH = m_image->height();
	} else {
		m_subRectW = 1;
		m_subRectH = 1;
	}
}

void Sprite::setX(float x)
{
	m_posX = x;
}

void Sprite::setY(float y)
{
	m_posY = y;
}

void Sprite::addX(float x)
{
	m_posX += x;
}

void Sprite::addY(float y)
{
	m_posY += y;
}

void Sprite::setSubRect(int x, int y, int width, int height)
{
	m_subRectX = x;
	m_subRectY = y;
	m_subRectW = width;
	m_subRectH = height;
}

void Sprite::setCenter()
{
	m_centerX = m_subRectW / 2;
	m_centerY = m_subRectH / 2;
}

void Sprite::setCenter(int x, int y)
{
	m_centerX = x;
	m_centerY = y;
}

void Sprite::setCenter(const PosVectorFloat pv)
{
	m_centerX = static_cast<int>(pv.x);
	m_centerY = static_cast<int>(pv.y);
}

void Sprite::setCenterBottom()
{
	m_centerX = m_subRectW / 2;
	m_centerY = m_subRectH;
}

void Sprite::setScale(const float scale)
{
	m_scaleX = scale;
	m_scaleY = scale;
}

void Sprite::setScaleX(const float scale)
{
	m_scaleX = scale;
}
void Sprite::setScaleY(const float scale)
{
	m_scaleY = scale;
}
void Sprite::setScale(const float x, const float y)
{
	m_scaleX = x;
	m_scaleY = y;
}

void Sprite::setSize(const float x, const float y)
{
	if (m_subRectW != 0 && m_subRectH != 0) {
		m_scaleX = x / static_cast<float>(m_subRectW);
		m_scaleY = y / static_cast<float>(m_subRectH);
	}
}

void Sprite::setSize(const PosVectorFloat pv)
{
	setSize(pv.x, pv.y);
}

void Sprite::setPosition(const float x, const float y)
{
	m_posX = x;
	m_posY = y;
}

void Sprite::setPosition(const PosVectorFloat pv)
{
	m_posX = pv.x;
	m_posY = pv.y;
}

void Sprite::setRotation(const float angle)
{
	m_rotation = angle;
}

void Sprite::setTransparency(const float t)
{
	m_colorAInternal = static_cast<int>(t * 255.f);
}

void Sprite::setColor(const float r, const float g, const float b)
{
	m_colorRInternal = std::max(static_cast<int>(r), 0);
	m_colorGInternal = std::max(static_cast<int>(g), 0);
	m_colorBInternal = std::max(static_cast<int>(b), 0);
}

void Sprite::setColor(const float r, const float g, const float b, const float a)
{
	setColor(r, g, b);
	m_colorAInternal = std::max(static_cast<int>(a), 0);
}

void Sprite::setVisible(const bool visible)
{
	m_visible = visible;
}

void Sprite::setBlendMode(const BlendingMode blendMode)
{
	m_blendMode = blendMode;
}

void Sprite::setFlipX(const bool f)
{
	m_flipX = f;
}
void Sprite::setFlipY(const bool f)
{
	m_flipY = f;
}

void Sprite::setColor()
{
	m_colorR = m_colorRInternal % 256;
	m_colorG = m_colorGInternal % 256;
	m_colorB = m_colorBInternal % 256;
	m_colorA = m_colorAInternal % 256;
}

void Sprite::draw(FeRenderTarget* target, FeShader* shader)
{
	if (!m_visible) {
        return;
    }

    setColor();
	target->pushMatrix();
	float sCenterX = static_cast<float>(m_centerX) * m_scaleX,
		  sCenterY = static_cast<float>(m_centerY) * m_scaleY;

	if (m_flipX) {
        sCenterX = sCenterX - (static_cast<float>(m_subRectW) * m_scaleX);
    }
    if (m_flipY) {
        sCenterY = sCenterY - (static_cast<float>(m_subRectH) * m_scaleY);
    }

    // Translation
	target->translate(-sCenterX, -sCenterY, 0);
	target->translate(m_posX, m_posY, 0);

	// Rotation
	target->translate(sCenterX, sCenterY, 0);
	target->rotate(-m_rotation, 0, 0, 1);
	target->translate(-sCenterX, -sCenterY, 0);

	// Scaling
	target->scale(m_scaleX, m_scaleY, 1);

	// Flipping
	target->scale(m_flipX ? -1.f : 1.f, m_flipY ? -1.f : 1.f, 1.f);

	// Blending
	target->setBlendMode(m_blendMode);
	target->setColor(m_colorR, m_colorG, m_colorB, m_colorA);

	// Render
	if (shader) {
        shader->bind();
    }
    target->drawRect(m_image, m_subRectX, m_subRectY, m_subRectW, m_subRectH);
	if (shader) {
        shader->unbind();
    }

    target->popMatrix();
}

void Sprite::redraw(FeRenderTarget* target)
{
	target->setBlendMode(AdditiveBlending);
	if (m_colorRInternal > 255 || m_colorGInternal > 255 || m_colorBInternal > 255) {
		int cR = m_colorRInternal, cG = m_colorGInternal, cB = m_colorBInternal;
		while (cR > 255 || cG > 255 || cB > 255) {
			cR = max(cR - 255, 0);
			cG = max(cG - 255, 0);
			cB = max(cB - 255, 0);
			m_colorR = cR;
			m_colorG = cG;
			m_colorB = cB;
			target->drawRect(m_image, m_subRectX, m_subRectY, m_subRectW, m_subRectH);
		}
	}
	target->setBlendMode(AlphaBlending);
}

FeImage* Sprite::getImage() const
{
	return m_image;
}

float Sprite::getSizeX() const
{
	return static_cast<float>(m_subRectW);
}

float Sprite::getSizeY() const
{
	return static_cast<float>(m_subRectH);
}

float Sprite::getScaleX() const
{
	return m_scaleX;
}
float Sprite::getScaleY() const
{
	return m_scaleY;
}

float Sprite::getX() const
{
	return m_posX;
}

float Sprite::getY() const
{
	return m_posY;
}
float Sprite::getAngle() const
{
	return m_rotation;
}
float Sprite::getTransparency() const
{
	return static_cast<float>(m_colorAInternal) / 255.0f;
}
PosVectorFloat Sprite::getPosition() const
{
	return { getX(), getY() };
}

}
