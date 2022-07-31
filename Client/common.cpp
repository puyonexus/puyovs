#include "common.h"
#include "../Puyolib/Game.h"
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QKeyEvent>
#include <QMessageBox>
#include <QObject>
#include <QString>
#include <QStringList>
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
		<< GameModePair(ppvs::Rules::TSU, QObject::tr("Tsu", "ModeTsu"))
		<< GameModePair(ppvs::Rules::ENDLESS, QObject::tr("Endless", "ModeEndless"))
		<< GameModePair(ppvs::Rules::ENDLESSFEVER, QObject::tr("Endless Fever", "ModeEndlessFever"))
		<< GameModePair(ppvs::Rules::FEVER, QObject::tr("Fever", "ModeFever"))
		<< GameModePair(ppvs::Rules::ENDLESSFEVERVS, QObject::tr("Non-stop Fever", "ModeEndlessFeverVs"));
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
	foreach (QString item, items) {
		QStringList subitem = item.split(':');
		int intVal = 0;

		if (subitem.count() < 1)
			continue;

		if (subitem.count() == 2)
			intVal = subitem.at(1).toInt();

		if (subitem[0] == "rules" && subitem.count() != 1) {
			if (subitem[1] == "Tsu")
				rs->setRules(ppvs::Rules::TSU_ONLINE);
			else if (subitem[1] == "Fever")
				rs->setRules(ppvs::Rules::FEVER_ONLINE);
			else if (subitem[1] == "Fever(15th)")
				rs->setRules(ppvs::Rules::FEVER15_ONLINE);
			else if (subitem[1] == "EndlessFeverVS")
				rs->setRules(ppvs::Rules::ENDLESSFEVERVS_ONLINE);
		} else if (subitem[0] == "marginTime")
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
			rs->numPlayers = intVal;
		else if (subitem[0] == "default")
			rs->custom = false;
	}
}

QString createRulesetString(ppvs::RuleSetInfo* rs)
{
	QString ruleString = "Tsu";

	if (rs == nullptr)
		return QString();

	if (rs->ruleSetType == ppvs::Rules::TSU_ONLINE)
		ruleString = "Tsu";
	else if (rs->ruleSetType == ppvs::Rules::FEVER_ONLINE)
		ruleString = "Fever";
	else if (rs->ruleSetType == ppvs::Rules::FEVER15_ONLINE)
		ruleString = "Fever(15th)";
	else if (rs->ruleSetType == ppvs::Rules::ENDLESSFEVERVS_ONLINE)
		ruleString = "EndlessFeverVS";

	if (rs->custom)
		return QString::asprintf("rules:%s|marginTime:%i|targetPoint:%i|requiredChain:%i|initialFeverCount:%i|feverPower:%i|puyoToClear:%i|quickDrop:%i|colors:%i|Nplayers:%i",
			ruleString.toUtf8().data(), rs->marginTime, rs->targetPoint, rs->requiredChain, rs->initialFeverCount, rs->feverPower, rs->puyoToClear, (int)rs->quickDrop, rs->colors, rs->numPlayers);
	return (QStringList() << "rules:" + ruleString << "default" << QString("Nplayers:%1").arg(rs->numPlayers)).join("|");
}

InputCondition::InputCondition(const ilib::InputEvent& e)
{
	switch (e.type) {
	case ilib::InputEvent::Type::ButtonUpEvent:
	case ilib::InputEvent::Type::ButtonDownEvent:
		type = ConditionType::ButtonType;
		button.device = e.device;
		button.id = e.button.id;
		break;
	case ilib::InputEvent::Type::AxisEvent:
		type = ConditionType::AxisType;
		axis.device = e.device;
		axis.id = e.axis.id;
		axis.direction = e.axis.value > 0 ? 1 : -1;
		break;
	case ilib::InputEvent::Type::HatEvent:
		type = ConditionType::HatType;
		hat.device = e.device;
		hat.id = e.hat.id;
		hat.direction = (int)e.hat.value;
		break;
	}
}

InputCondition::InputCondition(QKeyEvent* e)
{
	type = ConditionType::KeyType;
	key.code = e->key();
}

InputCondition::InputCondition(QString str)
{
	if (str.contains(':')) {
		QStringList condparts = str.split(':');

		if (condparts[0] == "pad" && condparts.size() >= 4 && condparts.size() <= 5) {
			int device = condparts[1].toInt();
			if (condparts[2] == "button" && condparts.size() == 4) {
				type = ConditionType::ButtonType;
				button.device = device;
				button.id = condparts[3].toInt();
			} else if (condparts[2] == "axis" && condparts.size() == 5) {
				type = ConditionType::AxisType;
				axis.device = device;
				axis.id = condparts[3].toInt();
				axis.direction = condparts[4] == "+" ? 1 : -1;
			} else if (condparts[2] == "hat" && condparts.size() == 5) {
				type = ConditionType::HatType;
				hat.device = device;
				hat.id = condparts[3].toInt();
				hat.direction = condparts[4].toInt();
			} else
				type = ConditionType::Unknown;
		} else if (condparts[0] == "key" && condparts.size() == 2) {
			type = ConditionType::KeyType;
			key.code = condparts[1].toInt();
		} else
			type = ConditionType::Unknown;
	} else {
		int code = nameToCode[str.toLower()];
		if (code != 0) {
			type = ConditionType::KeyType;
			key.code = code;
		} else
			type = ConditionType::Unknown;
	}
}

InputCondition::InputCondition()
{
	type = ConditionType::Unknown;
}

InputCondition::~InputCondition()
{
}

InputCondition::MatchResult InputCondition::match(const ilib::InputEvent& e) const
{
	switch (e.type) {
	case ilib::InputEvent::Type::ButtonUpEvent:
	case ilib::InputEvent::Type::ButtonDownEvent:
		if (type != ConditionType::ButtonType)
			return MatchResult::NoMatch;
		if (e.device != button.device)
			return MatchResult::NoMatch;
		if (e.button.id != button.id)
			return MatchResult::NoMatch;

		if (e.type == ilib::InputEvent::Type::ButtonDownEvent)
			return MatchResult::MatchDown;
		return MatchResult::MatchUp;

		break;
	case ilib::InputEvent::Type::AxisEvent:
		if (type != ConditionType::AxisType)
			return MatchResult::NoMatch;
		if (e.device != axis.device)
			return MatchResult::NoMatch;
		if (e.axis.id != axis.id)
			return MatchResult::NoMatch;

		if (e.axis.value >= -0.5 && e.axis.value <= 0.5)
			return MatchResult::MatchUp;
		if (axis.direction == 1 && e.axis.value > 0.5)
			return MatchResult::MatchDown;
		if (axis.direction == -1 && e.axis.value < -0.5)
			return MatchResult::MatchDown;

		break;
	case ilib::InputEvent::Type::HatEvent:
		if (type != ConditionType::HatType)
			return MatchResult::NoMatch;
		if (e.device != hat.device)
			return MatchResult::NoMatch;
		if (e.hat.id != hat.id)
			return MatchResult::NoMatch;

		if (e.hat.value | hat.direction)
			return MatchResult::MatchUp;
		return MatchResult::MatchDown;

		break;
	default:
		break;
	}

	return MatchResult::NoMatch;
}

InputCondition::MatchResult InputCondition::match(const QKeyEvent* e) const
{
	if (e->key() == key.code) {
		switch (e->type()) {
		case QEvent::KeyPress:
			return MatchResult::MatchDown;
		case QEvent::KeyRelease:
			return MatchResult::MatchUp;
		default:
			break;
		}
	}
	return MatchResult::NoMatch;
}

QString InputCondition::toString() const
{
	QString name;
	switch (type) {
	case ConditionType::KeyType:
		name = codeToName[key.code];

		if (!name.isEmpty())
			return name;

		return (QStringList() << "key" << QString::number(key.code)).join(":");
		break;
	case ConditionType::ButtonType:
		return (QStringList() << "pad" << QString::number(button.device) << "button" << QString::number(button.id)).join(":");
		break;
	case ConditionType::AxisType:
		return (QStringList() << "pad" << QString::number(axis.device) << "axis" << QString::number(axis.id) << (axis.direction > 0 ? "+" : "-")).join(":");
		break;
	case ConditionType::HatType:
		return (QStringList() << "pad" << QString::number(hat.device) << "hat" << QString::number(hat.id) << QString::number(hat.direction)).join(":");
		break;
	case ConditionType::Unknown:
		return "unknown";
		break;
	}
	return QString();
}
