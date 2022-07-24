#pragma once

#include "global.h"
#include <string>

namespace ppvs
{

struct controllerEvent
{
	controllerEvent() : time(0), ev(0) { }
	controllerEvent(int t, char ev) : time(t), ev(ev) { }
	int time;
	char ev;
};

struct finput;

class controller
{
public:
	controller();
	~controller();

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

	void init(int playernumber, playerType type, recordState s = PVS_NOTRECORDING);
	void setstate(const finput& Input, int t);
	void release();
	void record(int t);

	recordState state;
	std::vector<controllerEvent> recordEvents;

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
	playerType m_type;
};
}
