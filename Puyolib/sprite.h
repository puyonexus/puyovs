#pragma once

#include "fieldProp.h"
#include "frontend.h"

namespace ppvs
{

class frendertarget;
class fimage;

class sprite
{
public:
    sprite();
    ~sprite();

    //public variables
    bool m_visible;
    bool flipX;
    bool flipY;
    float posX,posY;
    float scaleX,scaleY;
    int subRectX,subRectY,subRectW,subRectH;
    int centerX, centerY;
    int colorA, colorR, colorG, colorB;
    float rotation;
    blendingMode blendMode;

    //functions (set)
    void setImage(fimage *img);
    void setX(float);
    void setY(float);
    void addX(float);
    void addY(float);
    void setSubRect(int x,int y,int width,int height);
    void setCenter();
    void setCenter(int x,int y);
    void setCenter(posVectorFloat pv);
    void setCenterBottom();
    void setScale(float scale);
    void setScaleX(float scale);
    void setScaleY(float scale);
    void setScale(float x,float y);
    void setSize(float x,float y);
    void setSize(posVectorFloat x);
    void setPosition(float,float);
    void setPosition(posVectorFloat pv);
    void setRotation(float);
    void setTransparency(float);
    void setColor(float r,float g,float b,float a);
    void setColor(float r,float g,float b);
    void setVisible(bool);
    void setBlendMode(blendingMode);
    void setFlipX(bool f);
    void setFlipY(bool f);

    void draw(frendertarget *t, fshader *s = nullptr);
    void redraw(frendertarget *t); //draw again with blendmode add

    float getSizeX() const;
    float getSizeY() const;
    float getScaleX() const;
    float getScaleY() const;
    posVectorFloat getPosition() const;
    float getX() const;
    float getY() const;
    float getAngle() const;
    float getTransparency() const;
    fimage *getImage() const;
protected:
private:
    void properFlip(bool x,bool y);
    void setColor();
    fimage *m_image;
    int m_colorR,m_colorG,m_colorB,m_colorA;

};

}
