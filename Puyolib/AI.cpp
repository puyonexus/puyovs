#include "AI.h"
#include "Player.h"

namespace ppvs {

AI::AI(Player* pl)
	: m_player(pl)
{
}

void AI::prepare(const MovePuyoType mpt, const int color1, const int color2)
{
	m_type = mpt;
	m_color1 = color1;
	m_color2 = color2;
	m_bigColor = color1;
	m_timer = 0;
	m_player->m_controls.m_down = 0;
}

void AI::findLargest()
{
	// Try all positions and rotations
	const FieldProp prop = m_player->m_activeField->getProperties();
	int remember = 0;
	for (int i = 0; i < prop.gridX; i++) {
		m_pos[0].x = i;
		m_pos[0].y = prop.gridY;
		for (int j = 0; j < 4; j++) { // Set rotation
			if (m_type != BIG)
				m_rotation = j;
			else
				m_color1 = j;
			setRotation();

			if (const int current = predictChain(); current > remember) {
				m_bestPos = i;
				m_bestRot = j;
				remember = current;
			}
		}
	}
	if ((remember == 0 || remember == 1) && !m_pinch) {
		// Randomize
		m_bestPos = getRandom(prop.gridX);
		m_bestRot = getRandom(4);

		// Reduce chances of bestpos being 2 or 3
		for (int i = 0; i < 2; i++)
			if (m_bestPos == 2 || m_bestPos == 3)
				m_bestPos = getRandom(prop.gridX);
	}
	m_bestChain = remember;
}

void AI::setRotation()
{
	// Refresh
	m_pos[1].x = 0;
	m_pos[1].y = 0;
	m_pos[2].x = 0;
	m_pos[2].y = 0;
	m_pos[3].x = 0;
	m_pos[3].y = 0;

	if (m_type == DOUBLET || m_type == TRIPLET) {
		// Set variables
		if (m_rotation == 0) {
			m_pos[1].x = m_pos[0].x;
			m_pos[1].y = m_pos[0].y + 1;
		} else if (m_rotation == 1) {
			m_pos[1].x = m_pos[0].x + 1;
			m_pos[1].y = m_pos[0].y;
		} else if (m_rotation == 2) {
			m_pos[1].x = m_pos[0].x;
			m_pos[1].y = m_pos[0].y - 1;
		} else if (m_rotation == 3) {
			m_pos[1].x = m_pos[0].x - 1;
			m_pos[1].y = m_pos[0].y;
		}
	}
	if (m_type == TRIPLET) {
		// Set variables
		if (m_rotation == 1) {
			m_pos[2].x = m_pos[0].x;
			m_pos[2].y = m_pos[0].y + 1;
		} else if (m_rotation == 2) {
			m_pos[2].x = m_pos[0].x + 1;
			m_pos[2].y = m_pos[0].y;
		} else if (m_rotation == 3) {
			m_pos[2].x = m_pos[0].x;
			m_pos[2].y = m_pos[0].y - 1;
		} else if (m_rotation == 0) {
			m_pos[2].x = m_pos[0].x - 1;
			m_pos[2].y = m_pos[0].y;
		}
	}
	if (m_type == QUADRUPLET || m_type == BIG) {
		// Quadruplet and big simply stay in pos==1 state
		m_pos[1].x = m_pos[0].x + 1;
		m_pos[1].y = m_pos[0].y;
		m_pos[2].x = m_pos[0].x;
		m_pos[2].y = m_pos[0].y + 1;
		m_pos[3].x = m_pos[0].x + 1;
		m_pos[3].y = m_pos[0].y + 1;
	}
}

int AI::predictChain()
{
	const FieldProp prop = m_player->m_activeField->getProperties();
	const int maxHeight = prop.gridY;
	bool moveUp[4] {};
	PosVectorInt tempPos[4] {};
	int colors[4] {};

	// Check if any puyo is stuck in wall
	for (const auto& pos : m_pos) {
		if (pos.x < 0 || pos.x >= prop.gridX) {
			return 0;
		}
	}

	int n = 2;
	if (m_type == TRIPLET || m_type == TRIPLET_R) {
		n = 3;
		tempPos[3].x = -1;
		tempPos[3].y = -1;
	} else if (m_type == QUADRUPLET || m_type == BIG) {
		n = 4;
	} else {
		tempPos[2].x = -1;
		tempPos[2].y = -1;
		tempPos[3].x = -1;
		tempPos[3].y = -1;
	}

	// Compare colors: which are on the bottom, which are on top?
	for (int k = 0; k < n; k++) {
		moveUp[k] = false;
		for (int j = 1; j < 5; j++) {
			// Compare m_pos with rest
			if (m_pos[k].y > m_pos[(k + j) % 4].y && m_pos[k].x == m_pos[(k + j) % 4].x)
				moveUp[k] = true;
		}
	}

	// Project every color downwards
	for (int k = 0; k < n; k++) {
		int i = 0;
		while (m_player->m_activeField->isEmpty(m_pos[k].x, m_pos[k].y - i) && i < maxHeight) {
			tempPos[k].x = m_pos[k].x;
			tempPos[k].y = m_pos[k].y - i;

			i++;
		}
		if (moveUp[k]) {
			tempPos[k].y += 1;
		}
	}

	// Set colors
	colors[0] = m_color1;
	colors[1] = m_color2;
	if (m_type == TRIPLET_R) {
		colors[1] = m_color1;
		colors[2] = m_color2;
	} else if (m_type == QUADRUPLET) {
		if (m_rotation == 0) {
			colors[0] = m_color2;
			colors[1] = m_color2;
			colors[2] = m_color1;
			colors[3] = m_color1;
		} else if (m_rotation == 1) {
			colors[0] = m_color2;
			colors[1] = m_color1;
			colors[2] = m_color2;
			colors[3] = m_color1;
		} else if (m_rotation == 2) {
			colors[0] = m_color1;
			colors[1] = m_color1;
			colors[2] = m_color2;
			colors[3] = m_color2;
		} else if (m_rotation == 3) {
			colors[0] = m_color1;
			colors[1] = m_color2;
			colors[2] = m_color1;
			colors[3] = m_color2;
		}
	} else if (m_type == BIG) {
		colors[0] = m_color1;
		colors[1] = m_color1;
		colors[2] = m_color1;
		colors[3] = m_color1;
	}

	return m_player->m_activeField->virtualChain(tempPos, n, colors);
}

}
