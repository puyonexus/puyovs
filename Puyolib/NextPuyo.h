#pragma once

#include "DropPattern.h"
#include "Sprite.h"
#include "global.h"
#include <deque>

namespace ppvs {

class NextPuyo {
public:
	NextPuyo();
	~NextPuyo();

	NextPuyo(const NextPuyo&) = delete;
	NextPuyo& operator=(const NextPuyo&) = delete;
	NextPuyo(NextPuyo&&) = delete;
	NextPuyo& operator=(NextPuyo&&) = delete;

    GameData* m_data;
	void init(float x, float y, float scale, bool orientation, GameData* data);
	void initImage();
	void setPuyo();
	void play();
	void update(const std::deque<int>& dq, PuyoCharacter p, int turn);
	
    [[nodiscard]] int getOrientation() const;
	void draw();
	void idle();
	void test();

private:
	void setSprite(Sprite&, Sprite&, Sprite&, Sprite&, const int&, const int&, const MovePuyoType&) const;
	void resetPuyoPos();
	bool m_orientation;
	bool m_initialize;
	float m_offsetX, m_offsetY, m_scale;
	float m_pair1X{}, m_pair1Y{}, m_pair2X{}, m_pair2Y{}, m_pair3X{}, m_pair3Y{};
	Sprite m_background;
	Sprite m_cutSprite;
	Sprite m_sprite11, m_sprite12, m_sprite21, m_sprite22, m_sprite31, m_sprite32;
	Sprite m_eye11, m_eye12, m_eye21, m_eye22, m_eye31, m_eye32;
	Sprite m_final;

	int m_color11, m_color12, m_color21, m_color22, m_color31, m_color32;
	int m_goNext;
	int m_sign;
	MovePuyoType m_type1 = DOUBLET, m_type2 = DOUBLET, m_type3 = DOUBLET;
};

}
