#pragma once

#include "FieldProp.h"
#include "DropPattern.h"

namespace ppvs
{

class Player;

class AI
{
public:
	AI(Player* pl);
	virtual ~AI();
	void prepare(MovePuyoType mpt, int color1, int color2);
	void findLargest();

	int bestPos;
	int bestRot;
	int bestChain;
	int timer;
	bool pinch;

private:
	int predictChain();
	void setRotation();
	MovePuyoType m_type;
	PosVectorInt m_pos[4];
	unsigned int m_rotation;
	int m_color1, m_color2;
	unsigned int m_bigColor;

	Player* m_player;
};

}
