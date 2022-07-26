#include "Controller.h"

namespace ppvs {

/*
0=press		up
1=release	up
2=press		down
3=release	down
4=press		left
5=release	left
6=press		right
7=release	right
8=press		A
9=release	A
10=press	B
11=release	B
*/

void Controller::init(const int playerNumber, const PlayerType type, const RecordState s)
{
	m_playerNumber = playerNumber;
	m_type = type;
	m_state = s;
}

void Controller::setState(const FeInput& input, int t)
{
	if (m_type == ONLINE) // ONLINE controls
		return;

	// Set indicators with delay
	if (m_up)
		m_delayUp = true;
	else
		m_delayUp = false;
	if (m_down)
		m_delayDown = true;
	else
		m_delayDown = false;
	if (m_left)
		m_delayLeft = true;
	else
		m_delayLeft = false;
	if (m_right)
		m_delayRight = true;
	else
		m_delayRight = false;
	if (m_a)
		m_delayA = true;
	else
		m_delayA = false;
	if (m_b)
		m_delayB = true;
	else
		m_delayB = false;
	if (m_start)
		m_delayStart = true;
	else
		m_delayStart = false;

	if (m_state != RecordState::REPLAYING) {
		if (input.a)
			m_a++;
		else
			m_a = 0;
		if (input.b)
			m_b++;
		else
			m_b = 0;
		if (input.up)
			m_up++;
		else
			m_up = 0;
		if (input.down)
			m_down++;
		else
			m_down = 0;
		if (input.left)
			m_left++;
		else
			m_left = 0;
		if (input.right)
			m_right++;
		else
			m_right = 0;
		if (input.start)
			m_start++;
		else
			m_start = 0;

		// Record state
		if (m_state == RecordState::RECORDING && t > 0)
			record(t);
	} else {
		// Check event by looping through vector
		// If the time is -1, it's considered as processed
		for (auto& m_recordEvent : m_recordEvents) {
			if (m_recordEvent.time != -1 && m_recordEvent.time == t) {
				// Process all events with the correct time
				switch (m_recordEvent.ev) {
				case 0:
					m_dUp = true;
					break;
				case 1:
					m_dUp = false;
					break;
				case 2:
					m_dDown = true;
					break;
				case 3:
					m_dDown = false;
					break;
				case 4:
					m_dLeft = true;
					break;
				case 5:
					m_dLeft = false;
					break;
				case 6:
					m_dRight = true;
					break;
				case 7:
					m_dRight = false;
					break;
				case 8:
					m_dA = true;
					break;
				case 9:
					m_dA = false;
					break;
				case 10:
					m_dB = true;
					break;
				case 11:
					m_dB = false;
					break;
				default:
					break;
				}
				m_recordEvent.time = -1;
			} else if (m_recordEvent.time > t)
				break;
		}
		// Set controls
		if (m_dUp)
			m_up++;
		else
			m_up = 0;
		if (m_dDown)
			m_down++;
		else
			m_down = 0;
		if (m_dLeft)
			m_left++;
		else
			m_left = 0;
		if (m_dRight)
			m_right++;
		else
			m_right = 0;
		if (m_dA)
			m_a++;
		else
			m_a = 0;
		if (m_dB)
			m_b++;
		else
			m_b = 0;
	}
}

void Controller::release()
{
	// Artificially release all buttons
	m_down = 0;
	m_up = 0;
	m_left = 0;
	m_right = 0;
	m_a = 0;
	m_b = 0;
	m_start = 0;

	m_delayDown = false;
	m_delayUp = false;
	m_delayLeft = false;
	m_delayRight = false;
	m_delayA = false;
	m_delayB = false;
	m_delayStart = false;

	m_dUp = false;
	m_dDown = false;
	m_dLeft = false;
	m_dRight = false;
	m_dA = false;
	m_dB = false;
	m_dStart = false;
}

void Controller::record(int t)
{
	if (m_up > 0 && m_dUp == false) {
		m_dUp = true;
		m_recordEvents.emplace_back(t, 0);
	}
	if (m_up == 0 && m_dUp == true) {
		m_dUp = false;
		m_recordEvents.emplace_back(t, 1);
	}
	if (m_down > 0 && m_dDown == false) {
		m_dDown = true;
		m_recordEvents.emplace_back(t, 2);
	}
	if (m_down == 0 && m_dDown == true) {
		m_dDown = false;
		m_recordEvents.emplace_back(t, 3);
	}
	if (m_left > 0 && m_dLeft == false) {
		m_dLeft = true;
		m_recordEvents.emplace_back(t, 4);
	}
	if (m_left == 0 && m_dLeft == true) {
		m_dLeft = false;
		m_recordEvents.emplace_back(t, 5);
	}
	if (m_right > 0 && m_dRight == false) {
		m_dRight = true;
		m_recordEvents.emplace_back(t, 6);
	}
	if (m_right == 0 && m_dRight == true) {
		m_dRight = false;
		m_recordEvents.emplace_back(t, 7);
	}
	if (m_a > 0 && m_dA == false) {
		m_dA = true;
		m_recordEvents.emplace_back(t, 8);
	}
	if (m_a == 0 && m_dA == true) {
		m_dA = false;
		m_recordEvents.emplace_back(t, 9);
	}
	if (m_b > 0 && m_dB == false) {
		m_dB = true;
		m_recordEvents.emplace_back(t, 10);
	}
	if (m_b == 0 && m_dB == true) {
		m_dB = false;
		m_recordEvents.emplace_back(t, 11);
	}
}

}
