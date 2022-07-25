#pragma once

#include "DropPattern.h"
#include "FieldProp.h"

namespace ppvs {

class Player;

class AI {
public:
    explicit AI(Player* pl);
	virtual ~AI();

    void prepare(MovePuyoType mpt, int color1, int color2);
	void findLargest();

	int m_bestPos;
	int m_bestRot;
	int m_bestChain;
	int m_timer;
	bool m_pinch;

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
