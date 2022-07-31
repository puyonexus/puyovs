#pragma once

#include "DropPattern.h"
#include "FieldProp.h"

namespace ppvs {

class Player;

class AI final {
public:
	explicit AI(Player* pl);
	~AI() = default;

	AI(const AI&) = delete;
	AI& operator=(const AI&) = delete;
	AI(AI&&) = delete;
	AI& operator=(AI&&) = delete;

	void prepare(MovePuyoType mpt, int color1, int color2);
	void findLargest();

	int m_bestPos = 0;
	int m_bestRot = 0;
	int m_bestChain = 0;
	int m_timer = 0;
	bool m_pinch = false;

private:
	int predictChain();
	void setRotation();
	MovePuyoType m_type = MovePuyoType::DOUBLET;
	PosVectorInt m_pos[4] = {};
	unsigned int m_rotation = 0u;
	int m_color1 = 0, m_color2 = 0;
	unsigned int m_bigColor = 0;

	Player* m_player;
};

}
