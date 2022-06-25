#include <algorithm>
#include "ruleset.h"
#include "player.h"
#include "game.h"

using namespace std;

namespace ppvs
{
//Lists
int linkBonus_TSU[11]={0,0,0,0,2,3,4,5,6,7,10};
int colorBonus_TSU[5]={0,3,6,12,24};
int chainBonus_TSU[22]={0,8,16,32,64,96,128,160,192,224,256,288,320,352,384,416,448,480,512,544,576,608};

int linkBonus_FEVER[11]={0,0,0,0,1,2,3,4,5,6,8};
int colorBonus_FEVER[5]={0,2,4,8,16};

//Character powers
int chainBonus_NORMAL[24][24]={
{4,11,24,33,51,106,179,274,371,472,600,732,882,999,999,999,999,999,999,999,999,999,999,999},
{4,12,24,32,48,96 ,160,240,320,400,500,600,700,800,900,999,999,999,999,999,999,999,999,999},
{4,12,24,33,50,101,169,254,341,428,538,648,763,876,990,999,999,999,999,999,999,999,999,999},
{4,13,25,33,48,96 ,158,235,310,384,475,564,644,728,810,890,968,999,999,999,999,999,999,999},
{4,12,25,35,52,106,179,269,362,456,575,696,826,952,999,999,999,999,999,999,999,999,999,999},
{4,12,25,32,49,96 ,160,241,320,400,501,600,700,800,901,999,999,999,999,999,999,999,999,999},
{4,12,25,33,52,106,179,275,372,472,600,732,882,999,999,999,999,999,999,999,999,999,999,999},
{4,11,21,28,41,82 ,135,202,267,332,413,492,567,644,720,795,869,936,999,999,999,999,999,999},
{4,11,23,33,51,110,188,288,392,500,638,780,945,999,999,999,999,999,999,999,999,999,999,999},
{4,12,24,32,48,96 ,160,240,320,400,500,600,700,800,900,999,999,999,999,999,999,999,999,999},
{4,13,25,33,49,96 ,159,235,310,385,475,565,645,729,810,890,969,999,999,999,999,999,999,999},
{4,11,22,29,43,86 ,144,216,288,360,450,540,630,720,810,900,990,999,999,999,999,999,999,999},
{4,11,22,30,45,91 ,153,230,309,388,488,588,693,796,900,999,999,999,999,999,999,999,999,999},
{4,11,24,33,51,106,179,274,371,472,600,732,882,999,999,999,999,999,999,999,999,999,999,999},
{4,12,25,34,52,106,178,269,362,456,575,696,826,952,999,999,999,999,999,999,999,999,999,999},
{4,12,22,31,45,92 ,153,231,309,389,489,589,693,796,901,999,999,999,999,999,999,999,999,999},
{4,12,22,29,43,86 ,145,216,289,360,451,540,630,720,810,901,991,999,999,999,999,999,999,999},
{4,11,24,33,51,106,179,274,371,472,600,732,882,999,999,999,999,999,999,999,999,999,999,999},
{4,11,23,33,51,101,167,250,331,412,513,612,766,966,999,999,999,999,999,999,999,999,999,999},
{4,11,23,33,51,110,188,288,392,500,638,780,945,999,999,999,999,999,999,999,999,999,999,999},
{4,11,22,29,43,86 ,144,216,288,360,450,540,630,720,810,900,990,999,999,999,999,999,999,999},
{4,11,21,28,41,82 ,135,202,267,332,413,492,567,644,720,795,869,936,999,999,999,999,999,999},
{4,12,23,31,46,92 ,152,229,305,380,476,570,665,760,855,898,935,969,999,999,999,999,999,999},
{4,11,22,29,43,86 ,144,216,288,360,450,540,630,720,810,900,990,999,999,999,999,999,999,999}};
int chainBonus_FEVER[24][24]={
{4,9 ,16,20,27,43,72,108,144,216,252,259,308,360,396,432,468,504,540,576,612,648,684,720},
{4,10,18,22,30,48,80,120,160,240,280,288,342,400,440,480,520,560,600,640,680,720,760,800},
{4,9 ,16,20,27,43,72,108,144,216,252,259,308,360,396,432,468,504,540,576,612,648,684,720},
{4,10,18,21,29,46,76,113,150,223,259,266,313,364,398,432,468,504,540,576,612,648,684,720},
{4,9 ,15,19,25,39,65,96 ,129,192,225,231,275,320,352,385,416,449,481,512,545,576,609,640},
{4,10,19,22,31,49,80,121,160,241,280,289,342,400,440,481,520,560,600,640,680,720,760,800},
{4,9 ,16,20,28,43,72,109,145,216,252,259,309,360,396,432,469,505,540,576,612,649,685,720},
{4,11,19,24,32,50,84,125,166,247,287,294,347,404,442,480,520,560,600,640,680,720,760,800},
{4,9 ,16,20,27,43,72,108,144,216,252,259,308,360,396,432,468,504,540,576,612,648,684,720},
{4,10,18,21,29,46,76,113,150,223,259,266,313,364,398,432,468,504,540,576,612,648,684,720},
{4,10,19,22,29,46,76,113,150,223,259,266,313,365,399,432,469,505,540,576,612,649,685,720},
{4,10,19,24,34,55,93,142,191,290,343,355,428,508,565,624,676,728,780,832,884,936,988,999},
{5,12,21,25,34,53,87,130,171,254,294,301,353,408,444,480,520,560,600,640,680,720,760,800},
{3,8 ,15,20,28,46,77,118,159,242,287,298,360,428,477,528,572,616,660,704,748,792,836,880},
{3,8 ,14,18,24,38,64,96 ,128,192,224,230,274,320,352,384,416,448,480,512,544,576,608,640},
{4,12,22,25,35,53,88,130,172,255,295,302,353,409,445,481,520,560,600,640,680,720,760,800},
{4,9 ,18,22,32,50,85,130,175,266,315,326,395,469,522,576,625,672,720,769,816,865,912,961},
{3,8 ,15,20,28,46,77,112,151,229,272,283,342,406,453,501,543,585,627,668,710,752,794,836},
{3,8 ,15,18,25,41,68,103,138,209,245,253,302,356,394,432,468,504,540,576,612,648,684,720},
{3,8 ,15,18,25,41,68,103,138,209,245,253,302,356,394,432,468,504,540,576,612,648,684,720},
{4,11,20,25,34,55,92,139,186,281,329,339,405,476,526,576,624,672,720,768,816,864,912,960},
{4,11,20,25,34,55,92,139,186,281,329,339,405,476,526,576,624,672,720,768,816,864,912,960},
{4,10,19,23,32,49,82,125,165,249,292,299,358,419,462,505,546,589,630,672,715,756,799,840},
{4,9 ,17,22,31,50,85,130,175,266,315,326,394,468,521,576,624,672,720,768,816,864,912,960}};

//target point calculation
int getTargetFromMargin(int initialTarget,int marginTime,int currentTime)
{
    if (currentTime>=marginTime)
    {
        int step=(currentTime-marginTime)/(16*60);
        step=min(step,13);
        int out=initialTarget;
        //even
        if (step%2==0)
        {
            out=initialTarget*3/4;
            step=step/2;
        }
        //uneven
        else
        {
            step=(step+1)/2;
        }
        return max(int(out/pow(2.,step)),1);
    }
    else
        return initialTarget;
}

//{Base rules
//-------------------------
ruleset::ruleset()
{
    //ctor
    delayedFall=true;
    doubleSpawn=true;
    addDropBonus=false;
    voicePatternFever=true;
    fastDrop=48;
    marginTime=0;
    initialFeverCount=0;
    puyoToClear=4;
    feverPower=1;
    requiredChain=0;
    NFeverChains=4;
    allClearStart=true;
    feverDeath=true;
    bonusEQ=false;
    quickDrop=false;
}

ruleset::~ruleset()
{
    //dtor
}

void ruleset::setRules(rules rulestring)
{
    m_rules=rulestring;

    //set constants
    switch (m_rules)
    {
        case ENDLESS:
            break;
        case TSU:
            break;
        case FEVER:
            break;
    }
}

void ruleset::setGame(game* g)
{
    currentgame=g;
    data=g->data;
    this->onSetGame();
}

phase ruleset::endPhase(phase,player*)
{
    return IDLE;
}

void ruleset::onSetGame()
{
    // use this to set legacyRandomizer
}

void ruleset::onInit(player* thisplayer)
{
    thisplayer->targetPoint=70;
}

void ruleset::onAllClearPop(player* thisplayer)
{
    thisplayer->allClear=0;
}

void ruleset::onAllClear(player* thisplayer)
{
    thisplayer->allClear=1;
}

void ruleset::onLose(player* thisplayer)
{
    thisplayer->losewin=LOSE;
    thisplayer->currentphase=LOSEDROP;
}

void ruleset::onWin(player* thisplayer)
{
    thisplayer->losewin=WIN;
    thisplayer->currentphase=WIN_IDLE;

}

void ruleset::onChain(player*)
{

}

void ruleset::onOffset(player*)
{

}

void ruleset::onAttack(player*)
{

}

int ruleset::getLinkBonus(int n)
{
    n--;
    if (n<0)
        return 0;

    if (n>=11)
        return linkBonus_TSU[10];
    else
        return linkBonus_TSU[n];
}

int ruleset::getColorBonus(int n)
{
    n--;
    if (n<0)
        return 0;

    if (n>=5)
        return colorBonus_TSU[4];
    else
        return colorBonus_TSU[n];
}

int ruleset::getChainBonus(player* pl)
{
    int n=pl->chain;
    n--;
    if (n<0)
            return 0;
    if (n>=16)
        return min(chainBonus[15]+32*(n - 15),999);
    else
        return chainBonus[n];
}
//}

//{ENDLESS
//==========================================================
ruleset_ENDLESS::ruleset_ENDLESS()
{
    delayedFall=true;
    doubleSpawn=true;
}

ruleset_ENDLESS::~ruleset_ENDLESS()
{

}


phase ruleset_ENDLESS::endPhase(phase currentPhase,player* thisplayer)
{
    switch (currentPhase)
    {
        case GETREADY:
            return DROPGARBAGE;
            break;
        case PREPARE:
            return MOVE;
            break;
        case MOVE:
            thisplayer->createPuyo=true;
            return CREATEPUYO;
            break;
        case CREATEPUYO:
            return DROPPUYO;
            break;
        case DROPPUYO:
            return FALLPUYO;
            break;
        case FALLPUYO:
            return SEARCHCHAIN;
            break;
        case SEARCHCHAIN:
            //continue to phase 40 if no chain found, else to 32
            if (!thisplayer->foundChain)
                return CHECKALLCLEAR;
            else //found chain
                thisplayer->destroyPuyosTimer=1;
                return DESTROYPUYO;
            break;
        case DESTROYPUYO:
            //return DROPPUYO;
            return GARBAGE;
            break;
        case GARBAGE:
            return DROPPUYO;
            break;
        case CHECKALLCLEAR:
            return DROPGARBAGE;
            break;
        case DROPGARBAGE:
            return FALLGARBAGE;
            break;
        case FALLGARBAGE:
            return CHECKLOSER;
            break;
        case CHECKLOSER:
            return PREPARE;
            break;

        default:
            return IDLE;

    }
}

void ruleset_ENDLESS::onInit(player* thisplayer)
{
    thisplayer->targetPoint=targetPoint;
}

void ruleset_ENDLESS::onAllClearPop(player *thisplayer)
{
    //add 30 nuisance puyos
    //triggers during phase 30
    if (thisplayer->currentphase==SEARCHCHAIN)
    {
        thisplayer->currentScore+=30*thisplayer->targetPoint;
        thisplayer->allClear=0;
    }
}

void ruleset_ENDLESS::onWin(player* thisplayer)
{
    thisplayer->losewin=WIN;
    thisplayer->currentphase=WIN_IDLE;
    //sound
    if (thisplayer==currentgame->players[0])
        data->snd.win.Play(data);
    else
        data->snd.lose.Play(data);
}

void ruleset_ENDLESS::onChain(player*)
{
    //thisplayer->forgiveGarbage=true;
}

void ruleset_ENDLESS::onOffset(player* thisplayer)
{
    thisplayer->forgiveGarbage=true;
    thisplayer->addFeverCount();
}
int ruleset_ENDLESS::getLinkBonus(int n)
{
    n--;
    if (n<0)
        return 0;

    if (n>=11)
        return linkBonus_FEVER[10];
    else
        return linkBonus_FEVER[n];
}

int ruleset_ENDLESS::getColorBonus(int n)
{
    n--;
    if (n<0)
        return 0;

    if (n>=5)
        return colorBonus_FEVER[4];
    else
        return colorBonus_FEVER[n];
}

int ruleset_ENDLESS::getChainBonus(player* pl)
{
    int n=pl->chain;
    int character = pl->getCharacter();
    n--;
    if (n<0)
        return 0;

    if (n>23)
        return 999;
    else
    {
        if (pl->feverMode)
            return chainBonus_FEVER[character][n];
        else
            return chainBonus_NORMAL[character][n];
    }
}
//}

//{TSU
//==========================================================
ruleset_TSU::ruleset_TSU()
{
    delayedFall=false;
    doubleSpawn=false;
    addDropBonus=true;
    voicePatternFever=false;
}

ruleset_TSU::~ruleset_TSU()
{

}


phase ruleset_TSU::endPhase(phase currentPhase,player* thisplayer)
{
    switch (currentPhase)
    {
        case GETREADY:
            return DROPGARBAGE;
            break;
        case PREPARE:
            return MOVE;
            break;
        case MOVE:
            thisplayer->createPuyo=true;
            return CREATEPUYO;
            break;
        case CREATEPUYO:
            return DROPPUYO;
            break;
        case DROPPUYO:
            return FALLPUYO;
            break;
        case FALLPUYO:
            return SEARCHCHAIN;
            break;
        case SEARCHCHAIN:
            //continue to phase 40 if no chain found, else to 32
            if (!thisplayer->foundChain)
                return CHECKALLCLEAR;
            else //found chain
                thisplayer->destroyPuyosTimer=1;
                return DESTROYPUYO;
            break;
        case DESTROYPUYO:
            return GARBAGE;
            break;
        case GARBAGE:
            return DROPPUYO;
            break;
        case CHECKALLCLEAR:
            return DROPGARBAGE;
            break;
        case DROPGARBAGE:
            return FALLGARBAGE;
            break;
        case FALLGARBAGE:
            return CHECKLOSER;
            break;
        case CHECKLOSER:
            return PREPARE;
            break;

        default:
            return IDLE;

    }
}

void ruleset_TSU::onInit(player* thisplayer)
{
    thisplayer->targetPoint=targetPoint;
    thisplayer->feverGauge.setVisible(false);
    thisplayer->useDropPattern=false;
}

void ruleset_TSU::onAllClearPop(player *thisplayer)
{
    //add 30 nuisance puyos
    //triggers during phase 30
    if (thisplayer->currentphase==SEARCHCHAIN)
    {
        thisplayer->currentScore+=30*thisplayer->targetPoint;
        thisplayer->allClear=0;
    }
}

void ruleset_TSU::onWin(player* thisplayer)
{
    thisplayer->losewin=WIN;
    thisplayer->currentphase=WIN_IDLE;
    //sound if player 1 wins or loses
    if (thisplayer==currentgame->players[0])
        data->snd.win.Play(data);
    else
        data->snd.lose.Play(data);
}

void ruleset_TSU::onLose(player* thisplayer)
{
    thisplayer->losewin=LOSE;
    thisplayer->currentphase=LOSEDROP;
    //sounds for losing (not player 1)
    //debugstring=to_string(currentgame->getActivePlayers());
    if (thisplayer!=currentgame->players[0] && currentgame->getActivePlayers()!=1)
        data->snd.lose.Play(data);
}

int ruleset_TSU::getLinkBonus(int n)
{
    n--;
    if (n<0)
        return 0;

    if (n>=11)
        return linkBonus_TSU[10];
    else
        return linkBonus_TSU[n];
}

int ruleset_TSU::getColorBonus(int n)
{
    n--;
    if (n<0)
        return 0;

    if (n>=5)
        return colorBonus_TSU[4];
    else
        return colorBonus_TSU[n];
}

int ruleset_TSU::getChainBonus(player* pl)
{
    int n=pl->chain;
    n--;
    if (n<0)
            return 0;
    if (n>=16)
        return min(chainBonus_TSU[15]+32*(n - 15),999);
    else
        return chainBonus_TSU[n];
}
//}

//{FEVER
//==========================================================
ruleset_FEVER::ruleset_FEVER()
{
    delayedFall=true;
    doubleSpawn=true;
}

ruleset_FEVER::~ruleset_FEVER()
{

}

phase ruleset_FEVER::endPhase(phase currentPhase,player* thisplayer)
{
    switch (currentPhase)
    {
        case GETREADY:
            return DROPGARBAGE;
            break;
        case PREPARE:
            return MOVE;
            break;
        case MOVE:
            thisplayer->createPuyo=true;
            return CREATEPUYO;
            break;
        case CREATEPUYO:
            return DROPPUYO;
            break;
        case DROPPUYO:
            return FALLPUYO;
            break;
        case FALLPUYO:
            return SEARCHCHAIN;
            break;
        case SEARCHCHAIN:
            //continue to phase 40 if no chain found, else to 32
            if (!thisplayer->foundChain)
                return CHECKALLCLEAR;
            else //found chain
                thisplayer->destroyPuyosTimer=1;
                return DESTROYPUYO;
            break;
        case DESTROYPUYO:
            //return DROPPUYO;
            return GARBAGE;
            break;
        case GARBAGE:
            return DROPPUYO;
            break;
        case CHECKALLCLEAR:
            if (thisplayer->feverMode)
                return CHECKENDFEVER;
            else
                return DROPGARBAGE;
            break;
        case CHECKENDFEVER:
            if (thisplayer->feverGauge.seconds==0)
                return ENDFEVER;
            else
            {
                if (!thisplayer->poppedChain) //no chain popped: normal progression
                    return DROPGARBAGE;
                else //player popped a chain, drop a new feverchain
                {
                    thisplayer->poppedChain=false; //chain must be reset here
                    return DROPFEVER;
                }
            }
            break;
        case ENDFEVER:
                return DROPGARBAGE;
            break;

        case DROPGARBAGE:
            return FALLGARBAGE;
            break;
        case FALLGARBAGE:
            return CHECKLOSER;
            break;
        case CHECKLOSER:
            if (!thisplayer->feverMode)
                return CHECKFEVER; //not in fever: check if fever should start
            else
                return PREPARE;
            break;
        case CHECKFEVER:
            if (thisplayer->feverMode)
                return PREPAREFEVER;
            else
            {
                if (thisplayer->allClear==0)
                    return PREPARE;
                else
                {
                    thisplayer->allClear=0;
                    return DROPPUYO;
                }
            }
            break;
        case PREPAREFEVER:
            return DROPFEVER;
            break;
        case DROPFEVER:
            return DROPPUYO;

        case LOSEDROP:
            return LOSEDROP; //in fever it's possible the game tries to call endphase in fever mode

        default:
            return IDLE;
    }
}

void ruleset_FEVER::onSetGame()
{
    currentgame->legacyRandomizer = true;
    currentgame->legacyNuisanceDrop = true;
}

void ruleset_FEVER::onInit(player* thisplayer)
{
    thisplayer->targetPoint=targetPoint;
    thisplayer->feverGauge.setVisible(true);
    puyoToClear=4; //must be 4!
}

void ruleset_FEVER::onAllClearPop(player *)
{
    //does nothing
}

void ruleset_FEVER::onAllClear(player* thisplayer)
{
    //drop a fever pattern on the field or increase feverchain amount & fevertime
    //implementation of allclear is inside player object (checkstartfever and checkendfever)
    thisplayer->allClear=1;
}

void ruleset_FEVER::onLose(player* thisplayer)
{
    thisplayer->losewin=LOSE;
    thisplayer->currentphase=LOSEDROP;
}

void ruleset_FEVER::onWin(player* thisplayer)
{
    thisplayer->losewin=WIN;
    thisplayer->currentphase=WIN_IDLE;
    //sound
    if (thisplayer==currentgame->players[0])
        data->snd.win.Play(data);
    else
        data->snd.lose.Play(data);
}

void ruleset_FEVER::onChain(player*)
{
    //thisplayer->forgiveGarbage=true;
    //thisplayer->feverColor=getRandom(5);
}

void ruleset_FEVER::onOffset(player* thisplayer)
{
    thisplayer->forgiveGarbage=true;
    thisplayer->addFeverCount();
}

void ruleset_FEVER::onAttack(player* thisplayer)
{
    if (!thisplayer->feverMode && thisplayer->feverGauge.seconds<60*30)
    {
        thisplayer->feverGauge.addTime(60);
        thisplayer->showSecondsObj(60);
    }
}

int ruleset_FEVER::getLinkBonus(int n)
{
    n--;
    if (n<0)
        return 0;

    if (n>=11)
        return linkBonus_FEVER[10];
    else
        return linkBonus_FEVER[n];
}

int ruleset_FEVER::getColorBonus(int n)
{
    n--;
    if (n<0)
        return 0;

    if (n>=5)
        return colorBonus_FEVER[4];
    else
        return colorBonus_FEVER[n];
}

int ruleset_FEVER::getChainBonus(player* pl)
{
    int n=pl->chain;
    int character=pl->getCharacter();
    n--;
    if (n<0)
        return 0;

    if (n>23)
        return 999;
    else
    {
        if (pl->feverMode)
            return chainBonus_FEVER[character][n];
        else
            return chainBonus_NORMAL[character][n];
    }
}
//}

//{ENDLESS FEVER
//==========================================================
ruleset_ENDLESSFEVER::ruleset_ENDLESSFEVER()
{
    delayedFall=true;
    doubleSpawn=true;
    allClearStart=false;
}

ruleset_ENDLESSFEVER::~ruleset_ENDLESSFEVER()
{

}

phase ruleset_ENDLESSFEVER::endPhase(phase currentPhase,player* thisplayer)
{
    switch (currentPhase)
    {
        case GETREADY:
            return CHECKFEVER;
            break;
        case PREPARE:
            return MOVE;
            break;
        case MOVE:
            thisplayer->createPuyo=true;
            return CREATEPUYO;
            break;
        case CREATEPUYO:
            return DROPPUYO;
            break;
        case DROPPUYO:
            return FALLPUYO;
            break;
        case FALLPUYO:
            return SEARCHCHAIN;
            break;
        case SEARCHCHAIN:
            //continue to phase 40 if no chain found, else to 32
            if (!thisplayer->foundChain)
                return CHECKALLCLEAR;
            else //found chain
                thisplayer->destroyPuyosTimer=1;
                return DESTROYPUYO;
            break;
        case DESTROYPUYO:
            return GARBAGE;
            break;
        case GARBAGE:
            return DROPPUYO;
            break;
        case CHECKALLCLEAR:
            if (thisplayer->feverMode)
                return CHECKENDFEVER;
            else
                return DROPGARBAGE;
            break;
        case CHECKENDFEVER:
            if (thisplayer->feverGauge.seconds==0)
            {//lose
                    thisplayer->losewin=LOSE;
                    return LOSEDROP;
            }
            else
            {
                if (!thisplayer->poppedChain) //no chain popped: normal progression
                    return DROPGARBAGE;
                else //player popped a chain, drop a new feverchain
                {
                    thisplayer->poppedChain=false; //chain must be reset here
                    return DROPFEVER;
                }
            }
            break;
        case ENDFEVER:
                return DROPGARBAGE;
            break;

        case DROPGARBAGE:
            return FALLGARBAGE;
            break;
        case FALLGARBAGE:
            return CHECKLOSER;
            break;
        case CHECKLOSER:
            if (!thisplayer->feverMode)
                return CHECKFEVER; //not in fever: check if fever should start
            else
                return PREPARE;
            break;
        case CHECKFEVER:
            if (thisplayer->feverMode)
                return PREPAREFEVER;
            else
            {
                if (thisplayer->allClear==0)
                    return PREPARE;
                else
                {
                    thisplayer->allClear=0;
                    return DROPPUYO;
                }
            }
            break;
        case PREPAREFEVER:
            //return IDLE; //record here
            return DROPFEVER;
            break;
        case DROPFEVER:
            return DROPPUYO;

        case LOSEDROP:
            return LOSEDROP; //in fever it's possible the game tries to call endphase in fever mode

        default:
            return IDLE;
    }
}

void ruleset_ENDLESSFEVER::onSetGame()
{
    currentgame->legacyRandomizer = true;
    currentgame->legacyNuisanceDrop = true;
}

void ruleset_ENDLESSFEVER::onInit(player* thisplayer)
{
    thisplayer->targetPoint=targetPoint;
    thisplayer->feverGauge.setVisible(true);
    puyoToClear=4; //must be 4!
    thisplayer->feverGauge.setCount(7);
    thisplayer->feverGauge.setSeconds(60*60);
    thisplayer->feverGauge.maxSeconds=99;
    thisplayer->nextPuyoActive=false;
}

void ruleset_ENDLESSFEVER::onAllClearPop(player *)
{
    //does nothing
}

void ruleset_ENDLESSFEVER::onAllClear(player* thisplayer)
{
    //drop a fever pattern on the field or increase feverchain amount & fevertime
    //implementation of allclear is inside player object (checkstartfever and checkendfever)
    thisplayer->allClear=1;
}

void ruleset_ENDLESSFEVER::onLose(player* thisplayer)
{
    thisplayer->losewin=LOSE;
    thisplayer->currentphase=LOSEDROP;
}

void ruleset_ENDLESSFEVER::onWin(player* thisplayer)
{
    thisplayer->losewin=WIN;
    thisplayer->currentphase=WIN_IDLE;
    //sound
    if (thisplayer==currentgame->players[0])
        data->snd.win.Play(data);
    else
        data->snd.lose.Play(data);
}

void ruleset_ENDLESSFEVER::onChain(player*)
{
    //thisplayer->forgiveGarbage=true;
    //thisplayer->feverColor=getRandom(5);
}

void ruleset_ENDLESSFEVER::onOffset(player* thisplayer)
{
    thisplayer->forgiveGarbage=true;
    thisplayer->addFeverCount();
}

void ruleset_ENDLESSFEVER::onAttack(player* thisplayer)
{
    if (!thisplayer->feverMode && thisplayer->feverGauge.seconds<60*30)
    {
        thisplayer->feverGauge.addTime(60);
        thisplayer->showSecondsObj(60);
    }
}

int ruleset_ENDLESSFEVER::getLinkBonus(int n)
{
    n--;
    if (n<0)
        return 0;

    if (n>=11)
        return linkBonus_FEVER[10];
    else
        return linkBonus_FEVER[n];
}

int ruleset_ENDLESSFEVER::getColorBonus(int n)
{
    n--;
    if (n<0)
        return 0;

    if (n>=5)
        return colorBonus_FEVER[4];
    else
        return colorBonus_FEVER[n];
}

int ruleset_ENDLESSFEVER::getChainBonus(player* pl)
{
    int n=pl->chain;
    int character=pl->getCharacter();
    n--;
    if (n<0)
        return 0;

    if (n>23)
        return 999;
    else
    {
        if (pl->feverMode)
            return chainBonus_FEVER[character][n];
        else
            return chainBonus_NORMAL[character][n];
    }
}
//}

//{ENDLESS FEVER VS
//==========================================================
ruleset_ENDLESSFEVERVS::ruleset_ENDLESSFEVERVS()
{
    delayedFall=true;
    doubleSpawn=true;
}

ruleset_ENDLESSFEVERVS::~ruleset_ENDLESSFEVERVS()
{

}

phase ruleset_ENDLESSFEVERVS::endPhase(phase currentPhase,player* thisplayer)
{
    switch (currentPhase)
    {
        case GETREADY:
            return CHECKFEVER;
            break;
        case PREPARE:
            return MOVE;
            break;
        case MOVE:
            thisplayer->createPuyo=true;
            return CREATEPUYO;
            break;
        case CREATEPUYO:
            return DROPPUYO;
            break;
        case DROPPUYO:
            return FALLPUYO;
            break;
        case FALLPUYO:
            return SEARCHCHAIN;
            break;
        case SEARCHCHAIN:
            //continue to phase 40 if no chain found, else to 32
            if (!thisplayer->foundChain)
                return CHECKALLCLEAR;
            else //found chain
                thisplayer->destroyPuyosTimer=1;
                return DESTROYPUYO;
            break;
        case DESTROYPUYO:
            //return DROPPUYO;
            return GARBAGE;
            break;
        case GARBAGE:
            return DROPPUYO;
            break;
        case CHECKALLCLEAR:
            if (thisplayer->feverMode)
                return CHECKENDFEVER;
            else
                return DROPGARBAGE;
            break;
        case CHECKENDFEVER:
            if (thisplayer->feverGauge.seconds==0)
                return ENDFEVER;
            else
            {
                if (!thisplayer->poppedChain) //no chain popped: normal progression
                    return DROPGARBAGE;
                else //player popped a chain, drop a new feverchain
                {
                    thisplayer->poppedChain=false; //chain must be reset here
                    return DROPFEVER;
                }
            }
            break;
        case ENDFEVER:
                return DROPGARBAGE;
            break;

        case DROPGARBAGE:
            return FALLGARBAGE;
            break;
        case FALLGARBAGE:
            return CHECKLOSER;
            break;
        case CHECKLOSER:
            if (!thisplayer->feverMode)
                return CHECKFEVER; //not in fever: check if fever should start
            else
                return PREPARE;
            break;
        case CHECKFEVER:
            if (thisplayer->feverMode)
                return PREPAREFEVER;
            else
            {
                if (thisplayer->allClear==0)
                    return PREPARE;
                else
                {
                    thisplayer->allClear=0;
                    return DROPPUYO;
                }
            }
            break;
        case PREPAREFEVER:
            return DROPFEVER;
            break;
        case DROPFEVER:
            return DROPPUYO;

        case LOSEDROP:
            return LOSEDROP; //in fever it's possible the game tries to call endphase in fever mode

        default:
            return IDLE;
    }
}

void ruleset_ENDLESSFEVERVS::onSetGame()
{
    currentgame->legacyRandomizer = true;
    currentgame->legacyNuisanceDrop = true;
}

void ruleset_ENDLESSFEVERVS::onInit(player* thisplayer)
{
    thisplayer->targetPoint=targetPoint;
    thisplayer->feverGauge.setVisible(true);
    puyoToClear=4; //must be 4!
    thisplayer->feverGauge.setCount(7);
    thisplayer->feverGauge.setSeconds(99*60);
    thisplayer->feverGauge.endless=true;
    thisplayer->normalGarbage.GQ=270;
    thisplayer->updateTray();
    thisplayer->nextPuyoActive=false;
}

void ruleset_ENDLESSFEVERVS::onAllClearPop(player *)
{
    //does nothing
}

void ruleset_ENDLESSFEVERVS::onAllClear(player* thisplayer)
{
    //drop a fever pattern on the field or increase feverchain amount & fevertime
    //implementation of allclear is inside player object (checkstartfever and checkendfever)
    thisplayer->allClear=1;
}

void ruleset_ENDLESSFEVERVS::onLose(player* thisplayer)
{
    thisplayer->losewin=LOSE;
    thisplayer->currentphase=LOSEDROP;
}

void ruleset_ENDLESSFEVERVS::onWin(player* thisplayer)
{
    thisplayer->losewin=WIN;
    thisplayer->currentphase=WIN_IDLE;
    //sound
    if (thisplayer==currentgame->players[0])
        data->snd.win.Play(data);
    else
        data->snd.lose.Play(data);
}

void ruleset_ENDLESSFEVERVS::onChain(player*)
{
    //thisplayer->forgiveGarbage=true;
    //thisplayer->feverColor=getRandom(5);
}

void ruleset_ENDLESSFEVERVS::onOffset(player* thisplayer)
{
    thisplayer->forgiveGarbage=true;
    thisplayer->addFeverCount();
}

void ruleset_ENDLESSFEVERVS::onAttack(player* thisplayer)
{
    if (!thisplayer->feverMode && thisplayer->feverGauge.seconds<60*30)
    {
        thisplayer->feverGauge.addTime(60);
        thisplayer->showSecondsObj(60);
    }
}

int ruleset_ENDLESSFEVERVS::getLinkBonus(int n)
{
    n--;
    if (n<0)
        return 0;

    if (n>=11)
        return linkBonus_FEVER[10];
    else
        return linkBonus_FEVER[n];
}

int ruleset_ENDLESSFEVERVS::getColorBonus(int n)
{
    n--;
    if (n<0)
        return 0;

    if (n>=5)
        return colorBonus_FEVER[4];
    else
        return colorBonus_FEVER[n];
}

int ruleset_ENDLESSFEVERVS::getChainBonus(player* pl)
{
    int n=pl->chain;
    int character=pl->getCharacter();
    n--;
    if (n<0)
        return 0;

    if (n>23)
        return 999;
    else
    {
        if (pl->feverMode)
            return chainBonus_FEVER[character][n];
        else
            return chainBonus_NORMAL[character][n];
    }
}
//}

//{TSU_ONLINE
//==========================================================
ruleset_TSU_ONLINE::ruleset_TSU_ONLINE()
{
    delayedFall=false;
    doubleSpawn=false;
    addDropBonus=true;
    voicePatternFever=false;
}

ruleset_TSU_ONLINE::~ruleset_TSU_ONLINE()
{

}


phase ruleset_TSU_ONLINE::endPhase(phase currentPhase,player* thisplayer)
{
    switch (currentPhase)
    {
        case GETREADY:
            if (thisplayer->getPlayerType()==ONLINE)
                return WAITGARBAGE;
            else
                return DROPGARBAGE;
            break;
        case PREPARE:
            return MOVE;
            break;
        case MOVE:
            thisplayer->createPuyo=true;
            return CREATEPUYO;
            break;
        case CREATEPUYO:
            return DROPPUYO;
            break;
        case DROPPUYO:
            return FALLPUYO;
            break;
        case FALLPUYO:
            return SEARCHCHAIN;
            break;
        case SEARCHCHAIN:
            //continue to phase 40 if no chain found, else to 32
            if (!thisplayer->foundChain)
                return CHECKALLCLEAR;
            else //found chain
                thisplayer->destroyPuyosTimer=1;
                return DESTROYPUYO;
            break;
        case DESTROYPUYO:
            return GARBAGE;
            break;
        case GARBAGE:
            return DROPPUYO;
            break;
        case CHECKALLCLEAR:
            if (thisplayer->getPlayerType()==ONLINE)
                return WAITGARBAGE;
            else
                return DROPGARBAGE;
            break;
        case WAITGARBAGE:
            return FALLGARBAGE;
            break;
        case DROPGARBAGE:
            return FALLGARBAGE;
            break;
        case FALLGARBAGE:
            //expect other players to confirm your move
            if (thisplayer->getPlayerType()==HUMAN)
            {
                return WAITCONFIRMGARBAGE;
            }
            else
                return CHECKLOSER;
            break;
        case WAITCONFIRMGARBAGE:
            return CHECKLOSER;
            break;
        case CHECKLOSER:
            return PREPARE;
            break;

        default:
            return IDLE;

    }
}

void ruleset_TSU_ONLINE::onInit(player* thisplayer)
{
    thisplayer->targetPoint=targetPoint;
    thisplayer->feverGauge.setVisible(false);
    thisplayer->useDropPattern=false;
}

void ruleset_TSU_ONLINE::onAllClearPop(player *thisplayer)
{
    //add 30 nuisance puyos
    //triggers during phase 30
    if (thisplayer->currentphase==SEARCHCHAIN)
    {
        thisplayer->currentScore+=30*thisplayer->targetPoint;
        thisplayer->allClear=0;
    }
}

void ruleset_TSU_ONLINE::onWin(player* thisplayer)
{
    thisplayer->losewin=WIN;
    thisplayer->currentphase=WIN_IDLE;
    //sound if player 1 wins or loses
    if (thisplayer==currentgame->players[0])
        data->snd.win.Play(data);
    else
        data->snd.lose.Play(data);
}

void ruleset_TSU_ONLINE::onLose(player* thisplayer)
{
    //human online player: wait for confirmation
    if (thisplayer->getPlayerType()==HUMAN)
        thisplayer->losewin=LOSEWAIT;
    else //no need to wait for anything
        thisplayer->losewin=LOSE;

    thisplayer->currentphase=LOSEDROP;
    //sounds for losing (not player 1)
    //debugstring=to_string(currentgame->getActivePlayers());
    if (thisplayer!=currentgame->players[0] && currentgame->getActivePlayers()!=1)
        data->snd.lose.Play(data);
}

int ruleset_TSU_ONLINE::getLinkBonus(int n)
{
    n--;
    if (n<0)
        return 0;

    if (n>=11)
        return linkBonus_TSU[10];
    else
        return linkBonus_TSU[n];
}

int ruleset_TSU_ONLINE::getColorBonus(int n)
{
    n--;
    if (n<0)
        return 0;

    if (n>=5)
        return colorBonus_TSU[4];
    else
        return colorBonus_TSU[n];
}

int ruleset_TSU_ONLINE::getChainBonus(player* pl)
{
    int n=pl->chain;
    n--;
    if (n<0)
            return 0;
    if (n>=16)
        return min(chainBonus_TSU[15]+32*(n - 15),999);
    else
        return chainBonus_TSU[n];
}

void ruleset_TSU_ONLINE::onOffset(player* thisplayer)
{
    thisplayer->forgiveGarbage=false;
}

//}

//{FEVER ONLINE
//==========================================================
ruleset_FEVER_ONLINE::ruleset_FEVER_ONLINE()
{
    delayedFall=true;
    doubleSpawn=true;
    allClearStart=false;
    feverDeath=false;
    bonusEQ=true;
}

ruleset_FEVER_ONLINE::~ruleset_FEVER_ONLINE()
{

}

phase ruleset_FEVER_ONLINE::endPhase(phase currentPhase,player* thisplayer)
{
    switch (currentPhase)
    {
        case GETREADY:
            if (thisplayer->getPlayerType()==ONLINE)
                return WAITGARBAGE;
            else
                return DROPGARBAGE;
            break;
        case PREPARE:
            return MOVE;
            break;
        case MOVE:
            thisplayer->createPuyo=true;
            return CREATEPUYO;
            break;
        case CREATEPUYO:
            return DROPPUYO;
            break;
        case DROPPUYO:
            return FALLPUYO;
            break;
        case FALLPUYO:
            return SEARCHCHAIN;
            break;
        case SEARCHCHAIN:
            //continue to phase 40 if no chain found, else to 32
            if (!thisplayer->foundChain)
                return CHECKALLCLEAR;
            else //found chain
                thisplayer->destroyPuyosTimer=1;
                return DESTROYPUYO;
            break;
        case DESTROYPUYO:
            //return DROPPUYO;
            return GARBAGE;
            break;
        case GARBAGE:
            return DROPPUYO;
            break;
        case CHECKALLCLEAR:
            if (thisplayer->feverMode)
            {
                if (thisplayer->getPlayerType()==ONLINE)
                    return CHECKENDFEVERONLINE;
                else
                    return CHECKENDFEVER;
            }
            else
            {
                //return DROPGARBAGE;
                if (thisplayer->getPlayerType()==ONLINE)
                    return WAITGARBAGE;
                else
                    return DROPGARBAGE;
            }
            break;
        case WAITGARBAGE:
            return FALLGARBAGE;
            break;
        case CHECKENDFEVERONLINE:
            return CHECKENDFEVER;
            break;
        case CHECKENDFEVER:
            if (thisplayer->feverEnd)
            {
                return ENDFEVER;
            }
            else
            {
                if (!thisplayer->poppedChain) //no chain popped: normal progression
                {
                    //return DROPGARBAGE;
                    if (thisplayer->getPlayerType()==ONLINE)
                        return WAITGARBAGE;
                    else
                        return DROPGARBAGE;
                }
                else //player popped a chain, drop a new feverchain
                {
                    thisplayer->poppedChain=false; //chain must be reset here
                    return DROPFEVER;
                }
            }
            break;
        case ENDFEVER:
                //return DROPGARBAGE;
            if (thisplayer->getPlayerType()==ONLINE)
                return WAITGARBAGE;
            else
                return DROPGARBAGE;
            break;

        case DROPGARBAGE:
            return FALLGARBAGE;
            break;
        case FALLGARBAGE:
            //expect other players to confirm your move
            if (thisplayer->getPlayerType()==HUMAN)
            {
                return WAITCONFIRMGARBAGE;
            }
            else
                return CHECKLOSER;
            break;
        case WAITCONFIRMGARBAGE:
            return CHECKLOSER;
            break;
        break;
        case CHECKLOSER:
            if (!thisplayer->feverMode)
                return CHECKFEVER; //not in fever: check if fever should start
            else
                return PREPARE;
            break;
        case CHECKFEVER:
            if (thisplayer->feverMode)
                return PREPAREFEVER;
            else
            {
                if (thisplayer->allClear==0)
                    return PREPARE;
                else
                {
                    thisplayer->allClear=0;
                    return DROPPUYO;
                }
            }
            break;
        case PREPAREFEVER:
            return DROPFEVER;
            break;
        case DROPFEVER:
            return DROPPUYO;

        case LOSEDROP:
            return LOSEDROP; //in fever it's possible the game tries to call endphase in fever mode
        case WAITLOSE:
            return WAITLOSE;

        default:
            return IDLE;
    }
}

void ruleset_FEVER_ONLINE::onSetGame()
{
    currentgame->legacyRandomizer = true;
    currentgame->legacyNuisanceDrop = true;
}

void ruleset_FEVER_ONLINE::onInit(player* thisplayer)
{
    thisplayer->targetPoint=targetPoint;
    thisplayer->feverGauge.setVisible(true);
    puyoToClear=4; //must be 4!
}

void ruleset_FEVER_ONLINE::onAllClearPop(player *)
{
    //does nothing
}

void ruleset_FEVER_ONLINE::onAllClear(player* thisplayer)
{
    //drop a fever pattern on the field or increase feverchain amount & fevertime
    //implementation of allclear is inside player object (checkstartfever and checkendfever)
    thisplayer->allClear=1;
}

void ruleset_FEVER_ONLINE::onLose(player* thisplayer)
{
    //human online player: wait for confirmation
    if (thisplayer->getPlayerType()==HUMAN)
        thisplayer->losewin=LOSEWAIT;
    else //no need to wait for anything
        thisplayer->losewin=LOSE;

    thisplayer->currentphase=LOSEDROP;
    if (thisplayer!=currentgame->players[0] && currentgame->getActivePlayers()!=1)
        data->snd.lose.Play(data);
}

void ruleset_FEVER_ONLINE::onWin(player* thisplayer)
{
    thisplayer->losewin=WIN;
    thisplayer->currentphase=WIN_IDLE;
    //sound
    if (thisplayer==currentgame->players[0])
        data->snd.win.Play(data);
    else
        data->snd.lose.Play(data);
}

void ruleset_FEVER_ONLINE::onChain(player*)
{
    //thisplayer->forgiveGarbage=true;
    //thisplayer->feverColor=getRandom(5);
}

void ruleset_FEVER_ONLINE::onOffset(player* thisplayer)
{
    thisplayer->forgiveGarbage=true;
    thisplayer->addFeverCount();
    //add a second to last attacker
    if (thisplayer->lastAttacker==NULL)
        return;

    if (!thisplayer->lastAttacker->feverMode && thisplayer->lastAttacker->feverGauge.seconds<60*30)
    {
        thisplayer->lastAttacker->feverGauge.addTime(60);
        thisplayer->lastAttacker->showSecondsObj(60);
    }
}

void ruleset_FEVER_ONLINE::onAttack(player*)
{

}

int ruleset_FEVER_ONLINE::getLinkBonus(int n)
{
    n--;
    if (n<0)
        return 0;

    if (n>=11)
        return linkBonus_FEVER[10];
    else
        return linkBonus_FEVER[n];
}

int ruleset_FEVER_ONLINE::getColorBonus(int n)
{
    n--;
    if (n<0)
        return 0;

    if (n>=5)
        return colorBonus_FEVER[4];
    else
        return colorBonus_FEVER[n];
}

int ruleset_FEVER_ONLINE::getChainBonus(player* pl)
{
    int n=pl->chain;
    int character=pl->getCharacter();
    n--;
    if (n<0)
        return 0;

    if (n==0)
        return 0; //first value is 0 in PP20th

    if (n>23)
        return 999;
    else
    {
        if (pl->feverMode)
            return chainBonus_FEVER[character][n]*(699.0/999.0);
        else
            return chainBonus_NORMAL[character][n]*(699.0/999.0);
    }
}
//}

//{FEVER15 ONLINE
//==========================================================
ruleset_FEVER15_ONLINE::ruleset_FEVER15_ONLINE()
{
    delayedFall=true;
    doubleSpawn=true;
    allClearStart=false;
    NFeverChains=12;
}

ruleset_FEVER15_ONLINE::~ruleset_FEVER15_ONLINE()
{

}

phase ruleset_FEVER15_ONLINE::endPhase(phase currentPhase,player* thisplayer)
{
    switch (currentPhase)
    {
        case GETREADY:
            if (thisplayer->getPlayerType()==ONLINE)
                return WAITGARBAGE;
            else
                return DROPGARBAGE;
            break;
        case PREPARE:
            return MOVE;
            break;
        case MOVE:
            thisplayer->createPuyo=true;
            return CREATEPUYO;
            break;
        case CREATEPUYO:
            return DROPPUYO;
            break;
        case DROPPUYO:
            return FALLPUYO;
            break;
        case FALLPUYO:
            return SEARCHCHAIN;
            break;
        case SEARCHCHAIN:
            //continue to phase 40 if no chain found, else to 32
            if (!thisplayer->foundChain)
                return CHECKALLCLEAR;
            else //found chain
                thisplayer->destroyPuyosTimer=1;
                return DESTROYPUYO;
            break;
        case DESTROYPUYO:
            //return DROPPUYO;
            return GARBAGE;
            break;
        case GARBAGE:
            return DROPPUYO;
            break;
        case CHECKALLCLEAR:
            if (thisplayer->feverMode)
            {
                if (thisplayer->getPlayerType()==ONLINE)
                    return CHECKENDFEVERONLINE;
                else
                    return CHECKENDFEVER;
            }
            else
            {
                //return DROPGARBAGE;
                if (thisplayer->getPlayerType()==ONLINE)
                    return WAITGARBAGE;
                else
                    return DROPGARBAGE;
            }
            break;
        case WAITGARBAGE:
            return FALLGARBAGE;
            break;
        case CHECKENDFEVERONLINE:
            return CHECKENDFEVER;
            break;
        case CHECKENDFEVER:
            if (thisplayer->feverEnd)
            {
                return ENDFEVER;
            }
            else
            {
                if (!thisplayer->poppedChain) //no chain popped: normal progression
                {
                    //return DROPGARBAGE;
                    if (thisplayer->getPlayerType()==ONLINE)
                        return WAITGARBAGE;
                    else
                        return DROPGARBAGE;
                }
                else //player popped a chain, drop a new feverchain
                {
                    thisplayer->poppedChain=false; //chain must be reset here
                    return DROPFEVER;
                }
            }
            break;
        case ENDFEVER:
                //return DROPGARBAGE;
            if (thisplayer->getPlayerType()==ONLINE)
                return WAITGARBAGE;
            else
                return DROPGARBAGE;
            break;

        case DROPGARBAGE:
            return FALLGARBAGE;
            break;
        case FALLGARBAGE:
            //expect other players to confirm your move
            if (thisplayer->getPlayerType()==HUMAN)
            {
                return WAITCONFIRMGARBAGE;
            }
            else
                return CHECKLOSER;
            break;
        case WAITCONFIRMGARBAGE:
            return CHECKLOSER;
            break;
        break;
        case CHECKLOSER:
            if (!thisplayer->feverMode)
                return CHECKFEVER; //not in fever: check if fever should start
            else
                return PREPARE;
            break;
        case CHECKFEVER:
            if (thisplayer->feverMode)
                return PREPAREFEVER;
            else
            {
                if (thisplayer->allClear==0)
                    return PREPARE;
                else
                {
                    thisplayer->allClear=0;
                    return DROPPUYO;
                }
            }
            break;
        case PREPAREFEVER:
            return DROPFEVER;
            break;
        case DROPFEVER:
            return DROPPUYO;

        case LOSEDROP:
            return LOSEDROP; //in fever it's possible the game tries to call endphase in fever mode
        case WAITLOSE:
            return WAITLOSE;

        default:
            return IDLE;
    }
}

void ruleset_FEVER15_ONLINE::onSetGame()
{
    currentgame->legacyRandomizer = true;
    currentgame->legacyNuisanceDrop = true;
}

void ruleset_FEVER15_ONLINE::onInit(player* thisplayer)
{
    thisplayer->targetPoint=targetPoint;
    thisplayer->feverGauge.setVisible(true);
    puyoToClear=4; //must be 4!
}

void ruleset_FEVER15_ONLINE::onAllClearPop(player *)
{
    //does nothing
}

void ruleset_FEVER15_ONLINE::onAllClear(player* thisplayer)
{
    //drop a fever pattern on the field or increase feverchain amount & fevertime
    //implementation of allclear is inside player object (checkstartfever and checkendfever)
    thisplayer->allClear=1;
}

void ruleset_FEVER15_ONLINE::onLose(player* thisplayer)
{
    //human online player: wait for confirmation
    if (thisplayer->getPlayerType()==HUMAN)
        thisplayer->losewin=LOSEWAIT;
    else //no need to wait for anything
        thisplayer->losewin=LOSE;

    thisplayer->currentphase=LOSEDROP;
    if (thisplayer!=currentgame->players[0] && currentgame->getActivePlayers()!=1)
        data->snd.lose.Play(data);
}

void ruleset_FEVER15_ONLINE::onWin(player* thisplayer)
{
    thisplayer->losewin=WIN;
    thisplayer->currentphase=WIN_IDLE;
    //sound
    if (thisplayer==currentgame->players[0])
        data->snd.win.Play(data);
    else
        data->snd.lose.Play(data);
}

void ruleset_FEVER15_ONLINE::onChain(player*)
{
    //thisplayer->forgiveGarbage=true;
    //thisplayer->feverColor=getRandom(5);
}

void ruleset_FEVER15_ONLINE::onOffset(player* thisplayer)
{
    thisplayer->forgiveGarbage=true;
    thisplayer->addFeverCount();
}

void ruleset_FEVER15_ONLINE::onAttack(player* thisplayer)
{
    if (!thisplayer->feverMode && thisplayer->feverGauge.seconds<60*30)
    {
        thisplayer->feverGauge.addTime(60);
        thisplayer->showSecondsObj(60);
    }
}

int ruleset_FEVER15_ONLINE::getLinkBonus(int n)
{
    n--;
    if (n<0)
        return 0;

    if (n>=11)
        return linkBonus_FEVER[10];
    else
        return linkBonus_FEVER[n];
}

int ruleset_FEVER15_ONLINE::getColorBonus(int n)
{
    n--;
    if (n<0)
        return 0;

    if (n>=5)
        return colorBonus_FEVER[4];
    else
        return colorBonus_FEVER[n];
}

int ruleset_FEVER15_ONLINE::getChainBonus(player* pl)
{
    int n=pl->chain;
    int character=pl->getCharacter();
    n--;
    if (n<0)
        return 0;

    if (n>23)
        return 999;
    else
    {
        if (pl->feverMode)
            return chainBonus_FEVER[character][n];
        else
            return chainBonus_NORMAL[character][n];
    }
}
//}


//{ENDLESSFEVERVS ONLINE
//==========================================================
ruleset_ENDLESSFEVERVS_ONLINE::ruleset_ENDLESSFEVERVS_ONLINE()
{
    delayedFall=true;
    doubleSpawn=true;
    allClearStart=false;
    NFeverChains=12;
}

ruleset_ENDLESSFEVERVS_ONLINE::~ruleset_ENDLESSFEVERVS_ONLINE()
{

}

phase ruleset_ENDLESSFEVERVS_ONLINE::endPhase(phase currentPhase,player* thisplayer)
{
    switch (currentPhase)
    {
        case GETREADY:
            return CHECKFEVER;
            break;
        case PREPARE:
            return MOVE;
            break;
        case MOVE:
            thisplayer->createPuyo=true;
            return CREATEPUYO;
            break;
        case CREATEPUYO:
            return DROPPUYO;
            break;
        case DROPPUYO:
            return FALLPUYO;
            break;
        case FALLPUYO:
            return SEARCHCHAIN;
            break;
        case SEARCHCHAIN:
            //continue to phase 40 if no chain found, else to 32
            if (!thisplayer->foundChain)
                return CHECKALLCLEAR;
            else //found chain
                thisplayer->destroyPuyosTimer=1;
                return DESTROYPUYO;
            break;
        case DESTROYPUYO:
            //return DROPPUYO;
            return GARBAGE;
            break;
        case GARBAGE:
            return DROPPUYO;
            break;
        case CHECKALLCLEAR:
            if (thisplayer->feverMode)
            {
                if (thisplayer->getPlayerType()==ONLINE)
                    return CHECKENDFEVERONLINE;
                else
                    return CHECKENDFEVER;
            }
            else
            {
                //return DROPGARBAGE;
                if (thisplayer->getPlayerType()==ONLINE)
                    return WAITGARBAGE;
                else
                    return DROPGARBAGE;
            }
            break;
        case WAITGARBAGE:
            return FALLGARBAGE;
            break;
        case CHECKENDFEVERONLINE:
            return CHECKENDFEVER;
            break;
        case CHECKENDFEVER:
            if (thisplayer->feverEnd)
            {
                return ENDFEVER;
            }
            else
            {
                if (!thisplayer->poppedChain) //no chain popped: normal progression
                {
                    //return DROPGARBAGE;
                    if (thisplayer->getPlayerType()==ONLINE)
                        return WAITGARBAGE;
                    else
                        return DROPGARBAGE;
                }
                else //player popped a chain, drop a new feverchain
                {
                    thisplayer->poppedChain=false; //chain must be reset here
                    return DROPFEVER;
                }
            }
            break;
        case ENDFEVER:
                //return DROPGARBAGE;
            if (thisplayer->getPlayerType()==ONLINE)
                return WAITGARBAGE;
            else
                return DROPGARBAGE;
            break;

        case DROPGARBAGE:
            return FALLGARBAGE;
            break;
        case FALLGARBAGE:
            //expect other players to confirm your move
            if (thisplayer->getPlayerType()==HUMAN)
            {
                return WAITCONFIRMGARBAGE;
            }
            else
                return CHECKLOSER;
            break;
        case WAITCONFIRMGARBAGE:
            return CHECKLOSER;
            break;
        break;
        case CHECKLOSER:
            if (!thisplayer->feverMode)
                return CHECKFEVER; //not in fever: check if fever should start
            else
                return PREPARE;
            break;
        case CHECKFEVER:
            if (thisplayer->feverMode)
                return PREPAREFEVER;
            else
            {
                if (thisplayer->allClear==0)
                    return PREPARE;
                else
                {
                    thisplayer->allClear=0;
                    return DROPPUYO;
                }
            }
            break;
        case PREPAREFEVER:
            return DROPFEVER;
            break;
        case DROPFEVER:
            return DROPPUYO;

        case LOSEDROP:
            return LOSEDROP; //in fever it's possible the game tries to call endphase in fever mode
        case WAITLOSE:
            return WAITLOSE;

        default:
            return IDLE;
    }
}

void ruleset_ENDLESSFEVERVS_ONLINE::onSetGame()
{
    currentgame->legacyRandomizer = true;
    currentgame->legacyNuisanceDrop = true;
}

void ruleset_ENDLESSFEVERVS_ONLINE::onInit(player* thisplayer)
{
    thisplayer->targetPoint=targetPoint;
    thisplayer->feverGauge.setVisible(true);
    puyoToClear=4; //must be 4!
    thisplayer->feverGauge.setCount(7);
    thisplayer->feverGauge.setSeconds(99*60);
    thisplayer->feverGauge.endless=true;
    thisplayer->normalGarbage.GQ=270;
    thisplayer->updateTray();
    thisplayer->nextPuyoActive=false;
}

void ruleset_ENDLESSFEVERVS_ONLINE::onAllClearPop(player *)
{
    //does nothing
}

void ruleset_ENDLESSFEVERVS_ONLINE::onAllClear(player* thisplayer)
{
    //drop a fever pattern on the field or increase feverchain amount & fevertime
    //implementation of allclear is inside player object (checkstartfever and checkendfever)
    thisplayer->allClear=1;
}

void ruleset_ENDLESSFEVERVS_ONLINE::onLose(player* thisplayer)
{
    //human online player: wait for confirmation
    if (thisplayer->getPlayerType()==HUMAN)
        thisplayer->losewin=LOSEWAIT;
    else //no need to wait for anything
        thisplayer->losewin=LOSE;

    thisplayer->currentphase=LOSEDROP;
    if (thisplayer!=currentgame->players[0] && currentgame->getActivePlayers()!=1)
        data->snd.lose.Play(data);
}

void ruleset_ENDLESSFEVERVS_ONLINE::onWin(player* thisplayer)
{
    thisplayer->losewin=WIN;
    thisplayer->currentphase=WIN_IDLE;
    //sound
    if (thisplayer==currentgame->players[0])
        data->snd.win.Play(data);
    else
        data->snd.lose.Play(data);
}

void ruleset_ENDLESSFEVERVS_ONLINE::onChain(player*)
{
    //thisplayer->forgiveGarbage=true;
    //thisplayer->feverColor=getRandom(5);
}

void ruleset_ENDLESSFEVERVS_ONLINE::onOffset(player* thisplayer)
{
    thisplayer->forgiveGarbage=true;
    thisplayer->addFeverCount();
}

void ruleset_ENDLESSFEVERVS_ONLINE::onAttack(player* thisplayer)
{
    if (!thisplayer->feverMode && thisplayer->feverGauge.seconds<60*30)
    {
        thisplayer->feverGauge.addTime(60);
        thisplayer->showSecondsObj(60);
    }
}

int ruleset_ENDLESSFEVERVS_ONLINE::getLinkBonus(int n)
{
    n--;
    if (n<0)
        return 0;

    if (n>=11)
        return linkBonus_FEVER[10];
    else
        return linkBonus_FEVER[n];
}

int ruleset_ENDLESSFEVERVS_ONLINE::getColorBonus(int n)
{
    n--;
    if (n<0)
        return 0;

    if (n>=5)
        return colorBonus_FEVER[4];
    else
        return colorBonus_FEVER[n];
}

int ruleset_ENDLESSFEVERVS_ONLINE::getChainBonus(player* pl)
{
    int n=pl->chain;
    int character=pl->getCharacter();
    n--;
    if (n<0)
        return 0;

    if (n>23)
        return 999;
    else
    {
        if (pl->feverMode)
            return chainBonus_FEVER[character][n];
        else
            return chainBonus_NORMAL[character][n];
    }
}
//}


//{EXCAVATION
//==========================================================
ruleset_EXCAVATION::ruleset_EXCAVATION()
{
    delayedFall=true;
    doubleSpawn=true;
    addDropBonus=true;
    voicePatternFever=true;
}

ruleset_EXCAVATION::~ruleset_EXCAVATION()
{

}


phase ruleset_EXCAVATION::endPhase(phase currentPhase,player* thisplayer)
{
    switch (currentPhase)
    {
        case GETREADY:
            return DROPPUYO;
            break;
        case PREPARE:
            return MOVE;
            break;
        case MOVE:
            thisplayer->createPuyo=true;
            return CREATEPUYO;
            break;
        case CREATEPUYO:
            return DROPPUYO;
            break;
        case DROPPUYO:
            return FALLPUYO;
            break;
        case FALLPUYO:
            return SEARCHCHAIN;
            break;
        case SEARCHCHAIN:
            //continue to phase 40 if no chain found, else to 32
            if (!thisplayer->foundChain)
                return CHECKALLCLEAR;
            else //found chain
                thisplayer->destroyPuyosTimer=1;
                return DESTROYPUYO;
            break;
        case DESTROYPUYO:
            return GARBAGE;
            break;
        case GARBAGE:
            return DROPPUYO;
            break;
        case CHECKALLCLEAR:
            return DROPGARBAGE;
            break;
        case DROPGARBAGE:
            return FALLGARBAGE;
            break;
        case FALLGARBAGE:
            return CHECKLOSER;
            break;
        case CHECKLOSER:
            return PREPARE;
            break;

        default:
            return IDLE;

    }
}

void ruleset_EXCAVATION::onInit(player* thisplayer)
{
    thisplayer->targetPoint=targetPoint;
    thisplayer->feverGauge.setVisible(false);
    thisplayer->useDropPattern=false;
}

void ruleset_EXCAVATION::onAllClearPop(player *thisplayer)
{
}

void ruleset_EXCAVATION::onWin(player* thisplayer)
{
    thisplayer->losewin=WIN;
    thisplayer->currentphase=WIN_IDLE;
    //sound if player 1 wins or loses
    if (thisplayer==currentgame->players[0])
        data->snd.win.Play(data);
    else
        data->snd.lose.Play(data);
}

void ruleset_EXCAVATION::onLose(player* thisplayer)
{
    thisplayer->losewin=LOSE;
    thisplayer->currentphase=LOSEDROP;
    //sounds for losing (not player 1)
    if (thisplayer!=currentgame->players[0] && currentgame->getActivePlayers()!=1)
        data->snd.lose.Play(data);
}

int ruleset_EXCAVATION::getLinkBonus(int n)
{
    n--;
    if (n<0)
        return 0;

    if (n>=11)
        return linkBonus_TSU[10];
    else
        return linkBonus_TSU[n];
}

int ruleset_EXCAVATION::getColorBonus(int n)
{
    n--;
    if (n<0)
        return 0;

    if (n>=5)
        return colorBonus_TSU[4];
    else
        return colorBonus_TSU[n];
}

int ruleset_EXCAVATION::getChainBonus(player* pl)
{
    int n=pl->chain;
    n--;
    if (n<0)
            return 0;
    if (n>=16)
        return min(chainBonus_TSU[15]+32*(n - 15),999);
    else
        return chainBonus_TSU[n];
}
//}


}
