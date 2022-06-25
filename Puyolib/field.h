#ifndef FIELD_H
#define FIELD_H

#include "fieldProp.h"
#include "puyo.h"
#include "otherObjects.h"
#include "global.h"
#include <vector>
#include <algorithm>

namespace ppvs
{
class game;
class player;

class field
{
    public:
        field();
        ~field();
        field& operator=(const field &other);
        void init(fieldProp properties,player*);
        gameData* data;
        fieldProp getProperties(){return m_properties;}
        void setProperties(fieldProp prop_in){m_properties=prop_in;}
        posVectorFloat getTopCoord(float offset=0);
        posVectorFloat getBottomCoord(bool s=false);
        posVectorFloat getGlobalCoord(int x,int y);
        posVectorFloat getLocalCoord(int x,int y);
        posVectorFloat getFieldSize();
        posVectorFloat getFieldScale();
        void setTransformScale(float x){m_transformScale=x;}
        float getFieldRotation(){return m_properties.angle;}

        player* getPlayer(){return m_player;}

        //============================
        //Puyo field related functions
        bool isEmpty(int x,int y);
        bool isPuyo(int x,int y);
        int getColor(int x,int y);
        bool addColorPuyo(int x,int y,int color,int fallFlag=0,int offset=0,int fallDelay=0);
        bool addNuisancePuyo(int x,int y,int fallFlag=0,int offset=0,int fallDelay=0);
        void drop();
        int dropSingle(int x, int y);
        void connectTrigger(int n,std::vector<int> &v);
        puyo* get(int x,int y);
        bool set(int x,int y,puyo*);
        void clearFieldVal(int x,int y);
        puyoType getPuyoType(int x,int y);
        void setLink(int x,int y,direction dir);
        void unsetLink(int x,int y,direction dir);
        bool findConnected(int x,int y,int n,std::vector<posVectorInt> &v);
        void unmark();
        void findConnectedLoop(posVectorInt pos,int &connected,std::vector<posVectorInt> &v);
        int count();
        int predictChain();
        void removePuyo(int x,int y);
        void clearField();

        //============================
        //Sprite related functions
        void setVisible(bool);
        void clear();
        void drawField();
        void draw();

        //============================
        //Gameplay related functions
        void createPuyo(); //Phase 20
        void searchFallDelay(); //Phase 20
        void unsetLinkAll(int x,int y);
        void dropPuyo();
        void fallPuyo();
        void bouncePuyo();
        void searchBounce(int x, int y, int posy);
        void searchLink(int x,int y);
        void endFallPuyoPhase();
        void searchChain(); //Phase 30
        void popPuyoAnim(); //Phase 32
        void dropGarbage(bool automatic=true,int dropAmount=0); //Phase 41
        void loseDrop(); //Phase 44
        void dropField(std::string fieldstring);
        void setFieldFromString(std::string fieldstring);
        std::string getFieldString();
        void throwAwayField();
        void test();


        //============================
        //Other objects
        void createParticle(float x,float y,int color);
        void createParticleThrow(puyo* p);
        void animateParticle();
        int getParticleNumber();
        void triggerGlow(posVectorInt shadowPos[4],int n,int colors[4]);
        int virtualChain(posVectorInt shadowPos[4],int n,int colors[4]);

    private:
        void createPuyoArray();
        void freePuyo(bool copy);
        void freePuyoArray();

        bool m_fieldInit;
        std::vector<particle*> m_particles;
        std::vector<particleThrow*> m_particlesThrow;
        std::vector<puyo*> m_deletedPuyo;
        player *m_player;
        fieldProp m_properties;
        float m_centerX, m_centerY;
        puyo ***fieldPuyoArray;
        puyo ***fieldPuyoArrayCopy; //useful for predicting chain

        //sprite related values
        sprite m_fieldSpriteFinal; //drawn onto screen
        bool m_visible;
        float m_posXreal,m_posYreal; //position of the  fieldsprite
        float m_transformScale; //fever transition

        //gameplay related values
        float m_sweepFall;
        std::vector<posVectorInt> m_vector;

};
}
#endif // FIELD_H
