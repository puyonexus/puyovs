#pragma once

#include "fieldProp.h"
#include "dropPattern.h"

namespace ppvs
{
class player;
class AI
{
    public:
        AI(player *pl);
        virtual ~AI();
        void prepare(movePuyoType mpt,int color1,int color2);
        void findLargest();

        int bestPos;
        int bestRot;
        int bestChain;
        int timer;
        bool pinch;

    protected:
    private:
        int predictChain();
        void setRotation();
        movePuyoType m_type;
        posVectorInt m_pos[4];
        unsigned int m_rotation;
        int m_color1,m_color2;
        unsigned int m_bigColor;

        player *m_player;
};
}
