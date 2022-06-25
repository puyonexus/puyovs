#ifndef DROPPATTERN_H
#define DROPPATTERN_H
#include <vector>
#include <string>
#include "global.h"

namespace ppvs
{
//contains droppatterns, fever patterns, voice patterns etc.

/*enum puyoCharacter
{//define characters
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
    ONION_PIXY,
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
};*/

enum movePuyoType
{//define types of movepuyo
    DOUBLET,TRIPLET,QUADRUPLET,BIG,TRIPLETR
};

struct drop_struct
{
    movePuyoType mpt[16];
};

//Call this function to get the movepuyoType
movePuyoType getFromDropPattern(puyoCharacter,int);
//Drop pattern for nuisance
int nuisanceDropPattern(int maxX,int cycle);
void createNuisancePattern(int max,int *array);

//see global voicePattern variable
int getVoicePattern(int chain,int predicted,bool fever=true);

}
#endif // DROPPATTERN_H
