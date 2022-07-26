#include "settings.h"
#include <QByteArray>
#include <QDir>

#include "common.h"

Settings::Settings(QObject* parent)
	: QObject(parent)
{
	mError = false;

	// Get a file handle.
	QString settingsFile = getDataLocation() + "/Settings.json";
	QFile file(settingsFile);

	// Open file.
	if (!file.open(QFile::ReadOnly)) {
		mExists = false;
		return;
	}
	mExists = true;

	// Read data.
	QByteArray data = file.readAll();
	file.close();

	// Handle empty file/file read error.
	if (data.isEmpty()) {
		mError = true;
		return;
	}

	// Handle UTF-8 BOM.
	if (data.startsWith("\xEF\xBB\xBF"))
		data.remove(0, 3);

	// Parse JSON data.
	Json::Reader reader;
	if (!reader.parse(data.data(), mSettingsRoot, false)) {
		mError = true;
		return;
	}
}

QStringList Settings::charMap()
{
	if (!mSettingsRoot.isMember("settings") || !mSettingsRoot["settings"].isMember("characters"))
		return QStringList();

	Json::Value value = mSettingsRoot["settings"]["characters"];

	if (!value.isArray())
		return QStringList();

	QStringList charList;
	int numElements = value.size();

	for (int i = 0; i < numElements; ++i) {
		charList.append(QString::fromStdString(value[i].asString()));
	}

	return charList;
}

void Settings::setCharMap(const QStringList& map)
{
	if (!mSettingsRoot.isMember("settings"))
		mSettingsRoot["settings"] = Json::Value(Json::objectValue);

	Json::Value value = Json::Value(Json::arrayValue);

	int mapSize = map.size();

	for (int i = 0; i < mapSize; ++i) {
		value.append(Json::Value(map.at(i).toStdString()));
	}

	mSettingsRoot["settings"]["characters"] = value;
}

bool Settings::getValue(const QString& section, const QString& key, Json::Value& out)
{
	std::string sectionstd = section.toStdString();
	std::string keystd = key.toStdString();

	if (!mSettingsRoot.isMember("settings") || !mSettingsRoot["settings"].isMember(sectionstd) || !mSettingsRoot["settings"][sectionstd].isMember(keystd))
		return false;

	out = mSettingsRoot["settings"][sectionstd][keystd];
	return true;
}

void Settings::setValue(const QString& section, const QString& key, Json::Value in)
{
	std::string sectionstd = section.toStdString();
	std::string keystd = key.toStdString();

	if (!mSettingsRoot.isMember("settings"))
		mSettingsRoot["settings"] = Json::Value(Json::objectValue);

	if (!mSettingsRoot["settings"].isMember(sectionstd))
		mSettingsRoot["settings"][sectionstd] = Json::Value(Json::objectValue);

	mSettingsRoot["settings"][sectionstd][keystd] = in;
}

QString Settings::string(const QString& section, const QString& key, const QString& defValue)
{
	Json::Value value = Json::Value(Json::nullValue);

	getValue(section, key, value);

	if (value.isNull()) {
		setString(section, key, defValue);
		return defValue;
	}

	return QString::fromStdString(value.asString());
}

void Settings::setString(const QString& section, const QString& key, const QString& value)
{
	std::string valuestd = value.toStdString();

	setValue(section, key, Json::Value(valuestd));
}

bool Settings::boolean(const QString& section, const QString& key, bool defValue)
{
	Json::Value value = Json::Value(Json::nullValue);

	getValue(section, key, value);

	if (value.isNull()) {
		setBoolean(section, key, defValue);
		return defValue;
	}

	return value.asBool();
}

void Settings::setBoolean(const QString& section, const QString& key, bool value)
{
	setValue(section, key, Json::Value(value));
}

qint64 Settings::integer(const QString& section, const QString& key, qint64 defValue)
{
	Json::Value value = Json::Value(Json::nullValue);

	getValue(section, key, value);

	if (value.isNull()) {
		setInteger(section, key, defValue);
		return defValue;
	}

	return value.asInt64();
}

void Settings::setInteger(const QString& section, const QString& key, qint64 value)
{
	setValue(section, key, Json::Value(value));
}

qreal Settings::real(const QString& section, const QString& key, qreal defValue)
{
	Json::Value value = Json::Value(Json::nullValue);

	getValue(section, key, value);

	if (value.isNull()) {
		setReal(section, key, defValue);
		return defValue;
	}

	return value.asDouble();
}

void Settings::setReal(const QString& section, const QString& key, qreal value)
{
	setValue(section, key, Json::Value(value));
}

void Settings::save()
{
	QString settingsFile = getDataLocation() + "/Settings.json";
	QFile file(settingsFile);

	mError = false;
	if (!file.open(QFile::WriteOnly)) {
		mError = true;
		return;
	}

	Json::StyledWriter writer;
	QString strdata = QString::fromStdString(writer.write(mSettingsRoot));
	QByteArray data = strdata.toUtf8();

	if (file.write(data) != -1) {
		mError = true;
		return;
	}

	file.close();

	mExists = true;

	emit saved();
}

bool Settings::exists() const
{
	return mExists;
}

bool Settings::error() const
{
	return mError;
}
