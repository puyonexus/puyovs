#ifndef NEXTPUYO_H
#define NEXTPUYO_H

#include "global.h"
#include "sprite.h"
#include "puyo.h"
#include "movePuyo.h"
#include "dropPattern.h"
#include <deque>
namespace ppvs
{
class nextPuyo
{
    public:
        nextPuyo();
        ~nextPuyo();
        gameData* gamedata;
        void init(float x,float y,float scale,bool orientation,gameData* globalp);
        void initImage();
        void setPuyo();
        void play();
        void update(std::deque<int> &dq,puyoCharacter p,int turn);

        int getPuyo();
        int getOrientation();
        void draw();
        void idle();
        void test();

    protected:
    private:
    void setSprite(sprite&,sprite&,sprite&,sprite&,int&,int&,movePuyoType&);
    void resetPuyoPos();
    bool m_orientation;
    bool initialize;
    float m_offsetX,m_offsetY,m_scale;
    float m_pair1X,m_pair1Y,m_pair2X,m_pair2Y,m_pair3X,m_pair3Y;
    sprite m_background;
    sprite m_cutSprite;
    sprite m_sprite11,m_sprite12,m_sprite21,m_sprite22,m_sprite31,m_sprite32;
    sprite m_eye11,m_eye12,m_eye21,m_eye22,m_eye31,m_eye32;
    sprite m_final;

    int m_color11,m_color12,m_color21,m_color22,m_color31,m_color32;
    int m_goNext;
    int m_sign;
    movePuyoType m_type1,m_type2,m_type3;
};
}
#endif // NEXTPUYO_H
