#include "global.h"
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>

namespace ppvs
{
//Constants
const float PI=3.14159265;
const int PUYOX=32;
const int PUYOY=32;
const int CHAINPOPSPEED=25;
const float GARBAGESPEED=4.8;
//Strings
const std::string folder_user_sounds="User/Sounds/";
const std::string folder_user_music="User/Music/";
const std::string folder_user_backgrounds="User/Backgrounds/";
const std::string folder_user_puyo="User/Puyo/";
const std::string folder_user_character="User/Characters/";

//Sound
std::vector<std::string> voicePattern;
std::vector<int> voicePatternClassic;
std::vector<std::string> musicListFever;
std::vector<std::string> musicListNormal;

//shader
bool useShaders=false;
//tunnel shader colors
float tunnelShaderColor[6][3]= {
    {0.0f, 0.0f, 1.0f},
    {0.0f, 0.7f, 0.0f},
    {0.8f, 0.1f, 0.1f},
    {0.8f, 0.0f, 0.8f},
    {0.8f, 0.8f, 0.0f},
    {1.0f, 1.0f, 1.0f}
};

//global randomizer
MersenneTwister grandomizer;
//Load ini file
usersettings gUserSettings;

//global functions
//================
int getRandom(int in)
{//returns random number with random seed
    unsigned long out;
    out=in*grandomizer.genrand_real1();
    return int(out);
}

void setBuffer(sound &s,fsound *sb)
{
    if (sb)
        s.SetBuffer(sb);
    else
        s.Unload();
}

std::string Lower(std::string strToConvert)
{//change each element of the string to lower case
   for(unsigned int i=0;i<strToConvert.length();i++)
   {
      strToConvert[i] = tolower(strToConvert[i]);
   }
   return strToConvert;//return the converted string
}

int sign(int x)
{//returns +1 or -1, +1 for 0
    return (x > 0) ? 1 : ((x < 0) ? -1 : 1);
}

int to_int(const std::string &s)
{
    /*int out;
    std::stringstream ss(s);
    ss >> out;
    return out;
    */
    int i=0;
    sscanf(s.c_str(),"%i",&i);
    return i;
}

double interpolate(std::string type,double s,double e,double t,double alpha,double beta)
{//input t must go from 0 to 1
    double out=0;
    if(type=="linear")
        out=(e-s)*t+s;
    else if(type=="quadratic")
        out=(e-s)*t*t+s;
    else if(type=="squareroot")
        out=(e-s)*pow(t,0.5)+s;
    else if(type=="cubic")
        out=(e-s)*t*t*t+s;
    else if(type=="cuberoot")
        out=(e-s)*pow(t,1.0/3.0)+s;
    else if(type=="exponential")
        out=(e-s)/(exp(alpha)-1)*exp(alpha*t)+s-(e-s)/(exp(alpha)-1);
    else if(type=="elastic") //beta=wavenumber
        out=(e-s)/(exp(alpha)-1)*cos(beta*t*2*PI)*exp(alpha*t)+s-(e-s)/(exp(alpha)-1);
    else if(type=="sin")
        out=s+e*sin(alpha*t*2*PI); //s=offset, e=amplitude, alpha=wavenumber
    else if(type=="cos")
        out=s+e*cos(alpha*t*2*PI); //s=offset, e=amplitude, alpha=wavenumber
    return out;
}

void splitString(std::string &in,char delimiter,stringList &v)
{
    std::stringstream stream(in);
    std::string token;
    while(getline(stream, token, delimiter))
    {
        v.push_back(token);
    }
}

void createFolder(std::string foldername)
{
    #ifdef _WIN32
        CreateDirectoryA(foldername.c_str(), NULL);
    #else
        mkdir(foldername.c_str(),0777);
    #endif
}

void initGlobal()
{//this only needs to be called once
    static bool init=false;
    if (init)
        return;

    //randomizer
    grandomizer.init_genrand(timeGetTime());

    //fever chains
    initFeverChains();

    //init voice pattern
    voicePattern.push_back("c1");
    voicePattern.push_back("c1e1");
    voicePattern.push_back("c1c2e1");
    voicePattern.push_back("c1c2c3e2");
    voicePattern.push_back("c1c2c3c4e2");
    voicePattern.push_back("c1c2c3c4c5e2");
    voicePattern.push_back("c1c2c3c4c5c5e3");
    voicePattern.push_back("c1c2c3c4c5c5c5e3");
    voicePattern.push_back("c1c2c3c4e1c5c5c5e3");
    voicePattern.push_back("c1c2c3c4e1c4c5c5c5e4");
    voicePattern.push_back("c1c2c3c4e1c3c4c5c5c5e4");
    voicePattern.push_back("c1c2c3c5e1c3c4c5e2c5c5e4");
    voicePattern.push_back("c1c2c3c5e1c3c4c5e2c5c5c5e5");
    voicePattern.push_back("c1c2c3c5e1c2c3c4c5e2c4c5c5e5");
    voicePattern.push_back("c1c2c3c5e1c2c3c4c5e2c3c4c5c5e5");
    voicePattern.push_back("c1c2c3c4c5e1c2c4c5e2c2c3c4c5c5e5");
    voicePattern.push_back("c1c2c3c4c5e1c2c4c5e2c2c3c4c5c5c5e5");
    voicePattern.push_back("c1c2c3c4c5e1c2c3c4c5e2c2c3c4c3c4c5e5");
    voicePattern.push_back("c1c2c3c4c5e1c2c3c4c5e2c2c3c4c3c4c5c5e5");
    voicePattern.push_back("c1c2c3c4c5e1c2c3c4c5e2c2c3c4c3c4c5c5c5e5");

    voicePattern.push_back("c1c2c3c4c5c5e1c2c3c4c5e2c2c3c4c3c4c5c5c5e4");
    voicePattern.push_back("c1c2c3c4c5c5e1c2c3c4c5c5e2c2c3c4c3c4c5c5c5e4");
    voicePattern.push_back("c1c2c3c4c5c5e1c2c3c4c5c5c5e2c2c3c4c3c4c5c5c5e4");
    voicePattern.push_back("c1c2c3c4c3c4c5e1c2c3c4c3c4c5e2c2c3c4c3c4c5c5c5e4");
    voicePattern.push_back("c1c2c3c4c3c4c5e1c2c3c4c3c4c5c5e2c2c3c4c3c4c5c5c5e4");
    voicePattern.push_back("c1c2c3c4c3c4c5c5e1c2c3c4c3c4c5c5e2c2c3c4c3c4c5c5c5e5");
    voicePattern.push_back("c1c2c3c4c3c4c5c5e1c2c3c4c3c4c5c5c5e2c2c3c4c3c4c5c5c5e5");
    voicePattern.push_back("c1c2c3c4c3c4c5c5c5e1c2c3c4c3c4c5c5c5e2c2c3c4c3c4c5c5c5e5");
    voicePattern.push_back("c1c2c3c4c3c4c5c5c5e1c2c3c4c3c4c5c5c5e2c2c3c4c3c4c3c4c5c5e5");
    voicePattern.push_back("c1c2c3c4c3c4c5c5c5e1c2c3c4c3c4c5c5c5e2c2c3c4c3c4c3c4c5c5c5e5");

    //classic voice pattern
    voicePatternClassic.push_back(0);
    voicePatternClassic.push_back(5);
    voicePatternClassic.push_back(6);
    voicePatternClassic.push_back(4);
    voicePatternClassic.push_back(7);
    voicePatternClassic.push_back(8);
    voicePatternClassic.push_back(9);


    init=true;
}

//==================

//debug
//sf::Font fontArial;
std::string debugstring="";
int debugMode=0;
}
