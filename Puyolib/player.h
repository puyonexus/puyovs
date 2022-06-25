#ifndef PLAYER_H
#define PLAYER_H

#include "controller.h"
#include "movePuyo.h"
#include "fieldProp.h"
#include "field.h"
#include "ruleset.h"
#include "dropPattern.h"
#include "nextPuyo.h"
#include "feverCounter.h"
#include "animation.h"
#include "AI.h"
#include "global.h"
#include "puyorng.h"
#include "mt.h"
#include <string>
#include <vector>
#include <map>
#include <deque>
#include <algorithm>

namespace ppvs
{
class player;
class game;

struct garbageCounter
{
    int CQ,GQ;
    std::vector<player*> accumulator;
};

struct voices
{
    sound chain[12],damage1,damage2,fever,feversuccess,feverfail,lose,win,choose;
};

enum attackState
{
    NOATTACK,ATTACK,DEFEND,COUNTERATTACK,COUNTERDEFEND,COUNTERIDLE
};

struct replay_player_header
{
    char name[32];
    short currentwins;
    int onlineID;
    char playernum;
    playerType playertype;// (HUMAN or ONLINE)
    puyoCharacter character;
    char active; //active at start
    char colors;
    int vectorsize_movement; //size of vector in bytes
    int vectorsize_message;
    int vectorsizecomp_movement; //size of compressed vector in bytes
    int vectorsizecomp_message;
};

struct messageEvent
{
    int time;
    char message[64];
};

class player
{
    public:
        /*****************
        *Functions
        ******************/

        player(playerType type,int playernum,int totalPlayers,game*);
        ~player();

        //Initialize
        void reset();
        void initValues(int randomSeed);
        fsound *loadVoice(const std::string &folder, const char *sound);
        void initVoices();
        void initNextList();
        void playerSetup(fieldProp &p,int playernum,int playerTotal);
        void setRandomSeed(unsigned long seed_in,PuyoRNG**);

        //Get and set
        int getRandom(int in,PuyoRNG*);
        int getPlayerNum(){return m_playernum;}
        playerType getPlayerType(){return m_type;}
        void setPlayerType(playerType playertype);
        int getReadyGoTimer(){return m_readyGoTimer;}
        float getGlobalScale(){return m_globalScale;}
        puyoCharacter getCharacter(){return m_character;}
        void setCharacter(puyoCharacter pl,bool show=false);
        void setFieldImage(puyoCharacter pc);
        field *getNormalField(){return &m_fieldNormal;}
        field *getFeverField(){return &m_fieldFever;}

        //Garbage
        void updateTray(garbageCounter* c=0);
        void playGarbageSound();
        int getGarbageSum();
        int getAttackSum();
        void playLightEffect();
        void addFeverCount();
        void addAttacker(garbageCounter*,player*);
        void removeAttacker(garbageCounter*,player*);
        void checkAnyAttacker(garbageCounter*);
        void checkFeverGarbage();
        void addToGQ(garbageCounter*);

        void resetNuisanceDropPattern();
        int nuisanceDropPattern();

        //Other objects
        void setScoreCounter();
        void setScoreCounterPB();
        void showSecondsObj(int n);
        void setMarginTimer();

        //Gameplay
        int TGMR(int color, int n);
        void addNewColorPair(int n);
        void popColor();
        void play();
        void endPhase();
        void prepareVoice(int chain,int predicted);
        void playVoice();
        void getReady();//phase 0
        void chooseColor();
        void prepare(); //phase 1
        void cpuMove(); //phase 10
        void destroyPuyos(); //phase 31
        void startGarbage(); //phase 33
        void garbagePhase(); //phase 34
        void counterGarbage();
        void endGarbage(); //phase 35
        void checkAllClear(); //phase 41
        void checkLoser(bool endphase=true); //phase 43
        void checkWinner();
        void loseGame();
        void winGame();
        void checkFever();//phase 50
        void startFever();//phase 51
        void checkEndFever();//phase 50
        void checkEndFeverOnline();
        void dropFeverChain(); //phase 52
        void endFever(); //phase 54
        void playFever();
        void setLose();

        //Draw code
        void draw();
        void drawEffect();
        void drawFieldBack(posVectorFloat position,float rotation);
        void drawFieldFeverBack(posVectorFloat position,float rotation);
        void drawAllClear(posVectorFloat position,float scaleX,float scaleY,float rotation);
        void drawCross(frendertarget *r);
        void drawLose();
        void drawWin();
        void drawColorMenu();

        /*****************
        *Public variables
        ******************/
        gameData* data;
        controller controls;
        field* activeField;
        fieldProp properties;
        game* currentgame;
        movePuyo movePuyos;
        chainWord* pchainWord;
        phase currentphase;
        feverCounter feverGauge;
        animation characterAnimation;
        voices characterVoices;
        AI *cpuAI;
        float dropspeed;
        int colors;
        int scoreVal,currentScore;
        int turns;
        int targetPoint;
        bool createPuyo;
        bool forgiveGarbage;
        bool useDropPattern;
        losewinState losewin;
        unsigned int randomSeedFever;
        int margintimer;
        bool nextPuyoActive;
        int destroyPuyosTimer;
        bool foundChain;

        //color select
        int colorMenuTimer;
        int menuHeight;
        int spiceSelect;
        sprite colorMenuBorder[9];
        sprite spice[5];
        bool pickedColor;
        bool takeover;

        //chain variables
        float garbageTimer;
        float garbageSpeed;
        int chainPopSpeed;
        int garbageEndTime;
        int puyoBounceSpeed;
        int puyoBounceEnd;
        float gravity;
        attackState attdef;
        int chain,puyosPopped,totalGroups,
            groupR,groupG,groupB,groupY,groupP,
            predictedChain,point,bonus,linkBonus,
            allClear,rememberMaxY,rememberX;
        int dropBonus;
        int divider;
        bool bonusEQ;

        //voice
        int diacute;
        int playvoice;
        int stutterTimer;

        //fever mode
        bool feverMode;
        bool feverEnd;
        int currentFeverChainAmount;
        bool poppedChain; //is reset during PREPARE phase
        int feverColor; //use enum?
        float feverColorR,feverColorG,feverColorB; //RGB values used
        int feverSuccess; //used for playing voice
        void checkAllClearStart();

        //garbage variables
        int EQ,tray;
        garbageCounter *activeGarbage;
        garbageCounter normalGarbage,feverGarbage;
        nuisanceTray normalTray,feverTray;
        std::map<player*,garbageCounter*> targetGarbage;
        int attackers;
        int garbageDropped;
        int garbageCycle;
        player *lastAttacker;

        //Online variables
        //sf::Image overlayImage;
        sprite overlaySprite;
        void bindPlayer(std::string name,unsigned int id,bool setActive);
        void unbindPlayer();
        void addMessage(std::string mes);
        std::string onlineName; //use this to check if peer is bound to player
        std::string previousName; //set at start of match, useful for replays
        unsigned int onlineID;
        bool active; //acknowledged as active player
        bool prepareActive; //bugfix: wait setting player to active until end of game
        int activeAtStart; //active player started game
        bool rematch;
        sprite charHolderSprite;
        sprite currentCharacterSprite;
        int showCharacterTimer;
        sprite readyToPlay;
        stringList messages;
        int proposedRandomSeed;
        int wins;
        bool pickingCharacter;
        sprite rematchIcon;
        int rematchIconTimer;
        bool loseConfirm;
        void getUpdate(std::string str);
        int calledRandomFeverChain;
        bool forceStatusText;
        void prepareDisconnect();

        //int moves;
        int waitForConfirm;
        void confirmGarbage();
        void waitGarbage();
        void waitLose();

        //text
        ffont *statusFont;
        ftext *statusText;
        std::string lastText;
        void setStatusText(const char* utf8);

        //debugging
        int debug;

        std::vector<messageEvent> recordMessages;

    private:
        void processMessage();
        void setDropsetSprite(int x,int y,puyoCharacter pc);
        sprite dropset[16];

        std::vector<lightEffect*> m_lightEffect;
        std::deque<int> m_nextList;
        std::vector<secondsObject*> m_secondsObj;

        //timers
        int m_lightTimer;
        int m_loseWinTimer;
        int m_readyGoTimer;
        int m_transitionTimer;
        int allclearTimer;
        float m_transformScale;

        //MTRand m_randomizer;
        PuyoRNG* m_randomizerNextList;
        PuyoRNG* m_randomizerFeverChain;
        PuyoRNG* m_randomizerFeverColor;
        MersenneTwister* m_randomizerNuisanceDrop;
        std::vector<int> m_nuisanceList;
        long m_randomseed;
        int m_playernum;
        float m_globalScale; //global scale for certain objects (for example sprite animations)

        playerType m_type;
        scoreCounter m_scoreCounter;
        feverLight m_feverLight;
        puyoCharacter m_character;
        nextPuyo m_nextPuyo;
        field m_fieldNormal;
        field m_fieldFever;
        field m_fieldTemp;
        sprite m_fieldSprite;
        sprite m_fieldFeverSprite;
        sprite m_allclearSprite;
        sprite m_crossSprite;
        sprite m_winSprite;
        sprite m_loseSprite;
        sprite m_borderSprite;
        float m_nextPuyoOffsetX;
        float m_nextPuyoOffsetY;
        float m_nextPuyoScale;

        //bugfix: garbage bug
        bool hasMoved; //check if player went into move phase as indication an attack had ended

        int debugCounter;
};
}
#endif // PLAYER_H
