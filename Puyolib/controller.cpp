#include "controller.h"
namespace ppvs
{
/*
0=press		up
1=release	up
2=press		down
3=release	down
4=press		left
5=release	left
6=press		right
7=release	right
8=press		A
9=release	A
10=press	B
11=release	B
*/

controller::controller()
{
    //Reset
    Down=0;
    Up=0;
    Left=0;
    Right=0;
    A=0;
    B=0;
    Start=0;

    DelayDown=false;
    DelayUp=false;
    DelayLeft=false;
    DelayRight=false;
    DelayA=false;
    DelayB=false;
    DelayStart=false;

    dUp=false;
    dDown=false;
    dLeft=false;
    dRight=false;
    dA=false;
    dB=false;
    dStart=false;

    m_playernumber=0;
    m_type=CPU;
    m_defined=false;

    state=PVS_NOTRECORDING;
}

controller::~controller()
{
    //dtor
}

void controller::init(int playernumber,playerType type,recordState s)
{
    m_playernumber=playernumber;
    m_type=type;
    state=s;
}

void controller::setstate(const finput& Input,int t)
{
    if (m_type==ONLINE) //ONLINE controls
        return;

    //set indicators with delay
    if (Up) DelayUp=true;
    else    DelayUp=false;
    if (Down) DelayDown=true;
    else    DelayDown=false;
    if (Left) DelayLeft=true;
    else    DelayLeft=false;
    if (Right) DelayRight=true;
    else    DelayRight=false;
    if (A) DelayA=true;
    else    DelayA=false;
    if (B) DelayB=true;
    else    DelayB=false;
    if (Start) DelayStart=true;
    else    DelayStart=false;

    if (state!=PVS_REPLAYING)
    {
        if (Input.a) A++; else A=0;
        if (Input.b) B++; else B=0;
        if (Input.up) Up++; else Up=0;
        if (Input.down) Down++; else Down=0;
        if (Input.left) Left++; else Left=0;
        if (Input.right) Right++; else Right=0;
        if (Input.start) Start++; else Start=0;

        //record state
        if(state==PVS_RECORDING && t>0)
            record(t);
    }
    else
    {
        //check event by looping through vector
        //if the time is -1, it's considered as processed
        for (size_t i=0; i<recordEvents.size(); i++)
        {
            if (recordEvents[i].time!=-1 && recordEvents[i].time==t)
            {
                //process all events with the correct time
                switch (recordEvents[i].ev)
                {
                case 0:
                    dUp=true;
                    break;
                case 1:
                    dUp=false;
                    break;
                case 2:
                    dDown=true;
                    break;
                case 3:
                    dDown=false;
                    break;
                case 4:
                    dLeft=true;
                    break;
                case 5:
                    dLeft=false;
                    break;
                case 6:
                    dRight=true;
                    break;
                case 7:
                    dRight=false;
                    break;
                case 8:
                    dA=true;
                    break;
                case 9:
                    dA=false;
                    break;
                case 10:
                    dB=true;
                    break;
                case 11:
                    dB=false;
                    break;
                }
                recordEvents[i].time=-1;
            }
            else if (recordEvents[i].time > t)
                break;
        }
        //set controls
        if (dUp) Up++;
        else	Up=0;
        if (dDown) Down++;
        else	Down=0;
        if (dLeft) Left++;
        else	Left=0;
        if (dRight) Right++;
        else	Right=0;
        if (dA) A++;
        else	A=0;
        if (dB) B++;
        else	B=0;
    }

}

void controller::release()
{
    //artificially release all buttons
    Down=0;
    Up=0;
    Left=0;
    Right=0;
    A=0;
    B=0;
    Start=0;

    DelayDown=false;
    DelayUp=false;
    DelayLeft=false;
    DelayRight=false;
    DelayA=false;
    DelayB=false;
    DelayStart=false;

    dUp=false;
    dDown=false;
    dLeft=false;
    dRight=false;
    dA=false;
    dB=false;
    dStart=false;
}

void controller::record(int t)
{
    if (Up>0 && dUp==false)
    {
        dUp=true;
        recordEvents.push_back(controllerEvent(t,0));
    }
    if (Up==0 && dUp==true)
    {
        dUp=false;
        recordEvents.push_back(controllerEvent(t,1));
    }
    if (Down>0 && dDown==false)
    {
        dDown=true;
        recordEvents.push_back(controllerEvent(t,2));
    }
    if (Down==0 && dDown==true)
    {
        dDown=false;
        recordEvents.push_back(controllerEvent(t,3));
    }
    if (Left>0 && dLeft==false)
    {
        dLeft=true;
        recordEvents.push_back(controllerEvent(t,4));
    }
    if (Left==0 && dLeft==true)
    {
        dLeft=false;
        recordEvents.push_back(controllerEvent(t,5));
    }
    if (Right>0 && dRight==false)
    {
        dRight=true;
        recordEvents.push_back(controllerEvent(t,6));
    }
    if (Right==0 && dRight==true)
    {
        dRight=false;
        recordEvents.push_back(controllerEvent(t,7));
    }
    if (A>0 && dA==false)
    {
        dA=true;
        recordEvents.push_back(controllerEvent(t,8));
    }
    if (A==0 && dA==true)
    {
        dA=false;
        recordEvents.push_back(controllerEvent(t,9));
    }
    if (B>0 && dB==false)
    {
        dB=true;
        recordEvents.push_back(controllerEvent(t,10));
    }
    if (B==0 && dB==true)
    {
        dB=false;
        recordEvents.push_back(controllerEvent(t,11));
    }

}


}
