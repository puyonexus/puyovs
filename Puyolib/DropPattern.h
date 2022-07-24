#pragma once
#include <vector>
#include <string>
#include "global.h"

namespace ppvs
{

// Contains droppatterns, fever patterns, voice patterns etc.

// Define types of movepuyo
enum MovePuyoType
{
	DOUBLET,
	TRIPLET,
	QUADRUPLET,
	BIG,
	TRIPLETR
};

struct DropSet
{
	MovePuyoType mpt[16];
};

// Call this function to get the movepuyoType
MovePuyoType getFromDropPattern(PuyoCharacter, int);

// Drop pattern for nuisance
int nuisanceDropPattern(int maxX, int cycle);
void createNuisancePattern(int max, int* array);

// See global voicePattern variable
int getVoicePattern(int chain, int predicted, bool fever = true);

}
