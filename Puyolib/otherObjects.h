#ifndef OTHEROBJECTS_H
#define OTHEROBJECTS_H

#include "sprite.h"
#include "global.h"
#include "fieldProp.h"
#include "frontend.h"
#include <math.h>
namespace ppvs
{
class particle
{//the particles puyos spit out after popping
    public:
        particle(float,float,int,gameData*);
        ~particle();
        int getTimer() { return m_timer; }
        void draw(frendertarget*);
        void play();
        bool destroy();
    private:
        float m_posX,m_posY,m_speedX;
        float gravityTimer;
        float m_scale;
        int m_timer;
        sprite m_sprite;
};

class particleThrow
{//puyos that are "thrown" out
    public:
        particleThrow(float,float,int,gameData*);
        ~particleThrow();
        int getTimer() { return m_timer; }
        void draw(frendertarget*);
        void play();
        bool destroy();
    private:
        float m_posX,m_posY,m_speedX,m_rotateSpeed;
        float gravityTimer;
        float m_scale,m_rotate;
        int m_timer;
        sprite m_sprite;
};

class chainWord
{//object that shows the word XX chain
    public:
        chainWord(gameData* gamedata);
        ~chainWord();
        int getTimer() { return m_timer; }
        void setScale(float scale){m_scale=scale;}
        void draw(frendertarget* rw);
        void showAt(float x,float y,int n);
        void move();
    private:
        float m_posX,m_posY,m_scale;
        int m_timer,m_number;
        bool m_visible;
        sprite m_spriteN1,m_spriteN2,m_spriteChain;
};

class secondsObject
{//object that shows the word +XX seconds
    public:
        secondsObject(gameData*);
        ~secondsObject();
        int getTimer() { return m_timer; }
        void setScale(float scale){m_scale=scale;}
        void draw(frendertarget* rw);
        void showAt(float x,float y,int n);
        void move();
    private:
        float m_posX,m_posY,m_scale;
        int m_timer,m_number;
        bool m_visible;
        sprite m_spriteN1,m_spriteN2,m_spritePlus;
};

class lightEffect
{//light that represents garbage
  public:
        lightEffect(gameData *gamedata,posVectorFloat& startpv,posVectorFloat& middlepv, posVectorFloat &endpv);
        ~lightEffect();
        float getTimer(){return m_timer;}
        void setTimer(float timer);
        void setStart(posVectorFloat &pv){m_start=pv;}
        void setEnd(posVectorFloat &pv){m_end=pv;}
        void setMiddle(posVectorFloat &pv){m_middle=pv;}
        void draw(frendertarget* rw);
  private:
    bool m_visible;
    float m_timer; float m_traileffect;
    posVectorFloat m_end,m_start,m_middle;
    sprite m_sprite;
    sprite m_tail;
    sprite m_hitlight;
};

class feverLight
{//light that represents a count for the fevergauge
    public:
    feverLight(gameData *gamedata);
    ~feverLight();
    float getTimer(){return m_timer;}
    void setTimer(float timer);
    void init(posVectorFloat&,posVectorFloat&,posVectorFloat&);
    void draw(frendertarget *rw);
    private:
    bool m_visible;
    posVectorFloat m_end,m_start,m_middle;
    float m_timer;
    float m_speed;
    sprite m_sprite;
    sprite m_hitLight;
};

class nuisanceTray
{//the 6 nuisance puyos above the field
    public:
        nuisanceTray();
        ~nuisanceTray();
        void init(gameData*);
        gameData* gamedata;
        void align(float x,float y,float scale);
        void setVisible(bool);
        void update(int);
        void play();
        void draw();
        void setImage(int sprite,const int image);
        void setDarken(bool d){darken=d;}
    private:
        float m_timer;
        float m_animationTimer[6];
        float m_scale[6];
        float m_globalScale;
        float offsetX;
        float offsetY;
        sprite m_sprite[6];
        sprite m_back;
        bool darken;
};

class scoreCounter
{//score
    public:
        scoreCounter();
        ~scoreCounter();
        gameData* gamedata;
        void init(gameData*,float x, float y, float scale);
        void setCounter(int);
        void setPointBonus(int,int);
        void draw();
    private:
        int timer;
        int score;
        int point;
        int bonus;
        sprite m_sprite[9];
        void setImage(int spr,int score);
};

}
#endif // OTHEROBJECTS_H
