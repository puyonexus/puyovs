#ifndef SETTINGS_H
#define SETTINGS_H

#include <QStringList>
#include <QObject>
#include <QFile>

#include <json/json.h>

class Settings : public QObject
{
    Q_OBJECT
public:
    explicit Settings(QObject *parent = 0);

    QStringList charMap();
    void setCharMap(const QStringList &map);

    bool getValue(const QString &section, const QString &key, Json::Value &out);
    void setValue(const QString &section, const QString &key, Json::Value in);

    QString string(const QString &section, const QString &key, const QString &defValue);
    void setString(const QString &section, const QString &key, const QString &string);

    bool boolean(const QString &section, const QString &key, bool defValue);
    void setBoolean(const QString &section, const QString &key, bool string);

    qint64 integer(const QString &section, const QString &key, qint64 defValue);
    void setInteger(const QString &section, const QString &key, qint64 value);

    qreal real(const QString &section, const QString &key, qreal defValue);
    void setReal(const QString &section, const QString &key, qreal value);

    void save();
    bool exists();
    bool error();
    
signals:
    void saved();
    
public slots:
    
private:
    Json::Value mSettingsRoot;
    bool mExists;
    bool mError;
};

#endif // SETTINGS_H
