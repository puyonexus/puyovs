#include "game.h"
#include <ctime>
#include <fstream>
#include <cstdio>
#include <zlib.h>
#include "../PVS_ENet/PVS_Client.h"
#include "../PVS_ENet/PVS_Channel.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace ppvs
{

void encode(const char *key,char* in,int length)
{
    for(int i=0;i<length;i++)
        in[i]=in[i]^key[i%strlen(key)];
}

game* activegame= nullptr;

game::game(gameSettings *gs)
{
    initGlobal();
    data = nullptr;
    currentruleset= nullptr;

    settings=gs;
    connected=false;
    menuSelect=0;
    currentGameStatus=GAMESTATUS_WAITING;
    stopChaining=false;
    activegame=this;
    if (gs->rankedMatch == true) {
        choiceTimer=5*60;
    } else {
        choiceTimer=30*60;
    }
    colorTimer=10*60;
    activeAtStart=0;
    replayTimer=3*60;
    rankedTimer=15*60;
    rankedState=0;
    currentReplayVersion=0;
    replayState=REPLAYSTATE_NORMAL;
    replayBackwardsTimer=0;
    backwardsOnce=false;
    legacyRandomizer = false;
    legacyNuisanceDrop = false;

    playNext=true;
    forceStatusText=false;

    targetVolumeNormal=100;
    currentVolumeNormal=0;
    targetVolumeFever=100;
    currentVolumeFever=100;
    globalVolume=1.0f;

    black.setImage(nullptr);
    black.setScale(640*2,480);
    black.setColor(0,0,0);
    black.setTransparency(0.5f);
    black.setPosition(-640/2,-480/4);

    statusFont = nullptr;
    statusText = nullptr;
}

game::~game()
{
    //delete players
    while (players.size()>0)
    {
        delete players.back();
        players.pop_back();
    }

    //delete associated global properties
    delete settings;
    //delete readyGoObj;
    delete charSelectMenu;
    delete mainMenu;
    delete currentruleset;
    delete statusText;
    delete statusFont;
    delete data->front;
    delete data;

}

void game::close()
{//Close game
    runGame=false;
}

void game::loadGlobal()
{
    //Load usersettings
    data->gUserSettings.str_background=settings->background;
    data->gUserSettings.str_puyo=settings->puyo;
    data->gUserSettings.str_sfx=settings->sfx;

    //Set global images
    loadImages();

    //Load all sounds
    loadSounds();

    //Init shaders
    data->glowShader = nullptr;
    data->tunnelShader = nullptr;

    if(useShaders)
    {
        const static char *glowShaderSource =
        "uniform sampler2D tex;"
        "uniform float color;"
        "void main()"
        "{"
        "   gl_FragColor=texture2D(tex,gl_TexCoord[0].xy)+vec4(color,color,color,0);"
        "}";

        data->glowShader = data->front->loadShader(glowShaderSource);
        if(data->glowShader)
        {
            data->glowShader->setCurrentTexture("tex");
            data->glowShader->setParameter("color", 1.0f);
        }

        const static char *tunnelShaderSource =
        "uniform vec4 cl;"
        "uniform float time;"
        "uniform sampler2D tex;"
        "void main(void) {"
        "   vec2 ccord = gl_TexCoord[0].xy;"
        "   vec2 pcord;"
        "   vec2 final;"
        "   float zoomspeed=0.5;"
        "   float rotatespeed=0.25;"
        "   ccord.x = step(0.5,ccord.y)*(-1.0+ 2.0*ccord.x)-(1.0-step(0.5,ccord.y))*(-1.0+ 2.0*ccord.x);"
        "   ccord.y = step(0.5,ccord.y)*(-1.0+ 2.0*ccord.y)-(1.0-step(0.5,ccord.y))*(-1.0+ 2.0*ccord.y);"
        "   pcord.x = 0.1/sqrt(ccord.x*ccord.x+ccord.y*ccord.y);pcord.y = atan(ccord.y,ccord.x)/3.141592;"
        "   final.x = step(0.25,pcord.x)*mod(pcord.x+zoomspeed*time,0.5)+(1.0-step(0.25,pcord.x))*mod(pcord.x+zoomspeed*time,0.5);"
        "   final.y = step(0.25,pcord.y)*mod(pcord.y+rotatespeed*time,0.5)+(1.0-step(0.25,pcord.y))*mod(pcord.y+rotatespeed*time,0.5);"
        "   vec3 col = texture2D(tex,final).xyz;"
        "   gl_FragColor = vec4(max(col/((pcord/0.1).x),0.1), 1.0) * cl;"
        "}";

        data->tunnelShader = data->front->loadShader(tunnelShaderSource);
        if(data->tunnelShader)
        {
            data->tunnelShader->setCurrentTexture("tex");
            data->tunnelShader->setParameter("time", 1.0f);
            data->tunnelShader->setParameter("cl", 0.0f, 0.0f, 1.0f, 1.0f);
        }
    }

    //other
    data->globalTimer=0;
    data->windowFocus=true;
    data->playSounds=settings->playSound;
    data->playMusic=settings->playMusic;

    statusFont = data->front->loadFont("Arial", 14);
    setStatusText("");
}

void game::loadSounds()
{
    sounds &snd = data->snd;
    setBuffer(snd.chain[0],(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/chain1.ogg"))));
    setBuffer(snd.chain[1],(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/chain2.ogg"))));
    setBuffer(snd.chain[2],(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/chain3.ogg"))));
    setBuffer(snd.chain[3],(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/chain4.ogg"))));
    setBuffer(snd.chain[4],(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/chain5.ogg"))));
    setBuffer(snd.chain[5],(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/chain6.ogg"))));
    setBuffer(snd.chain[6],(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/chain7.ogg"))));
    setBuffer(snd.allcleardrop,(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/allclear.ogg"))));
    setBuffer(snd.drop,(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/drop.ogg"))));
    setBuffer(snd.fever,(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/fever.ogg"))));
    setBuffer(snd.feverlight,(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/feverlight.ogg"))));
    setBuffer(snd.fevertimecount,(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/fevertimecount.ogg"))));
    setBuffer(snd.fevertimeend,(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/fevertimeend.ogg"))));
    setBuffer(snd.go,(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/go.ogg"))));
    setBuffer(snd.heavy,(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/heavy.ogg"))));
    setBuffer(snd.hit,(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/hit.ogg"))));
    setBuffer(snd.lose,(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/lose.ogg"))));
    setBuffer(snd.move,(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/move.ogg"))));
    setBuffer(snd.nuisance_hitL,(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/nuisance_hitL.ogg"))));
    setBuffer(snd.nuisance_hitM,(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/nuisance_hitM.ogg"))));
    setBuffer(snd.nuisance_hitS,(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/nuisance_hitS.ogg"))));
    setBuffer(snd.nuisanceL,(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/nuisanceL.ogg"))));
    setBuffer(snd.nuisanceS,(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/nuisanceS.ogg"))));
    setBuffer(snd.ready,(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/ready.ogg"))));
    setBuffer(snd.rotate,(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/rotate.ogg"))));
    setBuffer(snd.win,(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/win.ogg"))));
    setBuffer(snd.decide,(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/decide.ogg"))));
    setBuffer(snd.cancel,(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/cancel.ogg"))));
    setBuffer(snd.cursor,(data->front->loadSound(folder_user_sounds+data->gUserSettings.str_sfx+std::string("/cursor.ogg"))));

}

void game::loadImages()
{//Load images (user defined)
    //Load puyo
    data->imgPuyo=data->front->loadImage(folder_user_puyo+data->gUserSettings.str_puyo+std::string(".png"));

    //Check rotation center of quadruplet
    if(data->imgPuyo && !data->imgPuyo->error() && data->imgPuyo->height()>10)
    {
        for (int i=0;i<PUYOX;i++)
        {
            if (data->imgPuyo->pixel(11*PUYOX-i,14*PUYOY).a>50)
            {
                data->PUYOXCENTER=PUYOX-i;
                break;
            }
        }
    }
    data->imgPuyo->setFilter(linearFilter);

    //Lights
    data->imgLight = data->front->loadImage("Data/Light.png");
    data->imgLight_s = data->front->loadImage("Data/Light_s.png");
    data->imgLight_hit = data->front->loadImage("Data/Light_hit.png");
    data->imgfsparkle = data->front->loadImage("Data/CharSelect/fsparkle.png");
    data->imgfLight = data->front->loadImage("Data/fLight.png");
    data->imgfLight_hit = data->front->loadImage("Data/fLight_hit.png");

    data->imgTime = data->front->loadImage(folder_user_backgrounds+data->gUserSettings.str_background+std::string("/time.png"));

    //Menu
    char buffer[128];
    for(int i = 0; i < 3; ++i)
    {
        for(int j = 0; j < 2; ++j)
        {
            // safe (because i/j have defined ranges)
            sprintf(buffer, "Data/Menu/menu%i%i.png", i, j);
            data->imgMenu[i][j] = data->front->loadImage(buffer);
        }
    }

    //Backgrounds
    data->imgBackground=data->front->loadImage(folder_user_backgrounds+data->gUserSettings.str_background+std::string("/back.png"));
    data->imgFieldFever=data->front->loadImage(folder_user_backgrounds+data->gUserSettings.str_background+std::string("/ffield.png"));

    //Background of next puyo
    data->imgNextPuyoBackgroundR=data->front->loadImage(folder_user_backgrounds+data->gUserSettings.str_background+std::string("/nextR.png"));
    data->imgNextPuyoBackgroundL=data->front->loadImage(folder_user_backgrounds+data->gUserSettings.str_background+std::string("/nextL.png"));

    if(!useShaders)
    {
        for(int i = 0; i < 30; ++i)
            data->imgFeverBack[i] = data->front->loadImage(std::string("Data/Fever/f"+to_string(i)+".png").c_str());
    }

    //Load default fields. Custom fields should be loaded per character
    data->imgField1=data->front->loadImage(folder_user_backgrounds+data->gUserSettings.str_background+std::string("/field1.png"));
    data->imgField2=data->front->loadImage(folder_user_backgrounds+data->gUserSettings.str_background+std::string("/field2.png"));
    data->imgBorder1=data->front->loadImage(folder_user_backgrounds+data->gUserSettings.str_background+std::string("/border1.png"));
    data->imgBorder2=data->front->loadImage(folder_user_backgrounds+data->gUserSettings.str_background+std::string("/border2.png"));
    data->imgPlayerBorder=data->front->loadImage("Data/border.png");
    data->imgSpice=data->front->loadImage("Data/spice.png");
    //other
    data->imgScore=data->front->loadImage(folder_user_backgrounds+data->gUserSettings.str_background+std::string("/score.png"));
    data->imgAllClear=data->front->loadImage(folder_user_backgrounds+data->gUserSettings.str_background+std::string("/allclear.png"));
    data->imgLose=data->front->loadImage(folder_user_backgrounds+data->gUserSettings.str_background+std::string("/lose.png"));
    data->imgWin=data->front->loadImage(folder_user_backgrounds+data->gUserSettings.str_background+std::string("/win.png"));
    data->imgFeverGauge=data->front->loadImage(folder_user_backgrounds+data->gUserSettings.str_background+std::string("/fgauge.png"));
    data->imgSeconds=data->front->loadImage(folder_user_backgrounds+data->gUserSettings.str_background+std::string("/fcounter.png"));
    data->imgCharHolder = data->front->loadImage("Data/CharSelect/charHolder.png");
    data->imgNameHolder = data->front->loadImage("Data/CharSelect/nameHolder.png");
    data->imgBlack = data->front->loadImage("Data/CharSelect/black.png");
    data->imgDropset = data->front->loadImage("Data/CharSelect/dropset.png");
    data->imgChain = data->front->loadImage(std::string("User/Backgrounds/")+data->gUserSettings.str_background+std::string("/chain.png"));
    data->imgCheckmark = data->front->loadImage("Data/checkmark.png");
    data->imgPlayerCharSelect = data->front->loadImage("Data/CharSelect/charSelect.png");
    for(int i = 0; i < NUM_CHARACTERS; ++i)
    {
        data->imgCharField[i] = data->front->loadImage(folder_user_character+settings->characterSetup[i]+"/field.png");
        data->imgCharSelect[i] = data->front->loadImage(folder_user_character+settings->characterSetup[i]+"/select.png");
        data->imgCharName[i] = data->front->loadImage(folder_user_character+settings->characterSetup[i]+"/name.png");
        data->imgSelect[i] = data->front->loadImage(folder_user_character+settings->characterSetup[i]+"/select.png");
    }

    data->imgPlayerNumber = data->front->loadImage("Data/CharSelect/playernumber.png");
}

void game::initPlayers()
{
    //Create players according to settings
    int n=1;
    for (int i=0;i<settings->Nhumans;i++)
    {//assign human players
        addPlayer(HUMAN,n,settings->Nplayers);
        //give human controls
        n++;
    }

    if (settings->useCPUplayers)
    {
        for (int i=0;i<settings->Nplayers-settings->Nhumans;i++)
        {//CPU players
            addPlayer(CPU,n,settings->Nplayers);
            n++;
        }
    }
    else
    {
        for (int i=0;i<settings->Nplayers-settings->Nhumans;i++)
        {//Online players
            addPlayer(ONLINE,n,settings->Nplayers);
            n++;
        }
    }
}


int game::getActivePlayers()
{//count players that haven't lost
    int pl=0;
    for (size_t i=0;i<players.size();i++)
    {
        if (players[i]->losewin==NOWIN && players[i]->active)
            pl++;
    }
    return pl;
}

void game::initGame(frontend *f)
{
    data = new gameData;
    //global->nextPuyoRenderImage.create(65,128);
    //global->fieldRenderImage.create(192,336);
    data->front = f;
    loadGlobal();
    //set random seed
    randomSeed_NextList = getRandom(100000);
    //rule settings
    setRules();
    //set background
    m_spriteBackground.setImage(data->imgBackground);
    m_spriteBackground.setPosition(0.f, 0.f);
    initPlayers();
    runGame = true;
    if (!network) //start with character select menu or not
        menuSelect=int(settings->startWithCharacterSelect);
    timerEndMatch=0;

    //Initialize readygo animation
    readyGoObj.init(data,posVectorFloat(320,240),1,folder_user_backgrounds+data->gUserSettings.str_background+"/Animation/","ready.xml",3*60);
    backgroundAnimation.init(data,posVectorFloat(320,240),1,folder_user_backgrounds+data->gUserSettings.str_background+"/Animation/","animation.xml",30*60);
    backgroundAnimation.prepareAnimation("background");

    //Other stuff
    charSelectMenu = new characterSelect(this);
    charSelectMenu->prepare();
    mainMenu = new menu(this);

    timerSprite[0].setImage(data->imgPlayerNumber);
    timerSprite[1].setImage(data->imgPlayerNumber);
    timerSprite[0].setSubRect(0/10*24,0,0,32);
    timerSprite[1].setSubRect(0/10*24,0,0,32);
    timerSprite[0].setCenterBottom();
    timerSprite[1].setCenterBottom();
    timerSprite[0].setPosition(640-24,32);
    timerSprite[1].setPosition(640-24-24,32);

}

void game::setRules()
{//set rules from rulesetinfo
    if (currentruleset!= nullptr)
        delete currentruleset;

    //create rule object
    switch (settings->rulesetInfo.rulesetType)
    {
        case ENDLESS:
            currentruleset=new ruleset_ENDLESS();
            break;
        case TSU:
            currentruleset=new ruleset_TSU();
            break;
        case FEVER:
            currentruleset=new ruleset_FEVER();
            break;
        case ENDLESSFEVER:
            currentruleset=new ruleset_ENDLESSFEVER();
            break;
        case ENDLESSFEVERVS:
            currentruleset=new ruleset_ENDLESSFEVERVS();
            break;
        case TSU_ONLINE:
            currentruleset=new ruleset_TSU_ONLINE();
            break;
        case FEVER_ONLINE:
            currentruleset=new ruleset_FEVER_ONLINE();
            break;
        case FEVER15_ONLINE:
            currentruleset=new ruleset_FEVER15_ONLINE();
            break;
        case ENDLESSFEVERVS_ONLINE:
            currentruleset=new ruleset_ENDLESSFEVERVS_ONLINE();
            break;
        default:
            currentruleset=new ruleset_ENDLESS();
            break;
    }
    currentruleset->setGame(this);
    //if (settings->rulesetInfo.marginTime>0)
        currentruleset->marginTime=settings->rulesetInfo.marginTime*60;
    if (settings->rulesetInfo.targetPoint>0)
        currentruleset->targetPoint=settings->rulesetInfo.targetPoint;
    if (settings->rulesetInfo.feverPower>0)
        currentruleset->feverPower=static_cast<float>(settings->rulesetInfo.feverPower)/100.0f;
    if (settings->rulesetInfo.puyoToClear>0)
        currentruleset->puyoToClear=settings->rulesetInfo.puyoToClear;
    if (settings->rulesetInfo.requiredChain>=0)
        currentruleset->requiredChain=settings->rulesetInfo.requiredChain;
    if (settings->rulesetInfo.initialFeverCount>=0 && settings->rulesetInfo.initialFeverCount<=7)
        currentruleset->initialFeverCount=settings->rulesetInfo.initialFeverCount;
    if (settings->rulesetInfo.quickDrop)
        currentruleset->quickDrop=settings->rulesetInfo.quickDrop;
    if (settings->rulesetInfo.colors>=3 && settings->rulesetInfo.colors<=5)
    {
       for (size_t i=0;i<players.size();i++)
            players[i]->colors=settings->rulesetInfo.colors;
        settings->pickColors=false;
    }
    else
    {
       for (size_t i=0;i<players.size();i++)
            players[i]->colors=4;
    }
}

bool game::isFever()
{
    bool fever=false;
    for (size_t i=0;i<players.size();i++)
    {
        if (players[i]->active && players[i]->feverMode)
            fever=true;
    }
    return fever;
}

void game::playGame()
{
    if (settings->recording==PVS_REPLAYING && replayState==REPLAYSTATE_PAUSED)
        return;

    //Set controller states
    for(int i=0;i<players.size();i++)
    {
        if (settings->useCPUplayers && i >= settings->Nhumans)
            break;
        finput input = data->front->inputState(i);
        players[i]->controls.setstate(input, data->matchTimer);
    }

    //active player when NOT playing
    for(int i=0;i<players.size();i++)
    {
        if (currentGameStatus!=GAMESTATUS_PLAYING && players[i]->prepareActive)
        {
            players[i]->active=true;
            players[i]->prepareActive=false;
        }
    }

    //set choice timer
    if (currentGameStatus==GAMESTATUS_IDLE && !settings->useCPUplayers)
    {
        timerSprite[0].setSubRect(((choiceTimer/60)%10)*24,0,24,32);
        timerSprite[1].setSubRect((choiceTimer/60/10)*24,0,24,32);

        if (choiceTimer>10*60)
            timerSprite[1].setVisible(true);
        else
            timerSprite[1].setVisible(false);

        if (choiceTimer>0)
            choiceTimer--;

        //noone to rematch
        if (countActivePlayers()<=1 && settings->rankedMatch==false)
            choiceTimer=-1;
        //2 player game: no count down
        if (players.size() == 2 && settings->rankedMatch==false)
            choiceTimer=-1;

        //check if new player came on
        if (choiceTimer<0 && countActivePlayers()>1 && !settings->rankedMatch)
            choiceTimer=30*60;

        //ranked: start counting as soon as opponent enters
        if (choiceTimer<0 && countBoundPlayers()>1 && settings->rankedMatch==true)
            choiceTimer=5*60;

        //time is up
        if (choiceTimer==0)
        {
            players[0]->controls.release();
            players[0]->controls.A=1;
            //select rematch
            mainMenu->select=0;
        }

    }
    //color timer menu
    if (players[0]->getPlayerType()==HUMAN && players[0]->currentphase==PICKCOLORS && !settings->useCPUplayers)
    {
        timerSprite[0].setSubRect(((colorTimer/60)%10)*24,0,24,32);
        timerSprite[1].setSubRect((colorTimer/60/10)*24,0,24,32);

        if (colorTimer>10*60)
            timerSprite[1].setVisible(true);
        else
            timerSprite[1].setVisible(false);

        if (colorTimer>0)
            colorTimer--;
    }
    //replay timer
    if (settings->recording==PVS_REPLAYING && replayTimer>0)
    {
        replayTimer--;
        if (replayTimer==1)
        {//load new replay
            nextReplay();
        }
    }

    //Menus (display them over the main game)
    //Character select
    if (menuSelect==1)
        charSelectMenu->play();
    else if (menuSelect==2)
        mainMenu->play();

    //check if match can start
    if (currentGameStatus==GAMESTATUS_REMATCHING)
    {
        int rematching=0;
        int active=0;
        for(size_t i=0;i<players.size();i++)
        {
            if (players[i]->rematch)
                rematching++;
            if (players[i]->active)
                active++;
        }
        if (active==rematching && rematching>1 && active>0)
        {//start
            //reset rematch status
            for(size_t i=0;i<players.size();i++)
            {
                players[i]->rematch=false;
                players[i]->rematchIcon.setVisible(false);
            }
            if (!settings->spectating)
                currentGameStatus=GAMESTATUS_PLAYING;
            else
            {
                currentGameStatus=GAMESTATUS_SPECTATING;
                menuSelect=0;
            }
            //start game
            resetPlayers();
        }
        else if (rematching==0 && active==0)
        {
            currentGameStatus=GAMESTATUS_IDLE;
        }
    }

    //ready go
    if (players.size()>0)
    {
        readyGoObj.playAnimation();
    }
    backgroundAnimation.playAnimation();

    //replay
    if (settings->recording==PVS_REPLAYING)
    {
        int t=data->matchTimer;
        for(size_t j=0;j<players.size();j++)
        {
            //check event by looping through vector
            //if the time is -1, it's considered as processed
            for (size_t i=0;i<players[j]->recordMessages.size();i++)
            {
                if (players[j]->recordMessages[i].time==t)
                {//process all events with the correct time
                    //ignore color select
                    if (players[j]->recordMessages[i].message[0]=='s'
                        || players[j]->recordMessages[i].message[0]=='c')
                        continue;

                    //add message
                    players[j]->addMessage(players[j]->recordMessages[i].message);
                    players[j]->recordMessages[i].time=-1;
                }
                else if (players[j]->recordMessages[i].time > t)
                    break;
            }
        }
    }

    rankedMatch();

    //Main game
    for(int i=0;i<int(players.size());i++)
        players[i]->play();

    //check end of match
    checkEnd();

    //set status text
    for(size_t i=0;i<players.size();i++)
    {
        std::string str="";
        if (settings->recording==PVS_REPLAYING)
        {
            if (players[i]->onlineName=="")
                continue;

            //show playername and wins
            if (settings->showNames==0 || (settings->showNames==1 && i==0))
                str+=players[i]->onlineName+"\n";
            else
                str+="...\n";
            str+="Wins: "+to_string(players[i]->wins)+"\n";
            players[i]->setStatusText(str.c_str());
            continue;
        }

        //player is bound
        if (players[i]->onlineName!="")
        {
            str+=players[i]->onlineName+"\n";
            str+="Wins: "+to_string(players[i]->wins)+"\n";

            if (players[i]->getPlayerType()==ONLINE)
            {
                if (!players[i]->active)
                    str+="Waiting to join...\n";
                else
                {
                    if (players[i]->pickingCharacter && (currentGameStatus==GAMESTATUS_IDLE || currentGameStatus==GAMESTATUS_REMATCHING))
                        str+="Currently picking\n a character.";
                    else if (!players[i]->rematch && (currentGameStatus==GAMESTATUS_IDLE || currentGameStatus==GAMESTATUS_REMATCHING))
                        str+="Waiting for rematch.\n";
                }
            }
            players[i]->setStatusText(str.c_str());
        }
    }
    //force showing
//    if (Input.IsKeyDown(sf::Key::F1))
//    {
//        for(int i=0;i<players.size();i++)
//            players[i]->forceStatusText=true;
//    }
//    else
//    {
//        for(int i=0;i<players.size();i++)
//            players[i]->forceStatusText=false;
//    }

//    if (forceStatusText)
//    {
//        for(int i=0;i<players.size();i++)
//            players[i]->forceStatusText=true;
//    }
//    else
//    {
//        for(int i=0;i<players.size();i++)
//            players[i]->forceStatusText=false;
//    }

    changeMusicVolume();

    data->globalTimer++;
    data->matchTimer++;

    //access menu during endless mode
    if (settings->rulesetInfo.Nplayers==1 && menuSelect==0)
    {
        if (players.size()>0 && players[0]->losewin==NOWIN && players[0]->controls.Start>0)
        {
            players[0]->setLose();
        }
    }

    if (backwardsOnce)
        return;
    //replay normal
    if (settings->recording==PVS_REPLAYING && replayState==REPLAYSTATE_NORMAL)
        replayBackwardsTimer=data->matchTimer;
    //replay backwards
    if (settings->recording==PVS_REPLAYING && replayState==REPLAYSTATE_REWIND)
    {
        if (replayBackwardsTimer>0)
            replayBackwardsTimer-=60;

        if (!settings->oldReplayPlayList.empty())
        {
            bool soundSettings=settings->playSound;
            settings->playSound=false;
            data->playSounds=false;

            loadReplay(settings->oldReplayPlayList.back());
            //get ready to play
            resetPlayers();
            data->matchTimer=0;
            //loop until time is reached
            while (data->matchTimer < replayBackwardsTimer)
            {
                replayState=REPLAYSTATE_NORMAL;
                backwardsOnce=true;
                playGame();
                replayState=REPLAYSTATE_REWIND;
            }
            backwardsOnce=false;
            settings->playSound=soundSettings;
            data->playSounds=soundSettings;
            //end
            //if (replayBackwardsTimer<=0)
            {
                //replayBackwardsTimer=0;
                replayState=REPLAYSTATE_NORMAL;
            }
        }
        else
        {
            replayBackwardsTimer=data->matchTimer;
            replayState=REPLAYSTATE_NORMAL;
        }

    }
    //replay fastforward
    if (settings->recording==PVS_REPLAYING && replayState==REPLAYSTATE_FASTFORWARD && data->matchTimer%2==0)
    {
        playGame();
    }
    else if (settings->recording==PVS_REPLAYING && replayState==REPLAYSTATE_FASTFORWARDX4 && data->matchTimer%4==0)
    {
        playGame();
        playGame();
        playGame();
    }
}

void game::renderGame()
{
    //tunnel shader
    if(data->tunnelShader)
        data->tunnelShader->setParameter("time", data->globalTimer/60.0f);

    //Clear screen
    data->front->clear();
    //Draw background
    m_spriteBackground.draw(data->front);
    backgroundAnimation.draw();

    //draw timer
    if (currentGameStatus==GAMESTATUS_IDLE && !settings->useCPUplayers && (countActivePlayers()>1 || settings->rankedMatch))
    {
        timerSprite[0].draw(data->front);
        timerSprite[1].draw(data->front);
    }
    if (players[0]->getPlayerType()==HUMAN && players[0]->currentphase==PICKCOLORS
        && !settings->useCPUplayers && !players[0]->pickedColor)
    {
        timerSprite[0].draw(data->front);
        timerSprite[1].draw(data->front);
    }
    //Draw player related objects
    for(int i=0;i<int(players.size());i++)
    {//Draw fields
        players[i]->draw();
    }
    for(int i=0;i<int(players.size());i++)
    {//Draw light effect
        players[i]->drawEffect();
    }
    if (players.size()>0)
    {//Needs at least 1 player to drawy ready-go
        readyGoObj.draw();
    }

    //Draw menuSelects
    if (menuSelect==1)
        charSelectMenu->draw();
    if (menuSelect==2)
        mainMenu->draw();

    //darken screen
    if (rankedState >= 3)
    {
        black.draw(data->front);
        //draw status text
        if(statusText)
        {
            data->front->setColor(255, 255, 255, 255);
            statusText->draw(8, 0);
        }
    }

    data->front->swapBuffers();
}

void game::setStatusText(const char* utf8)
{
    if(utf8 == lastText) return;
    if(!statusFont) return;
    if(statusText) delete statusText;

    statusText = statusFont->render(utf8);
    lastText = utf8;
}

void game::setWindowFocus(bool focus)
{
    data->windowFocus=focus;
}
void game::Loop()
{//main loop of the game

    initGame(data->front);

    //Debug variables
    //fontArial.LoadFromFile("Data/arial.ttf");
    //sf::Text debug;
    //debug.SetFont(fontArial);
    //debug.SetCharacterSize(12);
    //debug.SetColor(sf::Color(255, 255, 255));
    //debug.Move(0.f,0.f);
    sprite debugSprite;
    debugSprite.setImage(data->imgNextPuyoBackgroundL);

    //Framelimiter variables
    double t = 0.0;
    double dt = 1.0/60.0;
    double accumulator = 0.0;
    double deltaTime;
    uint64_t startTime=timeGetTime();
    uint64_t endTime;
    //FPS counter
    double second = 0;
    int fps=0;
    int fpscounter=0;

    //Main loop
    while (runGame)
    {
        //Frame limiter
        endTime=timeGetTime();
        deltaTime=(endTime-startTime)/1000.0; //convert to second
        startTime=endTime;
        accumulator+=deltaTime;
        //fps counter (screen refresh rate)
        second+=deltaTime;
        if (second>1)
        {
            //show refresh rate
            //debugstring=to_string(int(fps/second))+"/"+to_string(fpscounter);
            fps=0;
            fpscounter=0;
            second=0;
        }

        //Main gameplay loop
        while (accumulator>=dt)
        {
            //Inner game loop
            playGame();

            //End of loop
            //Update framelimiter variables
            t += dt;
            accumulator -= dt;
            //fps
            fpscounter++;

            //Draw frame in the last loop
            if (accumulator<dt)
            {
                //screen refresh rate
                fps++;

                //innner loop render
                renderGame();

                //Debug messages
                //debug.SetString(debugstring);
                //Display
                data->front->swapBuffers();
            }
        }
    }
    //delete if using threads
    /*
    delete global;
    delete this;
    */
}

void game::addPlayer(playerType type,int playernum,int totalPlayer)
{
    players.push_back(new player(type,playernum,totalPlayer,this));
}

void game::resetPlayers()
{
    //set new nextlist seed
    if (settings->useCPUplayers)
        randomSeed_NextList=getRandom(10000);
    else
    {
        size_t lowest=players[0]->onlineID;
        int id=0;
        //find lowest ID
        for (size_t i=0;i<players.size();i++)
        {
            if (players[i]->active && players[i]->onlineID < lowest)
            {
                lowest=players[i]->onlineID;
                id=static_cast<int>(i);
            }
        }
        //set random seed to proposed random seed
        if (settings->recording!=PVS_REPLAYING)
            randomSeed_NextList=players[id]->proposedRandomSeed;
    }

    //reset players
    winsString="";
    for (size_t i=0;i<players.size();i++)
    {
        players[i]->reset();
        winsString+=to_string(players[i]->wins)+"-";
    }
    if (winsString.length()>0)
        winsString=winsString.substr(0,winsString.length()-1);
    activeAtStart=countActivePlayers();

    menuSelect=0;
    timerEndMatch=0;
    loadMusic();
}

void game::checkEnd()
{//check if match has ended.
    //Wait a little bit before actually goint into the menu, use timerEndMatch for that

    if (menuSelect==0)
    {//in match
        for (size_t i=0;i<players.size();i++)
        {//check if all players are in win or lose state
            if (players[i]->losewin==NOWIN && players[i]->active)
            {//still playing
                return;
            }
        }
        if (data->windowFocus)
        {
            data->front->musicEvent(music_can_stop);
        }
        targetVolumeNormal=25;
        targetVolumeFever=25;
        if (currentGameStatus==GAMESTATUS_PLAYING || settings->useCPUplayers)
        {
            timerEndMatch++;
            if (timerEndMatch==120)
            {//go to menu
                menuSelect=2;
                if (settings->rankedMatch) {
                    choiceTimer=5*60;
                } else {
                    choiceTimer=30*60;
                }
                colorTimer=10*60;
                rankedTimer=15*60;
                //online game: reset state if you were a player
                if (connected)
                    currentGameStatus=GAMESTATUS_IDLE;
                //release everyone's buttons (otherwise online players start choosing stuff)
                for (size_t i=0;i<players.size();i++)
                    players[i]->controls.release();

                //change channel description
                if (checkLowestID())
                    sendDescription();
            }
        }
        else if (settings->spectating)
        {
            //just wait until next match starts
            menuSelect=-1;
            if (connected)
                currentGameStatus=GAMESTATUS_REMATCHING;
            //play next replay
            if (settings->replayPlayList.size()>0)
            replayTimer=3*60;
        }
    }
}

int game::countActivePlayers()
{
    int n=0;
    for (size_t i=0;i<players.size();i++)
    {
        if (players[i]->active)
            n++;
    }
    return n;
}
int game::countBoundPlayers()
{
    int n=0;
    for (size_t i=0;i<players.size();i++)
    {
        if (players[i]->onlineName!="")
            n++;
    }
    return n;
}

bool game::checkLowestID()
{//check if active player has the lowest ID
    if (!connected)
        return false;

    //only player
    bool returnval=false;
    PVS_Channel *ch=network->channelManager.getChannel(channelName);
    if (ch!= nullptr && network->channelManager.getStatus(channelName,network->getPVS_Peer())==1)
    {
        //active
        if (currentGameStatus!=GAMESTATUS_WAITING && currentGameStatus!=GAMESTATUS_SPECTATING)
        {
            //lowest id?
            for (size_t i=1;i<players.size();i++)
            {
                if (players[i]->active && players[i]->onlineID > players[0]->onlineID)
                    returnval=false;
                    break;
            }
        }
    }

    return returnval;
}

void game::sendDescription()
{
    if (!connected)
        return;

    //player?
    PVS_Channel *ch=network->channelManager.getChannel(channelName);
    if (ch!= nullptr && network->channelManager.getStatus(channelName,network->getPVS_Peer())==1)
    {
        if (!settings->rankedMatch)
        {
            //send message about game
            std::string str="type:friendly|";//temp

            std::string ruleString="rules:Tsu";
            rulesetInfo_t *rs=&(settings->rulesetInfo);
            if (rs->rulesetType==TSU_ONLINE)
                ruleString="rules:Tsu";
            else if (rs->rulesetType==FEVER_ONLINE)
                ruleString="rules:Fever";
            else if (rs->rulesetType==FEVER15_ONLINE)
                ruleString="rules:Fever(15th)";
            else if (rs->rulesetType==ENDLESSFEVERVS_ONLINE)
                ruleString="rules:EndlessFeverVS";
            str+=ruleString;

            char other[1000];
            sprintf(other,"|marginTime:%i|targetPoint:%i|requiredChain:%i|initialFeverCount:%i|feverPower:%i|puyoToClear:%i|quickDrop:%i|Nplayers:%i|colors:%i"
                        ,rs->marginTime,rs->targetPoint,rs->requiredChain,rs->initialFeverCount,rs->feverPower,rs->puyoToClear,(int)rs->quickDrop,rs->Nplayers,rs->colors);
            str+=other;
            str+="|current:"+to_string(countBoundPlayers());
            if (rs->custom)
                str+="|custom:1";
            else
                str+="|custom:0";
            str+="|channelname:"+channelName;
            //add all peers
            std::string scr;
            for (size_t i=0;i<players.size();i++)
            {
                if (players[i]->onlineName!="")
                    scr+=players[i]->onlineName+" - "+to_string(players[i]->wins)+"\n";
            }
            str+="|currentscore:"+scr;
            network->requestChannelDescription(channelName,str);
        }
    }
}

std::string game::sendUpdate()
{
    //0[spectate]1[currentphase]2[fieldstringnormal]3[fevermode]4[fieldfever]5[fevercount]
    //6[rng seed]7[fever rng called]8[turns]9[colors]
    //10[margintimer]11[chain]12[currentFeverChainAmount]13[normal GQ]14[fever GQ]
    //15[predictedchain]16[allclear]
    std::string str="spectate|";
    player *pl=players[0];
    str+=to_string(static_cast<int>(pl->currentphase))+"|";
    //get normal field
    str+=pl->getNormalField()->getFieldString()+" |";
    str+=to_string(pl->feverMode)+"|";
    str+=pl->getFeverField()->getFieldString()+" |";
    str+=to_string(pl->feverGauge.getCount())+"|";
    str+=to_string(randomSeed_NextList)+"|";
    str+=to_string(pl->calledRandomFeverChain)+"|";
    str+=to_string(pl->turns)+"|";
    str+=to_string(pl->colors)+"|";
    str+=to_string(pl->margintimer)+"|";
    str+=to_string(pl->chain)+"|";
    str+=to_string(pl->currentFeverChainAmount)+"|";
    str+=to_string(pl->normalGarbage.GQ)+"|";
    str+=to_string(pl->predictedChain)+"|";
    str+=to_string(pl->allClear)+"|";

    return str;
}

void game::saveReplay()
{
    if (settings->recording!=PVS_RECORDING)
        return;

    if (activeAtStart==0)
        return;

    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    //open file
    std::ofstream outfile;
    std::string replayfolder="User/Replays/";
    //create userfolder
    if (!settings->spectating)
    {
        if (players.size()>2)
            replayfolder+=to_string(players.size())+"p/";
        else
            replayfolder+=to_string(players[1]->previousName)+"/";
    }
    else
    {
        replayfolder+="Spectator/";
    }
    createFolder(replayfolder);
    //create datefolder
    char dt[20];
    strftime(dt,20,"%Y-%m-%d",timeinfo);
    replayfolder+=std::string(dt)+"/";
    createFolder(replayfolder);

    //rulesname
    std::string rulesname="TSU_";
    if (settings->rulesetInfo.rulesetType==FEVER_ONLINE)
        rulesname="FEVER_";
    else if (settings->rulesetInfo.rulesetType==FEVER15_ONLINE)
        rulesname="FEVER15_";
    else if (settings->rulesetInfo.rulesetType==ENDLESSFEVERVS_ONLINE)
        rulesname="ENDLESSFEVER_";
    std::string playersname="_";
    if (players.size()>2)
        playersname=std::string("(")+to_string(activeAtStart)+"p)_";
    else if (players.size()==2)
    {
        playersname=to_string(players[0]->previousName)+"_vs_"+to_string(players[1]->previousName)+"_";
    }
    char ft[20];
    strftime(ft,20,"(%H%M%S)_",timeinfo);
    std::string scorename="";
    if (players.size()==2)
    {
        scorename=winsString;
    }
    //createFolder(replayfolder);
    std::string filename=rulesname+ft+playersname+scorename;
    outfile.open((replayfolder+filename+".rvs").c_str(),std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);

    //construct header
    replay_header rh=
    {
        {'R','P','V','S'},
        PVS_REPLAYVERSION,
        "",//date
        "",//time
        data->matchTimer,
        static_cast<char>(activeAtStart),
        randomSeed_NextList
    };
    strftime(rh.date,11,"%Y/%m/%d",timeinfo);
    strftime(rh.time,9,"%H:%M:%S",timeinfo);

    outfile.write( (char*) &rh,sizeof(replay_header));

    replay_ruleset_header rrh=
    {
        settings->rulesetInfo.rulesetType,
        settings->rulesetInfo.marginTime,
        settings->rulesetInfo.targetPoint,
        settings->rulesetInfo.requiredChain,
        settings->rulesetInfo.initialFeverCount,
        settings->rulesetInfo.feverPower,
        settings->rulesetInfo.puyoToClear,
        settings->rulesetInfo.Nplayers,
        settings->rulesetInfo.quickDrop,
        {0,0,0,0,0,0,0,0,0,0}
    };

    outfile.write( (char*) &rrh,sizeof(replay_ruleset_header));

    //go through all players
    for (size_t i=0;i<players.size();i++)
    {
        //player header
        int n=players[i]->getPlayerNum();

        replay_player_header rph=
        {
            "",//name
            static_cast<short>(players[i]->wins),
            static_cast<int>(players[i]->onlineID),
            static_cast<char>(n),
            players[i]->getPlayerType(),
            players[i]->getCharacter(),
            static_cast<char>(players[i]->activeAtStart),
            static_cast<char>(players[i]->colors),
            static_cast<int>(players[i]->controls.recordEvents.size()*sizeof(controllerEvent)),
            static_cast<int>(players[i]->recordMessages.size()*sizeof(messageEvent)),
            0,
            0
        };
        //copy name
        std::string name=players[i]->previousName.substr(0,31);
        strcpy(rph.name,name.c_str());

        //compress move vector
        int movLength=static_cast<int>(players[i]->controls.recordEvents.size()*sizeof(controllerEvent));
        unsigned long movComprLength=static_cast<unsigned long>(static_cast<float>(movLength)*1.1f+12);
        unsigned char *movcompressed = new unsigned char[movComprLength];
        if (movLength!=0)
        {
            int z_result = compress(
            movcompressed,
            &movComprLength,
            (unsigned char*) &players[i]->controls.recordEvents[0],
            movLength ) ;

            //check result
            if (z_result != Z_OK)
            {
                //stop
                outfile.close();
                remove((replayfolder+filename+".rvs").c_str());
                return;
            }
            //update size
            rph.vectorsizecomp_movement=int(movComprLength);

            encode("pvs2424",(char *) movcompressed,movComprLength);
        }
        else
            movComprLength=0;

        //compress messages
        int mesLength=static_cast<int>(players[i]->recordMessages.size()*sizeof(messageEvent));
        unsigned long mesComprLength=static_cast<unsigned long>(static_cast<float>(mesLength*1.1f)+12);
        unsigned char *mescompressed=new unsigned char[mesComprLength];
        if (mesLength!=0)
        {
            int z_result = compress(
            mescompressed,
            &mesComprLength,
            (unsigned char*) &players[i]->recordMessages[0],
            mesLength ) ;

            //check result
            if (z_result!=Z_OK)
            {
                //stop
                outfile.close();
                remove((replayfolder+filename+".rvs").c_str());
                return;
            }
            //update size
            rph.vectorsizecomp_message=int(mesComprLength);
            encode("pvs2424",(char*)mescompressed,mesComprLength);
        }
        else
            mesComprLength=0;

        //write compressed
        outfile.write((char*)&rph,sizeof(replay_player_header));
        if (movLength!=0)
            outfile.write((char*)movcompressed,movComprLength);
        if (mesLength!=0)
            outfile.write((char*)mescompressed,mesComprLength);

        //remove compressed
        delete [] movcompressed;
        players[i]->controls.recordEvents.clear();
        //remove compressed
        delete [] mescompressed;
        players[i]->recordMessages.clear();
    }
    outfile.close();
}

void game::loadReplay(std::string replayfile)
{
    settings->recording=PVS_REPLAYING;
    connected=false;
    network= nullptr;

    std::ifstream infile;
    infile.open(replayfile.c_str(),std::ios_base::in|std::ios_base::binary);
    if (!infile.is_open())
        return;

    //read header
    replay_header rh;
    infile.read((char*)&rh,sizeof(replay_header));

    //check version
    if (PVS_REPLAYVERSION < rh.versionNumber)
        return;
    else
        currentReplayVersion=rh.versionNumber;

    // backwards compatibility for randomizer: version 3 introduces the pp2 randomizer
    if (currentReplayVersion < 3) {
        legacyRandomizer =true;
        legacyNuisanceDrop =true;
    } else {
        legacyRandomizer= false;
        legacyNuisanceDrop = false;
    }

    //set
    randomSeed_NextList=rh.randomseed;

    //read rules header
    replay_ruleset_header rrh;
    infile.seekg(sizeof(replay_header),std::ios::beg);
    infile.read((char*)&rrh,sizeof(replay_ruleset_header));

    settings->rulesetInfo.quickDrop=rrh.quickDrop;
    settings->rulesetInfo.rulesetType=rrh.rulesetType;
    settings->rulesetInfo.marginTime=rrh.marginTime;
    settings->rulesetInfo.targetPoint=rrh.targetPoint;
    settings->rulesetInfo.requiredChain=rrh.requiredChain;
    settings->rulesetInfo.initialFeverCount=rrh.initialFeverCount;
    settings->rulesetInfo.feverPower=rrh.feverPower;
    settings->rulesetInfo.puyoToClear=rrh.puyoToClear;
    settings->rulesetInfo.Nplayers=rrh.Nplayers;
    settings->Nplayers=rrh.Nplayers;

    //set rules
    setRules();

    //set up players
    if ((int)players.size()!=rrh.Nplayers)
    {
        while(!players.empty())
        {
            delete players.back();
            players.pop_back();
        }
        initPlayers();
    }

    //by now the number of players should be correct
    if (rrh.Nplayers!=(int)players.size())
        return;

    unsigned int sizePrevious=sizeof(replay_header)+sizeof(replay_ruleset_header);
    for (size_t i=0;i<players.size();i++)
    {
        //read player header
        replay_player_header rph;
        infile.seekg(sizePrevious,std::ios::beg);
        infile.read((char*)&rph,sizeof(replay_player_header));

        //update player
        players[i]->bindPlayer(rph.name,rph.onlineID,true);
        players[i]->wins=rph.currentwins;
        players[i]->setPlayerType(rph.playertype);
        players[i]->setCharacter(rph.character);
        players[i]->active=rph.active;
        players[i]->colors=rph.colors;
        players[i]->controls.recordEvents.clear();
        players[i]->recordMessages.clear();

        //size of vectors
        unsigned long movSize=rph.vectorsize_movement;
        unsigned long mesSize=rph.vectorsize_message;
        unsigned long movSizeComp=rph.vectorsizecomp_movement;
        unsigned long mesSizeComp=rph.vectorsizecomp_message;

        //prepare vec
        if (movSize>0)
            players[i]->controls.recordEvents.resize(movSize/sizeof(controllerEvent));
        if (mesSize>0)
            players[i]->recordMessages.resize(mesSize/sizeof(messageEvent));

        //read compressed data
        unsigned char *movcompressed=new unsigned char[movSizeComp];
        unsigned char *mescompressed=new unsigned char[mesSizeComp];
        if (movSize>0)
        {
            infile.seekg(sizePrevious+sizeof(replay_player_header),std::ios::beg);
            infile.read((char*)movcompressed,movSizeComp);
            encode("pvs2424",(char*)movcompressed,movSizeComp);

        }
        if (mesSize>0)
        {
            infile.seekg(sizePrevious+sizeof(replay_player_header)+movSizeComp,std::ios::beg);
            infile.read((char*)mescompressed,mesSizeComp);
            encode("pvs2424",(char*)mescompressed,mesSizeComp);
        }

        //decompress into vectors
        if (movSize>0)
        {
            uncompress(
            (unsigned char*)&players[i]->controls.recordEvents[0],
            &movSize,
            movcompressed,   // source buffer - the compressed data
            movSizeComp );   // length of compressed data in bytes
        }
        if (mesSize>0)
        {
            uncompress(
            (unsigned char*)&players[i]->recordMessages[0],
            &mesSize,
            mescompressed,   // source buffer - the compressed data
            mesSizeComp );   // length of compressed data in bytes
        }
        //delete temporary buffer
        delete[] movcompressed;
        delete[] mescompressed;

        //update size
        sizePrevious+=sizeof(replay_player_header)+movSizeComp+mesSizeComp;
    }
    infile.close();

}

void game::nextReplay()
{
    if (settings->recording!=PVS_REPLAYING)
        return;

    if (!playNext)
    {
        settings->oldReplayPlayList.push_back(settings->replayPlayList.front());
        settings->replayPlayList.pop_front();
        playNext=true;
    }

    if (!settings->replayPlayList.empty())
    {
        loadReplay(settings->replayPlayList.front());
        settings->oldReplayPlayList.push_back(settings->replayPlayList.front());
        settings->replayPlayList.pop_front();
        //get ready to play
        resetPlayers();
    }
    else
        return;
    replayTimer=0;

}

void game::previousReplay()
{
    if (settings->recording!=PVS_REPLAYING)
        return;

    if (playNext)
    {
        settings->replayPlayList.push_front(settings->oldReplayPlayList.back());
        settings->oldReplayPlayList.pop_back();
        playNext=false;
    }

    if (!settings->oldReplayPlayList.empty())
    {
        settings->replayPlayList.push_front(settings->oldReplayPlayList.back());
        loadReplay(settings->replayPlayList.front());
        settings->oldReplayPlayList.pop_back();
        //get ready to play
        resetPlayers();
    }
    else
        return;
    replayTimer=0;

}

void game::rankedMatch()
{
    if ((currentGameStatus==GAMESTATUS_IDLE || currentGameStatus==GAMESTATUS_WAITING) && settings->rankedMatch)
    {
        if (rankedTimer>0)
            rankedTimer--;

        if (rankedTimer==0)
        {
            //the client is expected to have applied you for ranked match
            if (rankedState==0 && channelName.compare("")==0)
            {
                if (settings->rulesetInfo.rulesetType==TSU_ONLINE)
                    network->sendToServer(CHANNEL_MATCH,"find|0");
                else
                    network->sendToServer(CHANNEL_MATCH,"find|1");
                // upon matching again, the timer is reduced
                rankedTimer=5*60;
            }
            if (rankedState==2) //geting ready to close match
            {
                for (int i=0;i<int(players.size());i++)
                {
                    players[i]->unbindPlayer();
                }
                rankedState=3;
                rankedTimer=15*60;
            }
            else
                rankedTimer=15*60;
        }
        //wait for player input
        if (rankedState==3)
        {
            setStatusText(translatableStrings.rankedWaiting.c_str());
            if (players.size()>0 && players[0]->controls.Start==1)
            {
                if (!newRankedMatchMessage.empty())
                {
                    //apply for new match
                    network->sendToServer(CHANNEL_MATCH,newRankedMatchMessage.c_str());
                    newRankedMatchMessage="";
                }
                rankedState=0;
                rankedTimer=15*60;
            }
        }
    }

}

void game::changeMusicVolume()
{
    if (data->windowFocus)
    {
        if (currentVolumeNormal > targetVolumeNormal)
        {
            currentVolumeNormal -= 1;
            if (currentVolumeNormal<0)
                currentVolumeNormal=0;
            data->front->musicVolume(currentVolumeNormal/100.0f*globalVolume,false);
        }
        else if (currentVolumeNormal < targetVolumeNormal && data->globalTimer % 3==0)
        {
            currentVolumeNormal += 1;
            if (currentVolumeNormal>100)
                currentVolumeNormal=100;
            data->front->musicVolume(currentVolumeNormal/100.0f*globalVolume,false);
        }
        if (currentVolumeFever > targetVolumeFever)
        {
            currentVolumeFever -= 1;
            if (currentVolumeFever<0)
                currentVolumeFever=0;
            data->front->musicVolume(currentVolumeFever/100.0f*globalVolume,true);
        }
        else if (currentVolumeFever < targetVolumeFever)
        {
            currentVolumeFever += 1;
            if (currentVolumeFever>100)
                currentVolumeFever=100;
            data->front->musicVolume(currentVolumeFever/100.0f*globalVolume,true);
        }
    }
}

void game::loadMusic()
{
    if (!(settings->recording==PVS_REPLAYING && backwardsOnce==true))
    {
        data->front->musicEvent(music_continue);
        targetVolumeNormal=100;
        targetVolumeFever=100;
        //force to set volume
        currentVolumeNormal-=1;
        currentVolumeFever-=1;
    }
}

}
