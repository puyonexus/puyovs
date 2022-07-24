#pragma once

#include <QList>
#include <QPair>
#include <QString>
#include <QHash>

#include "pvsapplication.h"

// Protocol version (usually matches build str)
#define PVSVERSION 32
// Build string (displayed in title bar and other places)
#define PUYOVS_BUILD_STR "v32"
// Default username
#define PUYOVS_DEFAULT_USERNAME "Name"
// Default server
#define PUYOVS_DEFAULT_SERVER "ds-net.jp"
// User agent
#define PUYOVS_USER_AGENT "Mozilla/5.0 (Windows NT 6.3; Trident/7.0; rv:11.0) like Gecko"

namespace ppvs
{
struct RuleSetInfo;
}

// Modelist
typedef QPair<int, QString> GameModePair;
typedef QList<GameModePair> GameModeList;
typedef QListIterator<QPair<int, QString> > GameModeListIterator;
GameModeList getModeList();
void readRulesetString(QString str, ppvs::RuleSetInfo* rs);
QString getDataLocation();
QString createRulesetString(ppvs::RuleSetInfo* rs);

namespace ilib
{
struct InputEvent;
}

class QKeyEvent;

extern QHash<QString, int> nameToCode;
extern QHash<int, QString> codeToName;

class InputCondition
{
public:
	enum MatchResult { NoMatch, MatchUp, MatchDown };

	InputCondition(const ilib::InputEvent& e);
	InputCondition(QKeyEvent* e);
	InputCondition(QString str);
	InputCondition();
	~InputCondition();

	MatchResult match(const ilib::InputEvent& e) const;
	MatchResult match(QKeyEvent* e) const;
	QString toString() const;

private:
	enum condtype { keytype, buttontype, axistype, hattype, unknown };
	int type;
	union
	{
		struct { int code; } key;
		struct { int device; int id; } button;
		struct { int device; int id; int direction; } axis;
		struct { int device; int id; int direction; } hat;
	};
};

QString getCryptographicHash(QString str);

#ifdef _WIN32
#include <windows.h>
static inline
void msleep(unsigned long t) { Sleep(t); }
#else
#include <unistd.h>
static inline
void msleep(unsigned long t) { usleep(t * 1000); }
#endif
