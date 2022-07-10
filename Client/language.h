#pragma once

#include <QObject>
#include <QMap>

class QFileSystemWatcher;

struct LanguagePriv;
class Language : public QObject
{
    Q_OBJECT
public:
    enum Error
    {
        NoError,
        FileError,
        ReadError,
        EncodingError,
        JsonError
    };

    explicit Language(const QString &fileName, QObject *parent = 0);
    ~Language();

    Error error();
    QString errorString();

    QString realName();
    QString fileName();
    QString translate(const QString &section, const QString &key);

private:
    LanguagePriv *d;
};

class Translator;
class LanguageManager : public QObject
{
    Q_OBJECT
public:
    explicit LanguageManager(QObject *parent = 0);

    bool error();
    QString errorString();

    int languageCount();
    Language *language(int id);
    Language *currentLanguage();
    QString currentLanguageRealName();
    QString currentLanguageFileName();

    int currentLanguageIndex();
    void setCurrentLanguageIndex(int id);

    void setLanguageFromFilename(const QString &filename);

    QString translate(const QString &section, const QString &key);

private slots:
    void refreshLanguageList(QString v=QString());

signals:
    void languagesModified();

private:
    QFileSystemWatcher *mFsWatcher;
    QList <Language*> mLanguages;
    int mCurrentLanguage;
    bool mError;
    QString mErrorString;
    QString mLostLanguage;
    Translator *mTranslator;
};
