#include <algorithm>
#include "characterSelect.h"
#include "game.h"
#include "../PVS_ENet/PVS_Client.h"
using namespace std;

namespace ppvs
{

characterSelect::characterSelect(game* g)
{
    //ctor
    firstStart=true;
    currentgame=g;
    data=g->data;

    background.setImage(NULL);
    background.setScale(2*640,480);
    background.setPosition(-640/2,-480/4);
    background.setColor(0,0,0);

    //character order
    order[0]=ARLE;
    order[1]=SCHEZO;
    order[2]=RULUE;
    order[3]=DRACO;
    order[4]=AMITIE;
    order[5]=RAFFINE;
    order[6]=SIG;
    order[7]=RIDER;

    order[8]=WITCH;
    order[9]=SATAN;
    order[10]=SUKETOUDARA;
    order[11]=CARBUNCLE;
    order[12]=ACCORD;
    order[13]=KLUG;
    order[14]=DONGURIGAERU;
    order[15]=OCEAN_PRINCE;

    order[16]=RINGO;
    order[17]=MAGURO;
    order[18]=RISUKUMA;
    order[19]=ECOLO;
    order[20]=FELI;
    order[21]=LEMRES;
    order[22]=OSHARE_BONES;
    order[23]=YU_REI;

    int width=8;int height=3;
    for (int i=0;i<height;i++)
    {
        for (int j=0;j<width;j++)
        {
            holder[i*width+j].setImage(g->data->imgCharHolder);
            holder[i*width+j].setCenter(0,0);
            holder[i*width+j].setPosition(64+j*66,64+i*52);
            charSprite[i*width+j].setImage(g->data->front->loadImage(folder_user_character+currentgame->settings->characterSetup[order[i*width+j]]+"/icon.png"));
            charSprite[i*width+j].setCenter(0,0);
            charSprite[i*width+j].setPosition(64+j*66+1,64+i*52+1);
        }
    }
    timer=0;

    selectSprite=0;
    selectedCharacter=0;
    dropset=0;
    name=0;
    nameHolder=0;
    nameHolderNumber=0;
    playernumber=0;
    sel=0;
    madeChoice=0;

    scale=1;
    Nplayers=0;
}

characterSelect::~characterSelect()
{
    //dtor
    delete []selectSprite;
    delete []selectedCharacter;
    delete []name;
    delete []nameHolder;
    delete []nameHolderNumber;
    delete []dropset;
    delete []playernumber;
    delete []sel;
    delete []madeChoice;
}

void characterSelect::draw()
{
    //set colors
    for (int i=0;i<24;i++)
        charSprite[i].setColor(128,128,128);

    for (int i=0;i<Nplayers;i++)
        charSprite[sel[i]].setColor(255,255,255);

    //draw the rest
    background.draw(data->front);
    for (int i=0;i<24;i++)
    {
        holder[i].draw(data->front);
        charSprite[i].draw(data->front);
    }
    for (int i=0;i<Nplayers;i++)
    {
        selectSprite[i].draw(data->front);
        if (Nplayers<=4) //only draw characters if players>4
            selectedCharacter[i].draw(data->front);
        nameHolder[i].draw(data->front);
        name[i].draw(data->front);
        for (int j=0;j<16;j++)
            dropset[i*16+j].draw(data->front);

        for (int j=0;j<3;j++)
        {
            playernumber[i*3+j].draw(data->front);
            nameHolderNumber[i*3+j].draw(data->front);
        }
    }

}

void characterSelect::play()
{
    int Nplayers=currentgame->players.size();

    if (timer!=0)
        timer++;

    if (timer<=60 && timer>0)
    {//fade in
        background.setTransparency(interpolate("linear",0.0,0.5,timer/60.0));
    }
    if (timer<=80 && timer>0)
    {//move in
        int width=8;int height=3;
        for (int i=0;i<height;i++)
        {
            for (int j=0;j<width;j++)
            {
                float tt=(timer)/20.0-(i*width+j)/12.0;
                float move=interpolate("exponential",1,0,tt,-2,1);
                if (move > 1)
                    move=1;
                else if (move < 0)
                    move=0;
                holder[i*width+j].setPosition(64+j*66+640*move,64+i*52);
                charSprite[i*width+j].setPosition(64+j*66+1+640*move,64+i*52+1);
            }
        }
    }
    if (timer<=160 && timer>0)
    {//move in
        for (int i=0;i<Nplayers;i++)
        {
            //normal display
            float posX=640.0/(Nplayers*2)*(i*2+1);
            float posY=480;
            //display only name
            if (Nplayers>4)
            {
                int width=ceil(sqrt(double(Nplayers)));
                posX=640.0/(width*2)*(i%width*2+1);
                posY=480-((i)/width)*128*scale;
            }
            float tt=(timer)/30.0-(i*1.0)/Nplayers;
            float move=interpolate("elastic",1,0,tt,-5,0.5);
            if (move>-0.001 && move<0.001) move=0;
            nameHolder[i].setPosition(posX,posY+2+320*move);
        }
    }
    if (timer==80)
    {//set visible
        for (int i=0;i<Nplayers;i++)
        {
            selectSprite[i].setVisible(true);
            if (i>8)
            {
                playernumber[i*3+0].setVisible(true);
                nameHolderNumber[i*3+0].setVisible(true);
            }
            playernumber[i*3+1].setVisible(true);
            playernumber[i*3+2].setVisible(true);
            name[i].setVisible(true);
            nameHolder[i].setVisible(true);
            nameHolderNumber[i*3+1].setVisible(true);
            nameHolderNumber[i*3+2].setVisible(true);
            selectedCharacter[i].setVisible(true);
            for (int j=0;j<16;j++)
                dropset[i*16+j].setVisible(true);
        }
    }

    //Count players that made a choice
    int allChoice=0;
    bool endnow=false; //override: end character selection immediately
    for (int i=0;i<Nplayers;i++)
    {
        if (madeChoice[i])
        {
            allChoice++;
        }
    }

    //do stuff
    if (timer>40)
    for (int i=0;i<Nplayers;i++)
    {
        int currentplayer=i;

        //ONLINE: don't care about CPU players
        if (currentgame->settings->useCPUplayers)
        {
            //move CPU with player 1
            if (currentgame->players[currentplayer]->getPlayerType()==CPU && i==allChoice)
                currentplayer=0;

            if (madeChoice[i])
            {//TEMP chance to cancel
                if (currentgame->players[currentplayer]->controls.B==1)
                {
                    int j=i;

                    //check if CPU should be canceled first
                    if (currentplayer==0)
                        for (int ii=0;ii<Nplayers;ii++)
                        {
                            if (currentgame->players[ii]->getPlayerType()==CPU && madeChoice[ii])
                                j=ii;
                        }
                    data->snd.cancel.Play(data);
                    currentgame->players[currentplayer]->controls.B++;
                    madeChoice[j]=false;
                    selectSprite[j].setVisible(true);
                    if (j>8)
                        playernumber[j*3+0].setVisible(true);
                    playernumber[j*3+1].setVisible(true);
                    playernumber[j*3+2].setVisible(true);

                }
                continue;
            }
        }

        bool moved=false;
        int selX=sel[i]%8;
        int selY=sel[i]/8;
        if (currentgame->players[currentplayer]->getPlayerType()!=ONLINE)
        {
            if (currentgame->players[currentplayer]->controls.Right==1 ||
                (currentgame->players[currentplayer]->controls.Right>16 && currentgame->players[currentplayer]->controls.Right%3==0))
            {
                //sel[i]++;
                selX++;
                moved=true;
            }
            if (currentgame->players[currentplayer]->controls.Left==1 ||
                (currentgame->players[currentplayer]->controls.Left>16 && currentgame->players[currentplayer]->controls.Left%3==0))
            {
                //sel[i]--;
                selX--;
                moved=true;
            }
            if (currentgame->players[currentplayer]->controls.Down==1 ||
                (currentgame->players[currentplayer]->controls.Down>16 && currentgame->players[currentplayer]->controls.Down%3==0))
            {
                //sel[i]+=8;
                selY++;
                moved=true;
            }
            if (currentgame->players[currentplayer]->controls.Up==1 ||
                (currentgame->players[currentplayer]->controls.Up>16 && currentgame->players[currentplayer]->controls.Up%3==0))
            {
                //sel[i]-=8;
                selY--;
                moved=true;
            }
        }
        if (sel[i]<0)
            sel[i]+=24;
        if (selX<0)
            selX+=8;
        if (selY<0)
            selY+=3;

        //sel[i]%=24;
        selX%=8;
        selY%=3;
        sel[i]=selY*8+selX;
        int jj=sel[i]%8;
        int ii=sel[i]/8;

        //normal display
        float posX=640.0/(Nplayers*2)*(i*2+1);
        float posY=480;
        //display only name
        if (Nplayers>4)
        {
            int width=ceil(sqrt(double(Nplayers)));
            posX=640.0/(width*2)*(i%width*2+1);
            posY=480-((i)/width)*128*scale;
        }

        //make choice
        if (currentgame->players[currentplayer]->controls.A==1 && timer>80)
        {
            data->snd.decide.Play(data);
            madeChoice[i]=true;
            currentgame->players[i]->setCharacter(order[sel[i]]);
            currentgame->players[i]->characterVoices.choose.Play(data);
            selectSprite[i].setVisible(false);
            playernumber[i*3+0].setVisible(false);
            playernumber[i*3+1].setVisible(false);
            playernumber[i*3+2].setVisible(false);

            //ONLINE: send message
            if (currentgame->connected)
            {
                currentgame->network->sendToChannel(CHANNEL_GAME,std::string("choice|")+to_string((int)order[sel[i]]),currentgame->channelName);
                endnow=true;
            }
        }

    //    float x=selectSprite[i].getX()+(64+j*66+1-selectSprite[i].getX())/3.0;
    //    float y=selectSprite[i].getY()+(64+i*52+1-selectSprite[i].getY())/3.0;
    //    selectSprite[i].setPosition(x,y);
        if (moved)
        {
            data->snd.cursor.Play(data);
            if (currentgame->settings->useCharacterField)
                currentgame->players[i]->setFieldImage(order[sel[i]]);

            selectSprite[i].setPosition(64+jj*66,64+ii*52);

            selectedCharacter[i].setImage(data->imgCharSelect[(int)order[sel[i]]]);
            selectedCharacter[i].setSubRect(0,0,256,256);
            selectedCharacter[i].setCenterBottom();
            selectedCharacter[i].setPosition(posX,posY-38*scale);
            selectedCharacter[i].setScale(scale);
            if (selectedCharacter[i].getImage()==NULL)
                selectedCharacter[i].setVisible(false);

            name[i].setImage(data->imgCharName[(int)order[sel[i]]]);
            //name[i].setSubRect(0,0,width,48);
            name[i].setCenterBottom();
            name[i].setPosition(posX,posY);
            name[i].setScale(scale);

            setDropset(posX,posY-48*scale,i);

            //ONLINE: send message
            if (currentgame->connected)
                currentgame->network->sendToChannel(CHANNEL_GAME,std::string("select|")+to_string((int)order[sel[i]]),currentgame->channelName);
        }
        //player number
        float xx=playernumber[i*3].getX()+(selectSprite[i].getX()-playernumber[i*3].getX())/3.0+2;
        float yy=playernumber[i*3].getY()+(selectSprite[i].getY()-playernumber[i*3].getY())/3.0+6;
        playernumber[i*3+0].setPosition(xx   ,yy+1*sin(data->globalTimer/30.0+i*10));
        playernumber[i*3+1].setPosition(xx+10,yy+1*sin(data->globalTimer/30.0+i*10));
        playernumber[i*3+2].setPosition(xx+20,yy+1*sin(data->globalTimer/30.0+i*10));
    }

    //all choices made
    if ((allChoice==Nplayers && timer>0) || endnow)
    {
        //fade out
        timer=-120;
    }
    if (timer<0 && timer>-60)
    {
        float t=(timer+60)/60.0; //from 0 to 1
        background.setTransparency(interpolate("linear",0.0,0.5,-timer/60.0));

        //set invisible
        for (int i=0;i<Nplayers;i++)
        {
            selectSprite[i].setVisible(false);
            selectedCharacter[i].setTransparency(1-t);
            name[i].setTransparency(1-t);
            playernumber[i*3+0].setTransparency(1-t);
            playernumber[i*3+1].setTransparency(1-t);
            playernumber[i*3+2].setTransparency(1-t);
            nameHolderNumber[i*3+0].setTransparency(1-t);
            nameHolderNumber[i*3+1].setTransparency(1-t);
            nameHolderNumber[i*3+2].setTransparency(1-t);
            for (int j=0;j<16;j++)
                dropset[i*16+j].setTransparency(1-t);
            nameHolder[i].setTransparency(1-t);
        }
        for (int i=0;i<24;i++)
        {
            holder[i].setTransparency(1-t);
            charSprite[i].setTransparency(1-t);
        }
    }
    if (timer<0 && timer>-80)
    {//move in
        int width=8;int height=3;
        for (int i=0;i<height;i++)
        {
            for (int j=0;j<width;j++)
            {
                float tt=(-timer)/20.0-(i*width+j)/12.0;
                float move=interpolate("exponential",1,0,tt,-2,1);
                if (move > 1)
                    move=1;
                else if (move < 0)
                    move=0;
                holder[i*width+j].setPosition(64+j*66+640*move,64+i*52);
                charSprite[i*width+j].setPosition(64+j*66+1+640*move,64+i*52+1);
            }
        }
    }
    if (timer<0 && timer>=-160)
    {//move in
        for (int i=0;i<Nplayers;i++)
        {
            //normal display
            float posX=640.0/(Nplayers*2)*(i*2+1);
            float posY=480;
            //display only name
            if (Nplayers>4)
            {
                int width=ceil(sqrt(double(Nplayers)));
                posX=640.0/(width*2)*(i%width*2+1);
                posY=480-((i)/width)*128*scale;
            }
            float tt=(-timer)/30.0-(i*1.0)/Nplayers;
            float move=interpolate("elastic",1,0,tt,-5,0.5);
            if (move>-0.001 && move<0.001) move=0;
            nameHolder[i].setPosition(posX,posY+2+320*move);
        }
    }
    if (timer==-1)
    {//set visible
        for (int i=0;i<Nplayers;i++)
        {
            selectSprite[i].setVisible(false);
            if (i>8)
            {
                playernumber[i*3+0].setVisible(false);
                nameHolderNumber[i*3+0].setVisible(false);
            }
            playernumber[i*3+1].setVisible(false);
            playernumber[i*3+2].setVisible(false);
            name[i].setVisible(false);
            nameHolderNumber[i*3+1].setVisible(false);
            nameHolderNumber[i*3+2].setVisible(false);
            selectedCharacter[i].setVisible(false);
            for (int j=0;j<16;j++)
                dropset[i*16+j].setVisible(false);
        }
        end();
    }

}

void characterSelect::end()
{//end of character select
    timer=0;
    int Nplayers=currentgame->players.size();

    if (!currentgame->settings->useCPUplayers)
    {
        //ONLINE: do nothing, just go back to menu
        currentgame->menuSelect=2;
    }
    else
    {
        if (currentgame->players[0]->losewin==NOWIN) //this state is only at start
        {//set all players to pick colors, or play game now
            if (currentgame->settings->pickColors)
            {
                for (int i=0;i<Nplayers;i++)
                    currentgame->players[i]->currentphase=PICKCOLORS;
            }
            else
            {//play game right now
                for (int i=0;i<Nplayers;i++)
                    currentgame->players[i]->currentphase=GETREADY;
                currentgame->readyGoObj.prepareAnimation("readygo");
                data->matchTimer=0;
            }
            currentgame->menuSelect=0;
            currentgame->loadMusic();
        }
        else
        {//return to menu
            currentgame->menuSelect=2;
        }
    }

}

void characterSelect::prepare()
{
    timer=1;
    delete []selectSprite;
    delete []selectedCharacter;
    delete []name;
    delete []nameHolder;
    delete []nameHolderNumber;
    delete []dropset;
    delete []playernumber;
    delete []sel;
    delete []madeChoice;

    Nplayers=currentgame->players.size();
    scale=min(2.0/Nplayers,1.0);
    selectSprite=new sprite[Nplayers];
    selectedCharacter=new sprite[Nplayers];
    name=new sprite[Nplayers];
    nameHolder=new sprite[Nplayers];
    nameHolderNumber=new sprite[Nplayers*3];
    dropset=new sprite[Nplayers*16];
    playernumber=new sprite[Nplayers*3];

    sel=new int[Nplayers];
    madeChoice=new bool[Nplayers];

    //objects for every player
    for (int i=0;i<Nplayers;i++)
    {
        //release buttons for cpu
        if(currentgame->players[i]->getPlayerType()==CPU)
            currentgame->players[i]->controls.release();

        if (firstStart)
            sel[i]=i%24;
        else
            sel[i]=findCurrentCharacter(i);
        int jj=sel[i]%8;
        int ii=sel[i]/8;
        madeChoice[i]=false;


        //set initial field image
        if (currentgame->settings->useCharacterField)
            currentgame->players[i]->setFieldImage(order[sel[i]]);

        //normal display
        float posX=640.0/(Nplayers*2)*(i*2+1);
        float posY=480;
        //display only name
        if (Nplayers>4)
        {
            int width=ceil(sqrt(double(Nplayers)));
            scale=2.0f/float(width);
            posX=640.0/(width*2)*(i%width*2+1);
            posY=480-((i)/width)*128*scale;
        }

        selectSprite[i].setImage(data->imgPlayerCharSelect);
        selectSprite[i].setCenter(1,1);
        selectSprite[i].setPosition(64+jj*66,64+ii*52);
        selectSprite[i].setVisible(false);

        selectedCharacter[i].setImage(data->imgSelect[(int)order[sel[i]]]);
        selectedCharacter[i].setCenterBottom();
        selectedCharacter[i].setScale(scale);
        selectedCharacter[i].setPosition(posX,posY-38*scale);
        selectedCharacter[i].setVisible(false);
        if (selectedCharacter[i].getImage()==NULL)
            selectedCharacter[i].setVisible(false);

        for (int j=0;j<16;j++)
        {
            dropset[i*16+j].setImage(data->imgDropset);
            dropset[i*16+j].setVisible(false);
        }

        for (int j=0;j<3;j++)
        {
            playernumber[i*3+j].setImage(data->imgPlayerNumber);
            nameHolderNumber[i*3+j].setImage(data->imgPlayerNumber);
        }
        //number
        playernumber[i*3+0].setSubRect((i+1)/10*24,0,24,32);
        playernumber[i*3+0].setCenterBottom();
        playernumber[i*3+0].setScale(0.5);
        playernumber[i*3+1].setSubRect((i+1)%10*24,0,24,32);
        playernumber[i*3+1].setCenterBottom();
        playernumber[i*3+1].setScale(0.5);
        playernumber[i*3+2].setSubRect(240,0,24,32);
        playernumber[i*3+2].setCenterBottom();
        playernumber[i*3+2].setScale(0.5);
        nameHolderNumber[i*3+0].setSubRect((i+1)/10*24,0,24,32);
        nameHolderNumber[i*3+0].setCenterBottom();
        nameHolderNumber[i*3+0].setScale(scale);
        nameHolderNumber[i*3+1].setSubRect((i+1)%10*24,0,24,32);
        nameHolderNumber[i*3+1].setCenterBottom();
        nameHolderNumber[i*3+1].setScale(scale);
        nameHolderNumber[i*3+2].setSubRect(240,0,24,32);
        nameHolderNumber[i*3+2].setCenterBottom();
        nameHolderNumber[i*3+2].setScale(scale);
        if (i<9)
            nameHolderNumber[i*3+0].setVisible(false);
        playernumber[i*3+0].setPosition(64+jj*66+4,64+ii*52+6);
        playernumber[i*3+1].setPosition(64+jj*66+4,64+ii*52+6);
        playernumber[i*3+2].setPosition(64+jj*66+4,64+ii*52+6);
        playernumber[i*3+0].setVisible(false);
        playernumber[i*3+1].setVisible(false);
        playernumber[i*3+2].setVisible(false);
        nameHolderNumber[i*3+0].setPosition(posX+(00)*scale,posY-70*scale);
        nameHolderNumber[i*3+1].setPosition(posX+(20)*scale,posY-70*scale);
        nameHolderNumber[i*3+2].setPosition(posX+(40)*scale,posY-70*scale);
        nameHolderNumber[i*3+0].setVisible(false);
        nameHolderNumber[i*3+1].setVisible(false);
        nameHolderNumber[i*3+2].setVisible(false);
        name[i].setImage(data->imgCharName[(int)order[sel[i]]]);
        name[i].setCenterBottom();
        name[i].setPosition(posX,posY);
        name[i].setScale(scale);
        name[i].setVisible(false);
        nameHolder[i].setImage(data->imgNameHolder);
        nameHolder[i].setCenterBottom();
        nameHolder[i].setPosition(posX,posY);
        nameHolder[i].setScale(scale);
        //nameHolder[i].setVisible(false);
        setDropset(posX,posY-48*scale,i);
        //set visible
        for (int i=0;i<Nplayers;i++)
        {
            selectedCharacter[i].setTransparency(1);
            name[i].setTransparency(1);
            playernumber[i*3+0].setTransparency(1);
            playernumber[i*3+1].setTransparency(1);
            playernumber[i*3+2].setTransparency(1);
            nameHolderNumber[i*3+0].setTransparency(1);
            nameHolderNumber[i*3+1].setTransparency(1);
            nameHolderNumber[i*3+2].setTransparency(1);
            for (int j=0;j<16;j++)
                dropset[i*16+j].setTransparency(1);
            nameHolder[i].setTransparency(1);
        }
        for (int i=0;i<24;i++)
        {
            holder[i].setTransparency(1);
            charSprite[i].setTransparency(1);
        }


    }
    firstStart=false;

}

void characterSelect::setDropset(int x,int y,int pl)
{
    puyoCharacter pc=order[sel[pl]];
    //get total width and center dropset
    //float xx = 0;
    //float length=0;
    //for (int j=0;j<16;j++)
    //{
    //    movePuyoType mpt=getFromDropPattern(pc,j);
    //    if (mpt==DOUBLET)
    //        length+=10;
    //    else
    //        length+=18;
    //}
    //xx=-length/2.0-5;
    float xx = -128;

    for (int j=0;j<16;j++)
    {
        movePuyoType mpt=getFromDropPattern(pc,j);
        dropset[pl*16+j].setPosition(x+xx*scale,y);
        dropset[pl*16+j].setScale(scale);
        switch(mpt)
        {
            case DOUBLET:
                dropset[pl*16+j].setSubRect(0,0,16,24);
                dropset[pl*16+j].setCenter(0,24);
                xx+=10;
                break;
            case TRIPLET:
                dropset[pl*16+j].setSubRect(16,0,24,24);
                dropset[pl*16+j].setCenter(0,24);
                xx+=18;
                break;
            case TRIPLETR:
                dropset[pl*16+j].setSubRect(40,0,24,24);
                dropset[pl*16+j].setCenter(0,24);
                xx+=18;
                break;
            case QUADRUPLET:
                dropset[pl*16+j].setSubRect(64,0,24,24);
                dropset[pl*16+j].setCenter(0,24);
                xx+=18;
                break;
            case BIG:
                dropset[pl*16+j].setSubRect(88,0,24,24);
                dropset[pl*16+j].setCenter(0,24);
                xx+=18;
                break;
        }
    }

}

int characterSelect::findCurrentCharacter(int i)
{
    puyoCharacter pc = currentgame->players[i]->getCharacter();
    //find selection
    for (int j=0;j<24;j++)
    {
        if (order[j]==pc)
            return j;
    }
    return -1;
}

void characterSelect::setCharacter(int playernum,int selection,bool choice)
{//sets animation for setting a character, does not actually set the player character if charactersleect is not active!
    if (playernum >= (int)currentgame->players.size())
        return;

    if (timer>40)
    {
        int i=playernum;
        //find selection from character order
        int s=0;
        for (int j=0;j<24;j++)
        {
            if (selection==static_cast<int>(order[j]))
            {s=j;break;}
        }
        sel[i]=s;
        int jj=sel[i]%8;
        int ii=sel[i]/8;
        //normal display
        float posX=640.0/(Nplayers*2)*(i*2+1);
        float posY=480;
        //display only name
        if (Nplayers>4)
        {
            int width=ceil(sqrt(double(Nplayers)));
            posX=640.0/(width*2)*(i%width*2+1);
            posY=480-((i)/width)*128*scale;
        }

        //move
        if (!choice)
        {
            data->snd.cursor.Play(data);
            madeChoice[i]=false;
            if (currentgame->settings->useCharacterField)
                currentgame->players[i]->setFieldImage(order[sel[i]]);

            selectSprite[i].setPosition(64+jj*66,64+ii*52);

            selectedCharacter[i].setImage(data->front->loadImage(folder_user_character+currentgame->settings->characterSetup[order[sel[i]]]+"/select.png"));
            selectedCharacter[i].setSubRect(0,0,256,256);
            selectedCharacter[i].setCenterBottom();
            selectedCharacter[i].setPosition(posX,posY-38*scale);
            selectedCharacter[i].setScale(scale);
            if (selectedCharacter[i].getImage()==NULL)
                selectedCharacter[i].setVisible(false);

            name[i].setImage(data->imgCharName[(int)order[sel[i]]]);
            name[i].setCenterBottom();
            name[i].setPosition(posX,posY);
            name[i].setScale(scale);

            setDropset(posX,posY-48*scale,i);

        }
        else
        {//made choice
            data->snd.decide.Play(data);
            madeChoice[i]=true;
            currentgame->players[i]->setCharacter(order[sel[i]]);
            currentgame->players[i]->characterVoices.choose.Play(data);
            selectSprite[i].setVisible(false);
            playernumber[i*3+0].setVisible(false);
            playernumber[i*3+1].setVisible(false);
            playernumber[i*3+2].setVisible(false);
        }

    }
}

}
