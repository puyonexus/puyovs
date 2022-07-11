#include <algorithm>
#include "sprite.h"
#include "global.h"
#include "frontend.h"

using namespace std;

namespace ppvs
{

sprite::sprite():
	m_visible(true),
	flipX(false),flipY(false),
	posX(0),posY(0),
	scaleX(1),scaleY(1),
	subRectX(0),subRectY(0),subRectW(0),subRectH(0),
    centerX(0),centerY(0),
    colorA(255),colorR(255),colorG(255),colorB(255),
    rotation(0), blendMode(alphaBlending), m_image(nullptr),
    m_colorR(255), m_colorG(255), m_colorB(255), m_colorA(255)
{
    //ctor
}

sprite::~sprite()
{
    //delete m_image;
}

void sprite::setImage(fimage *img)
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

void sprite::setX(float x)
{
    posX=x;
}

void sprite::setY(float y)
{
    posY=y;
}

void sprite::addX(float x)
{
    posX+=x;
}

void sprite::addY(float y)
{
    posY+=y;
}

void sprite::setSubRect(int x,int y,int width,int height)
{
    subRectX=x;
	subRectY=y;
	subRectW=width;
	subRectH=height;
}

void sprite::setCenter()
{
    centerX = subRectW/2;
    centerY = subRectH/2;
}

void sprite::setCenter(int x, int y)
{
    centerX = x;
    centerY = y;
}

void sprite::setCenter(const posVectorFloat pv)
{
    centerX = static_cast<int>(pv.x);
    centerY = static_cast<int>(pv.y);
}

void sprite::setCenterBottom()
{
    centerX = subRectW/2;
    centerY = subRectH;
}

void sprite::setScale(float x)
{
    scaleX = x;
    scaleY = x;
}

void sprite::setScaleX(float x)
{
    scaleX = x;
}
void sprite::setScaleY(float x)
{
    scaleY = x;
}
void sprite::setScale(float x,float y)
{
    scaleX = x;
    scaleY = y;
}

void sprite::setSize(float x, float y)
{
    if (subRectW != 0 && subRectH != 0)
	{
		scaleX = x / static_cast<float>(subRectW);
		scaleY = y / static_cast<float>(subRectH);
	}
}

void sprite::setSize(const posVectorFloat pv)
{
    setSize(pv.x, pv.y);
}

void sprite::setPosition(float x,float y)
{
    posX = x;
    posY = y;
}

void sprite::setPosition(posVectorFloat pv)
{
    posX = pv.x;
	posY = pv.y;
}

void sprite::setRotation(float angle)
{
    rotation = angle;
}

void sprite::setTransparency(float t)
{
    m_colorA = static_cast<int>(t * 255.f);
}

void sprite::setColor(float r,float g,float b)
{
    m_colorR = std::max(static_cast<int>(r), 0);
    m_colorG = std::max(static_cast<int>(g), 0);
    m_colorB = std::max(static_cast<int>(b), 0);
}

void sprite::setColor(float r,float g,float b,float a)
{
    setColor(r, g, b);
    m_colorA = std::max(static_cast<int>(a), 0);
}

void sprite::setVisible(bool b)
{
    m_visible = b;
}

void sprite::setBlendMode(blendingMode b)
{
    blendMode = b;
}

void sprite::setFlipX(bool f)
{
    flipX = f;
}
void sprite::setFlipY(bool f)
{
    flipY = f;
}

void sprite::setColor()
{
    colorR = m_colorR%256;
    colorG = m_colorG%256;
    colorB = m_colorB%256;
    colorA = m_colorA%256;
}

void sprite::draw(frendertarget *target, fshader *shader)
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
    target->scale(flipX?-1.f:1.f, flipY?-1.f:1.f, 1.f);

    // Blending
    target->setBlendMode(blendMode);
    target->setColor(colorR, colorG, colorB, colorA);

    // Render
    if(shader) shader->bind();
    target->drawRect(m_image, subRectX, subRectY, subRectW, subRectH);
    if(shader) shader->unbind();

    //redraw(target);
    target->popMatrix();
}

void sprite::redraw(frendertarget *target)
{
    target->setBlendMode(additiveBlending);
    if (m_colorR > 255 || m_colorG > 255 || m_colorB > 255)
    {
        int cR = m_colorR, cG = m_colorG, cB = m_colorB;
        while (cR>255 || cG>255 || cB>255)
        {
            cR = max(cR - 255, 0);
            cG = max(cG - 255, 0);
            cB = max(cB - 255, 0);
            colorR=cR;colorG=cG;colorB=cB;
            target->drawRect(m_image, subRectX, subRectY, subRectW, subRectH);
        }
    }
    target->setBlendMode(alphaBlending);
}

fimage *sprite::getImage() const
{
    return m_image;
}

float sprite::getSizeX() const
{
    return static_cast<float>(subRectW);
}

float sprite::getSizeY() const
{
    return static_cast<float>(subRectH);
}

float sprite::getScaleX() const
{
    return scaleX;
}
float sprite::getScaleY() const
{
    return scaleY;
}

float sprite::getX() const
{
    return posX;
}

float sprite::getY() const
{
    return posY;
}
float sprite::getAngle() const
{
    return rotation;
}
float sprite::getTransparency() const
{
    return static_cast<float>(m_colorA) / 255.0f;
}
posVectorFloat sprite::getPosition() const
{
    return {getX(), getY()};
}

}
