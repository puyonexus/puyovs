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
    rotation(0), blendMode(alphaBlending), m_image(NULL),
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
    centerX=subRectW/2;
    centerY=subRectH/2;
}

void sprite::setCenter(int x,int y)
{
    centerX=x;
    centerY=y;
}

void sprite::setCenter(posVectorFloat pv)
{
    centerX=pv.x;
    centerY=pv.y;
}

void sprite::setCenterBottom()
{
    centerX = subRectW/2;
    centerY = subRectH;
}

void sprite::setScale(float x)
{
    scaleX=x;
    scaleY=x;
}

void sprite::setScaleX(float x)
{
    scaleX=x;
}
void sprite::setScaleY(float x)
{
    scaleY=x;
}
void sprite::setScale(float x,float y)
{
    scaleX=x;
    scaleY=y;
}

void sprite::setSize(float x,float y)
{
    float xx=subRectW;
    float yy=subRectH;
    if (xx!=0 && yy!=0)
	{
		scaleX=x/xx;
		scaleY=y/yy;
	}
}

void sprite::setSize(posVectorFloat pv)
{
    setSize(pv.x,pv.y);
}

void sprite::setPosition(float x,float y)
{
    posX=x;
    posY=y;
}

void sprite::setPosition(posVectorFloat pv)
{
    posX=pv.x; posY=pv.y;
}

void sprite::setRotation(float angle)
{
    rotation=angle;
}

void sprite::setTransparency(float t)
{
    m_colorA=t*255.f;
}

void sprite::setColor(float r,float g,float b)
{
    if(r<0)r=0;
    if(g<0)g=0;
    if(b<0)b=0;
    m_colorR=r;
    m_colorG=g;
    m_colorB=b;
}

void sprite::setColor(float r,float g,float b,float a)
{
    setColor(r,g,b);
    if(a<0)a=0;
    m_colorA=a;
}

void sprite::setVisible(bool b)
{
    m_visible=b;
}

void sprite::setBlendMode(blendingMode b)
{
    blendMode = b;
}

void sprite::setFlipX(bool f)
{
    flipX=f;
}
void sprite::setFlipY(bool f)
{
    flipY=f;
}

void sprite::setColor()
{
    colorR=m_colorR%256;
    colorG=m_colorG%256;
    colorB=m_colorB%256;
    colorA=m_colorA%256;
}

void sprite::draw(frendertarget *target, fshader *shader)
{
    if (!m_visible)
        return;

    setColor();
    target->pushMatrix();
    float scenterX = centerX * scaleX,
          scenterY = centerY * scaleY;

    if(flipX) scenterX = scenterX-(subRectW*scaleX);
    if(flipY) scenterY = scenterY-(subRectH*scaleY);

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
    target->scale(flipX?-1.:1., flipY?-1.:1., 1.);

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

fimage *sprite::getImage()
{
    return m_image;
}

float sprite::getSizeX()
{
    return subRectW;
}

float sprite::getSizeY()
{
    return subRectH;
}

float sprite::getScaleX()
{
    return scaleX;
}
float sprite::getScaleY()
{
    return scaleY;
}

float sprite::getX()
{
    return posX;
}

float sprite::getY()
{
    return posY;
}
float sprite::getAngle()
{
    return rotation;
}
float sprite::getTransparency()
{
    return m_colorA/255.0f;
}
posVectorFloat sprite::getPosition()
{
    return posVectorFloat(getX(),getY());
}

}
