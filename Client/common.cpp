#include <QObject>
#include <QString>
#include <QMessageBox>
#include <QStringList>
#include <QKeyEvent>
#include <QCryptographicHash>
#include <QDesktopServices>
#include "common.h"
#include "../Puyolib/Game.h"
#include <ilib/inputevent.h>

QHash<QString, int> buildNameToCode()
{
	QHash<QString, int> hashmap;
#define key(code, name) hashmap[name] = code;
#include "keynames.h"
#undef key
	return hashmap;
}

QHash<int, QString> buildCodeToName()
{
	QHash<int, QString> hashmap;
#define key(code, name) hashmap[code] = name;
#include "keynames.h"
#undef key
	return hashmap;
}

QHash<QString, int> nameToCode = buildNameToCode();
QHash<int, QString> codeToName = buildCodeToName();

GameModeList getModeList()
{
	GameModeList modeList = GameModeList()
		<< GameModePair(TSU, QObject::tr("Tsu", "ModeTsu"))
		<< GameModePair(ENDLESS, QObject::tr("Endless", "ModeEndless"))
		<< GameModePair(ENDLESSFEVER, QObject::tr("Endless Fever", "ModeEndlessFever"))
		<< GameModePair(FEVER, QObject::tr("Fever", "ModeFever"))
		<< GameModePair(ENDLESSFEVERVS, QObject::tr("Non-stop Fever", "ModeEndlessFeverVs"));
	return modeList;
}

QString getCryptographicHash(QString str)
{
	QString s(str + str + "pvs");
	return QString(QCryptographicHash::hash(s.toUtf8(), QCryptographicHash::Md5).toHex());
}

QString getDataLocation()
{
#if QT_VERSION >= 0x050000
	// Qt5: QStandardPaths::StandardLocation
	return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
	// Qt4
	return QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif
}

void readRulesetString(QString str, ppvs::RuleSetInfo* rs)
{
	// assert rs != 0

	QStringList items = str.split('|');
	foreach(QString item, items)
	{
		QStringList subitem = item.split(':');
		int intVal = 0;

		if (subitem.count() < 1)
			continue;

		if (subitem.count() == 2)
			intVal = subitem.at(1).toInt();

		if (subitem[0] == "rules" && subitem.count() != 1)
		{
			if (subitem[1] == "Tsu")
				rs->setRules(TSU_ONLINE);
			else if (subitem[1] == "Fever")
				rs->setRules(FEVER_ONLINE);
			else if (subitem[1] == "Fever(15th)")
				rs->setRules(FEVER15_ONLINE);
			else if (subitem[1] == "EndlessFeverVS")
				rs->setRules(ENDLESSFEVERVS_ONLINE);
		}
		else if (subitem[0] == "marginTime")
			rs->marginTime = intVal;
		else if (subitem[0] == "targetPoint")
			rs->targetPoint = intVal;
		else if (subitem[0] == "requiredChain")
			rs->requiredChain = intVal;
		else if (subitem[0] == "initialFeverCount")
			rs->initialFeverCount = intVal;
		else if (subitem[0] == "feverPower")
			rs->feverPower = intVal;
		else if (subitem[0] == "puyoToClear")
			rs->puyoToClear = intVal;
		else if (subitem[0] == "quickDrop")
			rs->quickDrop = intVal;
		else if (subitem[0] == "colors")
			rs->colors = intVal;
		else if (subitem[0] == "Nplayers")
			rs->Nplayers = intVal;
		else if (subitem[0] == "default")
			rs->custom = false;
	}
}


QString createRulesetString(ppvs::RuleSetInfo* rs)
{
	QString ruleString = "Tsu";

	if (rs == nullptr)
		return QString();

	if (rs->rulesetType == TSU_ONLINE) ruleString = "Tsu";
	else if (rs->rulesetType == FEVER_ONLINE) ruleString = "Fever";
	else if (rs->rulesetType == FEVER15_ONLINE) ruleString = "Fever(15th)";
	else if (rs->rulesetType == ENDLESSFEVERVS_ONLINE) ruleString = "EndlessFeverVS";

	if (rs->custom)
		return QString::asprintf("rules:%s|marginTime:%i|targetPoint:%i|requiredChain:%i|initialFeverCount:%i|feverPower:%i|puyoToClear:%i|quickDrop:%i|colors:%i|Nplayers:%i",
			ruleString.toUtf8().data(), rs->marginTime, rs->targetPoint, rs->requiredChain, rs->initialFeverCount, rs->feverPower, rs->puyoToClear, (int)rs->quickDrop, rs->colors, rs->Nplayers);
	return (QStringList() << "rules:" + ruleString << "default" << QString("Nplayers:%1").arg(rs->Nplayers)).join("|");
}

InputCondition::InputCondition(const ilib::InputEvent& e)
{
	switch (e.type)
	{
	case ilib::InputEvent::ButtonUpEvent:
	case ilib::InputEvent::ButtonDownEvent:
		type = buttontype;
		button.device = e.device;
		button.id = e.button.id;
		break;
	case ilib::InputEvent::AxisEvent:
		type = axistype;
		axis.device = e.device;
		axis.id = e.axis.id;
		axis.direction = e.axis.value > 0 ? 1 : -1;
		break;
	case ilib::InputEvent::HatEvent:
		type = hattype;
		hat.device = e.device;
		hat.id = e.hat.id;
		hat.direction = (int)e.hat.value;
		break;
	}
}

InputCondition::InputCondition(QKeyEvent* e)
{
	type = keytype;
	key.code = e->key();
}

InputCondition::InputCondition(QString str)
{
	if (str.contains(':'))
	{
		QStringList condparts = str.split(':');

		if (condparts[0] == "pad" && condparts.size() >= 4 && condparts.size() <= 5)
		{
			int device = condparts[1].toInt();
			if (condparts[2] == "button" && condparts.size() == 4)
			{
				type = buttontype;
				button.device = device;
				button.id = condparts[3].toInt();
			}
			else if (condparts[2] == "axis" && condparts.size() == 5)
			{
				type = axistype;
				axis.device = device;
				axis.id = condparts[3].toInt();
				axis.direction = condparts[4] == "+" ? 1 : -1;
			}
			else if (condparts[2] == "hat" && condparts.size() == 5)
			{
				type = hattype;
				hat.device = device;
				hat.id = condparts[3].toInt();
				hat.direction = condparts[4].toInt();
			}
			else type = unknown;
		}
		else if (condparts[0] == "key" && condparts.size() == 2)
		{
			type = keytype;
			key.code = condparts[1].toInt();
		}
		else type = unknown;
	}
	else
	{
		int code = nameToCode[str.toLower()];
		if (code != 0)
		{
			type = keytype;
			key.code = code;
		}
		else type = unknown;
	}
}

InputCondition::InputCondition()
{
	type = unknown;
}

InputCondition::~InputCondition()
{
}

InputCondition::MatchResult InputCondition::match(const ilib::InputEvent& e) const
{
	switch (e.type)
	{
	case ilib::InputEvent::ButtonUpEvent:
	case ilib::InputEvent::ButtonDownEvent:
		if (type != buttontype) return NoMatch;
		if (e.device != button.device) return NoMatch;
		if (e.button.id != button.id) return NoMatch;

		if (e.type == ilib::InputEvent::ButtonDownEvent) return MatchDown;
		return MatchUp;

		break;
	case ilib::InputEvent::AxisEvent:
		if (type != axistype) return NoMatch;
		if (e.device != axis.device) return NoMatch;
		if (e.axis.id != axis.id) return NoMatch;

		if (e.axis.value >= -0.5 && e.axis.value <= 0.5) return MatchUp;
		if (axis.direction == 1 && e.axis.value > 0.5) return MatchDown;
		if (axis.direction == -1 && e.axis.value < -0.5) return MatchDown;

		break;
	case ilib::InputEvent::HatEvent:
		if (type != hattype) return NoMatch;
		if (e.device != hat.device) return NoMatch;
		if (e.hat.id != hat.id) return NoMatch;

		if (e.hat.value | hat.direction) return MatchUp;
		return MatchDown;

		break;
	default:
		break;
	}

	return NoMatch;
}

InputCondition::MatchResult InputCondition::match(QKeyEvent* e) const
{
	switch (e->type())
	{
	case QEvent::KeyPress:
		if (e->key() == key.code)
			return MatchDown;
	case QEvent::KeyRelease:
		if (e->key() == key.code)
			return MatchUp;
	default:
		break;
	}

	return NoMatch;
}

QString InputCondition::toString() const
{
	QString name;
	switch (type)
	{
	case keytype:
		name = codeToName[key.code];

		if (!name.isEmpty())
			return name;

		return (QStringList() << "key" << QString::number(key.code)).join(":");
		break;
	case buttontype:
		return (QStringList() << "pad" << QString::number(button.device) << "button" << QString::number(button.id)).join(":");
		break;
	case axistype:
		return (QStringList() << "pad" << QString::number(axis.device) << "axis" << QString::number(axis.id) << (axis.direction > 0 ? "+" : "-")).join(":");
		break;
	case hattype:
		return (QStringList() << "pad" << QString::number(hat.device) << "hat" << QString::number(hat.id) << QString::number(hat.direction)).join(":");
		break;
	case unknown:
		return "unknown";
		break;
	}
	return QString();
}
