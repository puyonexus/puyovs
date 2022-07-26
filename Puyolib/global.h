#pragma once
#include "Frontend.h"
#include "RNG/MersenneTwister.h"
#include "Sound.h"

#include <deque>
#include <map>
#include <sstream>
#include <vector>

#define CHANNEL_CHAT 0
#define CHANNEL_CHALLENGE 1
#define CHANNEL_CHALLENGERESPONSE 2
#define CHANNEL_GAME 3
#define CHANNEL_CHAT_PRIVATE 4

#define CHANNEL_MATCH 9

#ifdef _WIN32
#ifdef _MSC_VER
extern "C" __declspec(dllimport) unsigned long __stdcall timeGetTime(void);
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

namespace ppvs {

enum PlayerType : unsigned char {
	HUMAN,
	CPU,
	ONLINE,
};

enum PuyoCharacter : unsigned char {
	ACCORD,
	AMITIE,
	ARLE,
	DONGURIGAERU,
	DRACO,
	CARBUNCLE,
	ECOLO,
	FELI,
	KLUG,
	LEMRES,
	MAGURO,
	OCEAN_PRINCE,
	OSHARE_BONES,
	RAFFINE,
	RIDER,
	RISUKUMA,
	RINGO,
	RULUE,
	SATAN,
	SCHEZO,
	SIG,
	SUKETOUDARA,
	WITCH,
	YU_REI,
};

constexpr int kNumCharacters = 24;

enum class RecordState : int {
	NOT_RECORDING,
	RECORDING,
	REPLAYING,
};

// Global sounds struct
struct Sounds {
	Sound allClearDrop, drop, feverLight, fever, feverTimeCount, feverTimeEnd, go, heavy, hit, lose, move,
		nuisanceHitL, nuisanceHitM, nuisanceHitS,
		nuisanceL, nuisanceS, ready, rotate, win,
		decide, cancel, cursor;
	Sound chain[7];
};

// Settings by the users for modding, are loaded at start of game
struct UserSettings {
	// Strings to user directories
	std::string puyoDirPath;
	std::string backgroundDirPath;
	std::string sfxDirPath;
	std::string voiceDirPath;
	std::string replayDirPath;
};

// Variables through that are global in game
struct GameData {
	// Controllers
	Frontend* front {};
	FeImage *imgPuyo {}, *imgBackground {}, *imgField1 {}, *imgField2 {}, *imgBorder1 {}, *imgBorder2 {}, *imgPlayerBorder {}, *imgSpice {};
	FeImage *imgFieldFever {}, *imgScore {}, *imgChain {}, *imgCheckMark {};
	FeImage *imgAllClear {}, *imgLose {}, *imgWin {};
	FeImage *imgNextPuyoBackgroundR {}, *imgNextPuyoBackgroundL {};
	FeImage *imgFeverGauge {}, *imgSeconds {};
	FeImage *imgLight {}, *imgLightS {}, *imgLightHit {}, *imgFSparkle {}, *imgFLight {}, *imgFLightHit {}, *imgTime {};
	FeImage* imgMenu[3][2] {};
	FeImage* imgFeverBack[30] {};
	FeImage *imgCharHolder {}, *imgNameHolder {}, *imgPlayerNumber {}, *imgPlayerCharSelect {}, *imgBlack {};
	FeImage *imgCharField[kNumCharacters] {}, *imgCharSelect[kNumCharacters] {}, *imgSelect[kNumCharacters] {}, *imgCharName[kNumCharacters] {};
	FeImage* imgDropSet {};
	FeShader* glowShader {};
	FeShader* tunnelShader {};
	Sounds snd;
	UserSettings gUserSettings;
	int globalTimer {}; // Global timer in game
	int matchTimer {}; // Duration of a match
	int quadrupletCenter {}; // Center of rotation for quadruplets
	bool windowFocus {};
	bool playSounds {}, playMusic {};
};

struct TranslatableStrings {
	std::string waitingForPlayer;
	std::string disconnected;
	std::string rankedWaiting;
	std::string rankedSearching;
	TranslatableStrings()
		: waitingForPlayer("Waiting for player...")
		, disconnected("Player disconnected.")
		, rankedWaiting("Please press the Start button\n to find a new opponent. (default: Enter button)")
		, rankedSearching("Searching for opponent...")
	{
	}
};

typedef std::deque<std::string> StringList;

// Ini file is edited from launcher
extern std::map<std::string, int> controlsMap;

// Sound
extern std::vector<std::string> voicePattern;
extern std::vector<int> voicePatternClassic;

// Global constants throughout all games
constexpr float kPiF = 3.14159265f;
constexpr double kPiD = 3.141592653589793238463;
constexpr int kPuyoX = 32;
constexpr int kPuyoY = 32;
constexpr int kChainPopSpeed = 25;
constexpr float kGarbageSpeed = 4.8f;

// Global strings for local folders (to shorten expressions)
extern const std::string kFolderUserSounds;
extern const std::string kFolderUserMusic;
extern const std::string kFolderUserBackgrounds;
extern const std::string kFolderUserPuyo;
extern const std::string kFolderUserCharacter;

// Global RNG
extern MersenneTwister gRng;

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
void splitString(std::string& in, char delimiter, StringList& v);
void createFolder(std::string folderName);

// Debugging
extern FeFont* fontArial;
extern std::string debugString;
extern int debugMode;

// Convert int to string
template <class T>
std::string toString(const T& t)
{
	std::stringstream ss;
	ss << t;
	return ss.str();
}

int toInt(const std::string& s);

}
