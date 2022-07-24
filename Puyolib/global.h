#pragma once
#include "RNG/MersenneTwister.h"
#include "Sound.h"
#include "Sprite.h"
#include "Frontend.h"

#include <map>
#include <vector>
#include <deque>
#include <sstream>
#include <stdint.h>

#define CHANNEL_CHAT                0
#define CHANNEL_CHALLENGE           1
#define CHANNEL_CHALLENGERESPONSE   2
#define CHANNEL_GAME                3
#define CHANNEL_CHAT_PRIVATE        4

#define CHANNEL_MATCH               9

#ifdef _WIN32
#ifdef _MSC_VER
extern "C" __declspec(dllimport)
unsigned long __stdcall timeGetTime(void);
#endif
#else
#include <sys/time.h>
inline static unsigned long timeGetTime()
{
	timeval tv;
	gettimeofday(&tv, 0);

	return (tv.tv_usec / 1000) + tv.tv_sec * 1000;
}
#endif

namespace ppvs
{

typedef char PlayerType;
#define HUMAN   0
#define CPU     1
#define ONLINE  2

typedef char PuyoCharacter;
#define ACCORD          0
#define AMITIE          1
#define ARLE            2
#define DONGURIGAERU    3
#define DRACO           4
#define CARBUNCLE       5
#define ECOLO           6
#define FELI            7
#define KLUG            8
#define LEMRES          9
#define MAGURO          10
#define OCEAN_PRINCE    11
#define OSHARE_BONES    12
#define RAFFINE         13
#define RIDER           14
#define RISUKUMA        15
#define RINGO           16
#define RULUE           17
#define SATAN           18
#define SCHEZO          19
#define SIG             20
#define SUKETOUDARA     21
#define WITCH           22
#define YU_REI          23
#define NUM_CHARACTERS  24

typedef int RecordState;
#define PVS_NOTRECORDING 0
#define PVS_RECORDING    1
#define PVS_REPLAYING    2

// Global sounds struct
struct Sounds
{
	Sound allcleardrop, drop, feverlight, fever, fevertimecount, fevertimeend, go, heavy, hit, lose, move,
		nuisance_hitL, nuisance_hitM, nuisance_hitS,
		nuisanceL, nuisanceS, ready, rotate, win,
		decide, cancel, cursor;
	Sound chain[7];
};

// Settings by the users for modding, are loaded at start of game
struct UserSettings
{
	// Strings to userfolders
	std::string str_puyo;
	std::string str_background;
	std::string str_sfx;
	std::string str_voice;
	std::string str_replay;
};

// Variables through that are global in game
struct GameData
{
	// Controllers
	Frontend* front;
	FeImage* imgPuyo, * imgBackground, * imgField1, * imgField2, * imgBorder1, * imgBorder2, * imgPlayerBorder, * imgSpice;
	FeImage* imgFieldFever, * imgScore, * imgChain, * imgCheckmark;
	FeImage* imgAllClear, * imgLose, * imgWin;
	FeImage* imgNextPuyoBackgroundR, * imgNextPuyoBackgroundL;
	FeImage* imgFeverGauge, * imgSeconds;
	FeImage* imgLight, * imgLight_s, * imgLight_hit, * imgfsparkle, * imgfLight, * imgfLight_hit, * imgTime;
	FeImage* imgMenu[3][2];
	FeImage* imgFeverBack[30];
	FeImage* imgCharHolder, * imgNameHolder, * imgPlayerNumber, * imgPlayerCharSelect, * imgBlack;
	FeImage* imgCharField[NUM_CHARACTERS], * imgCharSelect[NUM_CHARACTERS], * imgSelect[NUM_CHARACTERS], * imgCharName[NUM_CHARACTERS];
	FeImage* imgDropset;
	FeShader* glowShader;
	FeShader* tunnelShader;
	Sounds snd;
	UserSettings gUserSettings;
	int globalTimer; // Global timer in game
	int matchTimer; // Duration of a match
	int PUYOXCENTER; // Center of rotation for quadruplets
	bool windowFocus;
	bool playSounds, playMusic;

};

struct TranslatableStrings
{
	std::string waitingForPlayer;
	std::string disconnected;
	std::string rankedWaiting;
	std::string rankedSearching;
	TranslatableStrings()
		: waitingForPlayer("Waiting for player...")
		, disconnected("Player disconnected.")
		, rankedWaiting("Please press the Start button\n to find a new opponent. (default: Enter button)")
		, rankedSearching("Searching for opponent...")
	{}
};

typedef std::deque<std::string> stringList;

// Ini file is edited from launcher
extern std::map<std::string, int> controls_map;

// Sound
extern std::vector<std::string> voicePattern;
extern std::vector<int> voicePatternClassic;

// Global constants throughout all games
extern const float PI;
extern const int PUYOX; // Size of puyo in image
extern const int PUYOY;
extern const int CHAINPOPSPEED;
extern const float GARBAGESPEED;

// Global strings for local folders (to shorten expressions)
extern const std::string folder_user_sounds;
extern const std::string folder_user_music;
extern const std::string folder_user_backgrounds;
extern const std::string folder_user_puyo;
extern const std::string folder_user_character;

// Global randomizer
extern MersenneTwister grandomizer;

// Shader
extern bool useShaders;
extern float tunnelShaderColor[6][3];

// Global functions
void initGlobal();
void initFeverChains();
std::string getFeverChain(int type, int colors, int chain, int offset = 0);
int getRandom(int);
std::string Lower(std::string str);
void setBuffer(Sound& s, FeSound* sb);
int sign(int i);
double interpolate(std::string type, double startVal, double endVal, double t, double alpha = 1, double beta = 1);
void splitString(std::string& in, char delimiter, stringList& v);
void createFolder(std::string foldername);

// Debugging
extern FeFont* fontArial;
extern std::string debugstring;
extern int debugMode;

// Convert int to string
template <class T>
std::string to_string(const T& t)
{
	std::stringstream ss;
	ss << t;
	return ss.str();
}

int to_int(const std::string& s);

}
