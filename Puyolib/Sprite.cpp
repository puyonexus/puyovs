#include <algorithm>
#include "Sprite.h"
#include "global.h"
#include "Frontend.h"

using namespace std;

namespace ppvs
{

Sprite::Sprite() :
	m_visible(true),
	flipX(false), flipY(false),
	posX(0), posY(0),
	scaleX(1), scaleY(1),
	subRectX(0), subRectY(0), subRectW(0), subRectH(0),
	centerX(0), centerY(0),
	colorA(255), colorR(255), colorG(255), colorB(255),
	rotation(0), blendMode(AlphaBlending), m_image(nullptr),
	m_colorR(255), m_colorG(255), m_colorB(255), m_colorA(255)
{
}

Sprite::~Sprite()
{
}

void Sprite::setImage(FeImage* img)
{
	m_image = img;
	subRectX = 0;
	subRectY = 0;
	if (m_image)
	{
		subRectW = m_image->width();
		subRectH = m_image->height();
	}
	else
	{
		subRectW = 1;
		subRectH = 1;
	}
}

void Sprite::setX(float x)
{
	posX = x;
}

void Sprite::setY(float y)
{
	posY = y;
}

void Sprite::addX(float x)
{
	posX += x;
}

void Sprite::addY(float y)
{
	posY += y;
}

void Sprite::setSubRect(int x, int y, int width, int height)
{
	subRectX = x;
	subRectY = y;
	subRectW = width;
	subRectH = height;
}

void Sprite::setCenter()
{
	centerX = subRectW / 2;
	centerY = subRectH / 2;
}

void Sprite::setCenter(int x, int y)
{
	centerX = x;
	centerY = y;
}

void Sprite::setCenter(const PosVectorFloat pv)
{
	centerX = static_cast<int>(pv.x);
	centerY = static_cast<int>(pv.y);
}

void Sprite::setCenterBottom()
{
	centerX = subRectW / 2;
	centerY = subRectH;
}

void Sprite::setScale(float x)
{
	scaleX = x;
	scaleY = x;
}

void Sprite::setScaleX(float x)
{
	scaleX = x;
}
void Sprite::setScaleY(float x)
{
	scaleY = x;
}
void Sprite::setScale(float x, float y)
{
	scaleX = x;
	scaleY = y;
}

void Sprite::setSize(float x, float y)
{
	if (subRectW != 0 && subRectH != 0)
	{
		scaleX = x / static_cast<float>(subRectW);
		scaleY = y / static_cast<float>(subRectH);
	}
}

void Sprite::setSize(const PosVectorFloat pv)
{
	setSize(pv.x, pv.y);
}

void Sprite::setPosition(float x, float y)
{
	posX = x;
	posY = y;
}

void Sprite::setPosition(PosVectorFloat pv)
{
	posX = pv.x;
	posY = pv.y;
}

void Sprite::setRotation(float angle)
{
	rotation = angle;
}

void Sprite::setTransparency(float t)
{
	m_colorA = static_cast<int>(t * 255.f);
}

void Sprite::setColor(float r, float g, float b)
{
	m_colorR = std::max(static_cast<int>(r), 0);
	m_colorG = std::max(static_cast<int>(g), 0);
	m_colorB = std::max(static_cast<int>(b), 0);
}

void Sprite::setColor(float r, float g, float b, float a)
{
	setColor(r, g, b);
	m_colorA = std::max(static_cast<int>(a), 0);
}

void Sprite::setVisible(bool b)
{
	m_visible = b;
}

void Sprite::setBlendMode(BlendingMode b)
{
	blendMode = b;
}

void Sprite::setFlipX(bool f)
{
	flipX = f;
}
void Sprite::setFlipY(bool f)
{
	flipY = f;
}

void Sprite::setColor()
{
	colorR = m_colorR % 256;
	colorG = m_colorG % 256;
	colorB = m_colorB % 256;
	colorA = m_colorA % 256;
}

void Sprite::draw(FeRenderTarget* target, FeShader* shader)
{
	if (!m_visible)
		return;

	setColor();
	target->pushMatrix();
	float scenterX = static_cast<float>(centerX) * scaleX,
		scenterY = static_cast<float>(centerY) * scaleY;

	if (flipX) scenterX = scenterX - (static_cast<float>(subRectW) * scaleX);
	if (flipY) scenterY = scenterY - (static_cast<float>(subRectH) * scaleY);

	// Translation
	target->translate(-scenterX, -scenterY, 0);
	target->translate(posX, posY, 0);

	// Rotation
	target->translate(scenterX, scenterY, 0);
	target->rotate(-rotation, 0, 0, 1);
	target->translate(-scenterX, -scenterY, 0);

	// Scaling
	target->scale(scaleX, scaleY, 1);

	// Flipping
	target->scale(flipX ? -1.f : 1.f, flipY ? -1.f : 1.f, 1.f);

	// Blending
	target->setBlendMode(blendMode);
	target->setColor(colorR, colorG, colorB, colorA);

	// Render
	if (shader) shader->bind();
	target->drawRect(m_image, subRectX, subRectY, subRectW, subRectH);
	if (shader) shader->unbind();

	target->popMatrix();
}

void Sprite::redraw(FeRenderTarget* target)
{
	target->setBlendMode(AdditiveBlending);
	if (m_colorR > 255 || m_colorG > 255 || m_colorB > 255)
	{
		int cR = m_colorR, cG = m_colorG, cB = m_colorB;
		while (cR > 255 || cG > 255 || cB > 255)
		{
			cR = max(cR - 255, 0);
			cG = max(cG - 255, 0);
			cB = max(cB - 255, 0);
			colorR = cR; colorG = cG; colorB = cB;
			target->drawRect(m_image, subRectX, subRectY, subRectW, subRectH);
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
	return static_cast<float>(subRectW);
}

float Sprite::getSizeY() const
{
	return static_cast<float>(subRectH);
}

float Sprite::getScaleX() const
{
	return scaleX;
}
float Sprite::getScaleY() const
{
	return scaleY;
}

float Sprite::getX() const
{
	return posX;
}

float Sprite::getY() const
{
	return posY;
}
float Sprite::getAngle() const
{
	return rotation;
}
float Sprite::getTransparency() const
{
	return static_cast<float>(m_colorA) / 255.0f;
}
PosVectorFloat Sprite::getPosition() const
{
	return { getX(), getY() };
}

}
