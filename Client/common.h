#pragma once

#include <QHash>
#include <QList>
#include <QPair>
#include <QString>

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

namespace ppvs {
enum class Rules : int;
struct RuleSetInfo;
}

// Mode list
typedef QPair<ppvs::Rules, QString> GameModePair;
typedef QList<GameModePair> GameModeList;
typedef QListIterator<QPair<ppvs::Rules, QString>> GameModeListIterator;
GameModeList getModeList();
void readRulesetString(QString str, ppvs::RuleSetInfo* rs);
QString getCacheLocation();
QString getDataLocation();
QString getSettingsLocation();
QString createRulesetString(ppvs::RuleSetInfo* rs);

namespace ilib {
enum class HatPosition : int;
struct InputEvent;
}

class QKeyEvent;

extern QHash<QString, int> nameToCode;
extern QHash<int, QString> codeToName;

class InputCondition {
public:
	enum class MatchResult {
		NoMatch,
		MatchUp,
		MatchDown
	};

	explicit InputCondition(const ilib::InputEvent& e);
	explicit InputCondition(QKeyEvent* e);
	explicit InputCondition(QString str);
	InputCondition();
	~InputCondition();

	[[nodiscard]] MatchResult match(const ilib::InputEvent& e) const;
	[[nodiscard]] MatchResult match(const QKeyEvent* e) const;
	[[nodiscard]] QString toString() const;

private:
	enum class ConditionType {
		KeyType,
		ButtonType,
		AxisType,
		HatType,
		Unknown
	};
	ConditionType type;
	union {
		struct {
			int code;
		} key;
		struct {
			int device;
			int id;
		} button;
		struct {
			int device;
			int id;
			int direction;
		} axis;
		struct {
			int device;
			int id;
			ilib::HatPosition direction;
		} hat;
	};
};

QString getCryptographicHash(QString str);

#ifdef _WIN32
#include <windows.h>
static inline void msleep(unsigned long t)
{
	Sleep(t);
}
#else
#include <unistd.h>
static inline void msleep(unsigned long t)
{
	usleep(t * 1000);
}
#endif
