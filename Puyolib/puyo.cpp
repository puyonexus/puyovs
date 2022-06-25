#include <algorithm>
#include "puyo.h"
#include "field.h"
#include "player.h"
using namespace std;
namespace ppvs
{
//puyo::puyo()
//{
    //ctor
//}

puyo::puyo(int Xindex,int Yindex,int color,field* f,float spriteXreal,float spriteYreal,gameData* globalp)
{
    m_field=f;
    data=globalp;
    fieldProp prop=m_field->getProperties();

    //set position and color
    m_posX=Xindex;
    m_posY=Yindex;
    m_color=color;
    m_spriteX=spriteXreal;
    m_spriteY=spriteYreal;
    m_spriteYsc=0;
    m_scaleXd=1;
    m_scaleYd=1;
    m_scaleX=1;
    m_scaleY=1;
    m_scaleYcor=1;
    m_accelY=0;
    m_targetY=0;

    //set sprite
    m_sprite.setImage(data->imgPuyo);
    //correction factor in Y
    m_scaleYcor=(prop.gridHeight/float((PUYOY+prop.gridHeight)/2.01f));

    //other stuff
    dropable=true;
    m_type=NOPUYO;
    m_targetY=0;
    fallDelay=0;
    fallFlag=0;
    bounceFlag0=false;
    bounceFlag=0;
    searchBounce=0;
    bounceY=0;
    bounceTimer=2;
    bounceMultiplier=0;
    bottomY=0;
    glow=false;
    lastNuisance=false;
    hard=false;
    mark=false;
    destroy=false;
    m_destroyTimer=0;
    m_linkDown=false; m_linkUp=false; m_linkRight=false; m_linkLeft=false;
    temporary=false;
}

puyo::puyo(const puyo& self)
{//initialize copy of base class
    //copy only important things, no sprite stuff
    m_posX=self.GetindexX();
    m_posY=self.GetindexY();
    m_type=self.getType();
    m_color=self.getColor();
    dropable=self.dropable;
    mark=false;
    destroy=false;
    temporary=self.temporary;
}

puyo::~puyo()
{
    //dtor
}

puyo* puyo::clone()
{
    return new puyo(*this);
}

void puyo::AddAccelY(float val)
{//Add to acceleration and set position
    m_accelY+=val;
    m_spriteY+=m_accelY;
}

int puyo::getColor()const
{//Only colorpuyo return something valid
    return -1;
}

void puyo::updateSprite()
{
    //Bugfixes
    if (fallFlag!=0)
    {//there is a memory effect, nasty effects
        m_scaleX=1.0f;
        m_scaleY=1.0f;
        bounceY=0;
    }
    //weird scaling events
    if (m_scaleX>3.0 || m_scaleY>3.0 || m_scaleX<0.1)
    {
        m_scaleX=1.0f;
        m_scaleY=1.0f;
    }
    if (bounceY>PUYOY || bounceY<-PUYOY)
        bounceY=0;

    //Position
    fieldProp prop=m_field->getProperties();
    //note: puyo width is now 30 (to remove artifacts)
    m_sprite.setPosition(m_spriteX-m_posX*2+prop.gridX/2,m_spriteY+bounceY);

    //Scale
    m_sprite.setScale(m_scaleX*m_scaleXd,m_scaleY*m_scaleYd*m_scaleYcor);
}

void puyo::draw(frendertarget *target)
{//Draw puyo on screen
    updateSprite();
    if (!glow)
        m_sprite.draw(target);
    else if(data->glowShader)
        m_sprite.draw(target, data->glowShader);
}

void puyo::pop()
{//Popping animation
    if (destroy==true)
    {
        m_destroyTimer++;
        if (m_destroyTimer<10)
        {
            m_scaleXd=1.3f;
            m_scaleYd=1.3f;
            m_spriteYsc=m_spriteYsc+(m_scaleYd-1)/2.0*m_field->getProperties().gridHeight*1.0;
        }
        else
        {
            m_scaleXd=1.3-1.3*(m_destroyTimer-10)/(m_field->getPlayer()->chainPopSpeed*1.0);
            m_scaleYd=1.3-1.3*(m_destroyTimer-10)/(m_field->getPlayer()->chainPopSpeed*1.0);
            m_spriteYsc=m_spriteYsc+(m_scaleY-1)/2.0*m_field->getProperties().gridHeight*1.0;
        }

    }
    else
    {
        m_scaleXd=1.0f;
        m_scaleYd=1.0f;
        return;
    }
}

bool puyo::destroyPuyo()
{
    if (m_destroyTimer>m_field->getPlayer()->chainPopSpeed)
    {
        destroy=false;
        return true;
    }
    else
        return false;
}

void puyo::setLink(direction dir)
{//Connect puyo to another in some direction (though it can be done in base class, only has meaning for colorpuyo for now)
    switch(dir)
    {
        case BELOW:
            m_linkDown=true;
            break;
        case ABOVE:
            m_linkUp=true;
            break;
        case RIGHT:
            m_linkRight=true;
            break;
        case LEFT:
            m_linkLeft=true;
            break;
    }
}

void puyo::unsetLink(direction dir)
{//Unconnect puyo in some direction
    switch(dir)
    {
        case BELOW:
            m_linkDown=false;
            break;
        case ABOVE:
            m_linkUp=false;
            break;
        case RIGHT:
            m_linkRight=false;
            break;
        case LEFT:
            m_linkLeft=false;
            break;
    }
}

void puyo::landProper()
{//Don't remember what this is for actually, see MMF source code
    fallFlag=0;
    m_spriteY=m_targetY;
    m_accelY=0;
    bounceFlag=1;
    bounceTimer=2;
    searchBounce=1;
}

void puyo::bounce()
{//Do the bounce animation
    fieldProp prop=m_field->getProperties();
    if (hard==true)
    {//hard puyos do not bounce
        if (bounceTimer>0)
            bounceTimer+=20; //even hard puyos need to end their bounce
        return;
    }
    if (bounceTimer>2 && fallFlag==0)
    {//change scale
        m_scaleX=(((exp(bounceTimer*-1.0f/20.f)*bounceMultiplier*cos((bounceTimer*6-45)*PI/180.f)+1.05f)*(1-0.5f*exp(bounceTimer*-1/5.f))));
        m_scaleY=((-1*exp(bounceTimer*-1.0f/20.f)*bounceMultiplier*cos((bounceTimer*6-45)*PI/180.f)+1)+exp(bounceTimer/2.f*-1));
    }
    else if (fallFlag!=0)
    {
        m_scaleX=1.0f;
        m_scaleY=1.0f;
    }
    if (bounceTimer>0 && m_posY>0 && fallFlag==0)
    {//change y
        bounceY=(exp(bounceTimer*-1.f/20.f)*prop.gridHeight*1.14*min(1.2f-0.2f*(max(5-m_posY+bottomY,0)+1),bounceMultiplier)
                 *cos((bounceTimer*6-45)*PI/180.f)+1)+exp(bounceTimer*-1.0f/2.f);
    }
    else if (fallFlag!=0)
    {
        bounceY=0;
    }
    //set timer
    if (bounceTimer>0)
    bounceTimer+=m_field->getPlayer()->puyoBounceSpeed;
}


void puyo::setFallTarget(int target)
{//Calculate the position the puyo has to fall to
    fieldProp prop=m_field->getProperties();
    m_targetY=prop.gridHeight*((prop.gridY-3)-target);
}

void puyo::neighbourPop(field *,bool /*virt*/)
{//what should happen if a neighbouring puyo is popped
    return;
}

//=======================================
//colorpuyo
//=======================================

//colorPuyo::colorPuyo() : puyo()
//{
//
//}

colorPuyo::colorPuyo(int Xindex,int Yindex,int color,field* f,float spriteXreal,float spriteYreal,gameData* globalp)
: puyo(Xindex,Yindex,color,f,spriteXreal,spriteYreal,globalp)
{
    m_type=COLORPUYO;
}

colorPuyo::colorPuyo(const colorPuyo& self) : puyo(self)
{
    //copy colorpuyo attributes
}

colorPuyo* colorPuyo::clone()
{
    return new colorPuyo(*this);
}

int colorPuyo::getColor() const
{
    return m_color;
}

void colorPuyo::updateSprite()
{
    //Set rectangle
    float xRect=PUYOX*(m_linkDown+(m_linkUp<<1)+(m_linkRight<<2)+(m_linkLeft<<3));
    float yRect=PUYOY*m_color;
    //destroy animation
    if (destroy==true)
    {
        xRect=2*PUYOX*(m_color/2);
        yRect=12*PUYOY+PUYOY*(m_color%2);

    }
    m_sprite.setSubRect(xRect,yRect,PUYOX,PUYOY);
    m_sprite.setCenter(PUYOX/2,PUYOY);

    puyo::updateSprite();
}

void colorPuyo::draw(frendertarget *target)
{//Draw puyo on screen
    updateSprite();

    //Draw
    if (!glow)
    {
        m_sprite.draw(target);
    }
    else
    {
        if (data->glowShader)
        {
            data->glowShader->setParameter("color", 0.10f + 0.10f * sin(data->globalTimer / 8.f));
            m_sprite.draw(target, data->glowShader);
        }
        else
        {//without shaders
            m_sprite.setBlendMode(alphaBlending);
            m_sprite.draw(target);
            m_sprite.setBlendMode(additiveBlending);
            m_sprite.setTransparency(0.20f+0.20f*sin(data->globalTimer/8.f));
            m_sprite.draw(target);
            m_sprite.setTransparency(1);
            m_sprite.setBlendMode(alphaBlending);
        }
    }
}

void colorPuyo::pop()
{//Popping animation
    if (destroy==true)
    {
        glow=false;
        m_destroyTimer++;
        if (m_destroyTimer<10)
        {
            m_scaleXd=1.3f;
            m_scaleYd=1.3f;
            m_spriteYsc=m_spriteYsc+(m_scaleYd-1)/2.0*m_field->getProperties().gridHeight*1.0;
        }
        else
        {
            m_scaleXd=1.3-1.3*(m_destroyTimer-10)/(m_field->getPlayer()->chainPopSpeed*1.0);
            m_scaleYd=1.3-1.3*(m_destroyTimer-10)/(m_field->getPlayer()->chainPopSpeed*1.0);
            m_spriteYsc=m_spriteYsc+(m_scaleY-1)/2.0*m_field->getProperties().gridHeight*1.0;
        }
    }
    else
    {
        m_scaleXd=1.0f;
        m_scaleYd=1.0f;
        return;
    }
}

void colorPuyo::landProper()
{//Don't remember what this is for actually, see MMF source code
    fallFlag=0;
    m_spriteY=m_targetY;
    m_accelY=0;
    bounceFlag=1;
    bounceTimer=2;
    searchBounce=1;
}

bool colorPuyo::destroyPuyo()
{//Color puyo also chucks out particles before being destroyed
    if (m_destroyTimer>m_field->getPlayer()->chainPopSpeed)
    {
        //createParticles
        m_field->createParticle(m_spriteX,m_spriteY,m_color);
        m_field->createParticle(m_spriteX,m_spriteY,m_color);
        m_field->createParticle(m_spriteX,m_spriteY,m_color);
        destroy=false;
        return true;
    }
    else
        return false;
}

void colorPuyo::neighbourPop(field *,bool /*virt*/)
{//what should happen if a neighbouring puyo is popped
    return;
}

//=======================================
//nuiancepuyo
//=======================================

nuisancePuyo::nuisancePuyo(int Xindex,int Yindex,int color,field* f,float spriteXreal,float spriteYreal,gameData* globalp)
: puyo(Xindex,Yindex,color,f,spriteXreal,spriteYreal,globalp)
{
    m_type=NUISANCEPUYO;
}

nuisancePuyo::nuisancePuyo(const nuisancePuyo& self) : puyo(self)
{
    //copy nuisancepuyo attributes
}

nuisancePuyo* nuisancePuyo::clone()
{
    return new nuisancePuyo(*this);
}

void nuisancePuyo::landProper()
{
    fallFlag=0;
    m_spriteY=m_targetY;
    m_accelY=0;
    bounceFlag=0;
    bounceTimer=0;
    searchBounce=0;
}
void nuisancePuyo::updateSprite()
{
    m_sprite.setSubRect(PUYOX*6,PUYOY*12,PUYOX,PUYOY);
    m_sprite.setCenter(PUYOX/2,PUYOY);

    puyo::updateSprite();
}

void nuisancePuyo::draw(frendertarget *target)
{
    updateSprite();
    //Draw
    m_sprite.draw(target);
}

int nuisancePuyo::getColor() const
{
    return -1;
}

void nuisancePuyo::pop()
{//Popping animation
    if (destroy==true)
    {
        m_destroyTimer++;
        if (m_destroyTimer<10)
        {
            m_scaleXd=1.3f;
            m_scaleYd=1.3f;
            m_spriteYsc=m_spriteYsc+(m_scaleYd-1)/2.0*m_field->getProperties().gridHeight*1.0;
        }
        else
        {
            m_scaleXd=1.3-1.3*(m_destroyTimer-10)/(m_field->getPlayer()->chainPopSpeed*1.0);
            m_scaleYd=1.3-1.3*(m_destroyTimer-10)/(m_field->getPlayer()->chainPopSpeed*1.0);
            m_spriteYsc=m_spriteYsc+(m_scaleY-1)/2.0*m_field->getProperties().gridHeight*1.0;
        }
    }
    else
    {
        m_scaleXd=1;
        m_scaleYd=1;
        return;
    }
}

bool nuisancePuyo::destroyPuyo()
{
    if (m_destroyTimer>m_field->getPlayer()->chainPopSpeed)
    {
        destroy=false;
        return true;
    }
    else
        return false;
}

void nuisancePuyo::neighbourPop(field* f,bool v)
{//what should happen if a neighbouring puyo is popped
    if (v==false)
    {//delete self by adding to deletedpuyo list
        f->removePuyo(m_posX,m_posY);
        f->set(m_posX,m_posY,0);
    }
    else
    {//as a copy: unlink and delete self
        f->set(m_posX,m_posY,0);
        delete this;
    }

}
//=======================================
//hardpuyo
//=======================================

hardPuyo::hardPuyo(int Xindex,int Yindex,int color,field* f,float spriteXreal,float spriteYreal,gameData* globalp)
: puyo(Xindex,Yindex,color,f,spriteXreal,spriteYreal,globalp)
{
    m_type=HARDPUYO;
    hard=true;
    life=2;
    vlife=2;
}

hardPuyo::hardPuyo(const hardPuyo& self) : puyo(self)
{
    //copy hardPuyo attributes
    life=self.life;
    vlife=self.vlife;
}

hardPuyo* hardPuyo::clone()
{
    return new hardPuyo(*this);
}

void hardPuyo::landProper()
{
    fallFlag=0;
    m_spriteY=m_targetY;
    m_accelY=0;
    bounceFlag=0;
    bounceTimer=0;
    searchBounce=0;
}
void hardPuyo::updateSprite()
{
    if (life==2)
    {
        m_sprite.setSubRect(PUYOX*14,PUYOY*11,PUYOX,PUYOY);
        m_sprite.setCenter(PUYOX/2,PUYOY);
    }
    else
    {
        m_sprite.setSubRect(PUYOX*6,PUYOY*12,PUYOX,PUYOY);
        m_sprite.setCenter(PUYOX/2,PUYOY);
    }

    puyo::updateSprite();
}

void hardPuyo::draw(frendertarget *target)
{
    updateSprite();
    //Draw
    m_sprite.draw(target);
}

int hardPuyo::getColor() const
{
    return -1;
}

void hardPuyo::pop()
{//Popping animation
    if (destroy==true)
    {
        m_destroyTimer++;
        if (m_destroyTimer<10)
        {
            m_scaleXd=1.3f;
            m_scaleYd=1.3f;
            m_spriteYsc=m_spriteYsc+(m_scaleYd-1)/2.0*m_field->getProperties().gridHeight*1.0;
        }
        else
        {
            m_scaleXd=1.3-1.3*(m_destroyTimer-10)/(m_field->getPlayer()->chainPopSpeed*1.0);
            m_scaleYd=1.3-1.3*(m_destroyTimer-10)/(m_field->getPlayer()->chainPopSpeed*1.0);
            m_spriteYsc=m_spriteYsc+(m_scaleY-1)/2.0*m_field->getProperties().gridHeight*1.0;
        }
    }
    else
    {
        m_scaleXd=1;
        m_scaleYd=1;
        return;
    }
}

bool hardPuyo::destroyPuyo()
{
    if (m_destroyTimer>m_field->getPlayer()->chainPopSpeed)
    {
        destroy=false;
        return true;
    }
    else
        return false;
}

void hardPuyo::neighbourPop(field* f,bool v)
{//what should happen if a neighbouring puyo is popped
    if (v==false)
    {//delete self by adding to deletedpuyo list
        life-=1;
        if (life <=0)
        {
            f->removePuyo(m_posX,m_posY);
            f->set(m_posX,m_posY,0);
        }
    }
    else
    {//as a copy: unlink and delete self
        vlife-=1;
        if (vlife <=0)
        {
            f->set(m_posX,m_posY,0);
            delete this;
        }
    }

}
}
