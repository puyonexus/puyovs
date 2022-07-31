#pragma once
#include "global.h"

namespace ppvs
{

// Contains drop patterns, fever patterns, voice patterns etc.

// Define types of move puyo
enum class MovePuyoType
{
	DOUBLET,
	TRIPLET,
	QUADRUPLET,
	BIG,
	TRIPLET_R
};

struct DropSet
{
	MovePuyoType mpt[16];
};

// Call this function to get the move puyo type
MovePuyoType getFromDropPattern(PuyoCharacter, int);

// Drop pattern for nuisance
int nuisanceDropPattern(int maxX, int cycle);
void createNuisancePattern(int max, int* array);

// See global voicePattern variable
int getVoicePattern(int chain, int predicted, bool fever = true);

}
