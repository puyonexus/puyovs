#pragma once

#include "global.h"
#include <string>

namespace ppvs
{

struct ControllerEvent
{
	ControllerEvent() : time(0), ev(0) { }
	ControllerEvent(int t, char ev) : time(t), ev(ev) { }
	int time;
	char ev;
};

struct FeInput;

class Controller
{
public:
	Controller();
	~Controller();

	int Down;
	int Up;
	int Left;
	int Right;
	int A;
	int B;
	int Start;

	bool DelayDown;
	bool DelayUp;
	bool DelayLeft;
	bool DelayRight;
	bool DelayA;
	bool DelayB;
	bool DelayStart;

	void init(int playernumber, PlayerType type, RecordState s = PVS_NOTRECORDING);
	void setstate(const FeInput& Input, int t);
	void release();
	void record(int t);

	RecordState state;
	std::vector<ControllerEvent> recordEvents;

private:
	// Delayed checking
	bool dUp;
	bool dDown;
	bool dLeft;
	bool dRight;
	bool dA;
	bool dB;
	bool dStart;

	bool m_defined;
	int m_playernumber;
	PlayerType m_type;
};
}
