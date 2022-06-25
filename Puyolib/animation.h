#ifndef ANIMATION_H
#define ANIMATION_H

#include "sprite.h"
#include "fieldProp.h"
#include "global.h"
#include "tinyxml.h"
#include <map>
#include <list>
#include <string>
#include <cmath>
#include <algorithm>

namespace ppvs
{

struct animationSprite
{//string float float
    sprite *spr;
    std::string parent;
    posVectorFloat position;
    posVectorFloat childOffset;
    posVectorFloat pathPos;
    posVectorFloat scale;
    float angle;
    float transparency;
    std::vector<std::string> children;
    animationSprite(): transparency(0.0f) {}
};

typedef std::vector<posVectorFloat> nodes;

class animation
{
    public:
        animation();
        virtual ~animation();
        void init(gameData *g,posVectorFloat offsetPos,float scale,std::string folder="",std::string scriptName="animation.xml",int maxTime=120);
        void prepareAnimation(std::string c){t=0;animationName=c;resetSprites();}
        void playAnimation();
        void draw();

        //callback functions
        void newSprite(std::string &name,std::string &image,std::string &before);
        void newSound(std::string &name,std::string &buffer);
        void addChild(std::string &parent,std::string &child,float x,float y);
        void setRect(std::string &name,float x,float y,float width,float height);
        void setCenter(std::string &name,float x,float y);
        void setPosition(std::string &name,float x,float y);
        void setBlendmode(std::string &name,std::string &blend);
        void setScaleX(std::string &name,float x);
        void setScaleY(std::string &name,float x);
        void setAngle(std::string &name,float x);
        void setTransparency(std::string &name,float x);
        void setVisible(std::string &name,bool x);
        void setColor(std::string &name,std::string &color,float x);
        void move(std::string &name,std::string &path,float x);
        void playSound(std::string &name);

        posVectorFloat offset;
        float globalScale;
    protected:
        bool spriteExists(std::string &name);
        void clearSprites();
        void clearSounds();
        void updateSprites();
        void updateChildren(std::string parent);
        void resetSprites();
        double getLocalTimer(std::string type,double startVal,double endVal,double t,double alpha=1,double beta=1);
        float getTotalDistance(std::string &path);
        posVectorFloat getPosition(std::string &path,float distance);
        posVectorFloat getPositionExtra(std::string &path,float distance,float total,bool start);

        int t;
        int duration;
        std::string animationName;
        std::map<std::string,animationSprite> sprites;
        std::list<animationSprite*> drawSprites;
        std::map<std::string,nodes> paths;
        std::map<std::string,sound*> sounds;
        gameData *gamedata;
//        player *m_player;
//        std::string currentCharacter;
        std::string sourceFolder;
        bool error;
        TiXmlDocument doc;
};

}
#endif // ANIMATION_H
