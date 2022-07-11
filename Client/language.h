#pragma once

#include <QObject>

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

	explicit Language(const QString& fileName, QObject* parent = nullptr);
	~Language() override;

	Error error() const;
	QString errorString() const;

	QString realName() const;
	QString fileName() const;
	QString translate(const QString& section, const QString& key) const;

private:
	LanguagePriv* d;
};

class Translator;
class LanguageManager : public QObject
{
	Q_OBJECT
public:
	explicit LanguageManager(QObject* parent = nullptr);

	bool error() const;
	QString errorString();

	int languageCount() const;
	Language* language(int id) const;
	Language* currentLanguage() const;
	QString currentLanguageRealName() const;
	QString currentLanguageFileName() const;

	int currentLanguageIndex() const;
	void setCurrentLanguageIndex(int id);

	void setLanguageFromFilename(const QString& filename);

	QString translate(const QString& section, const QString& key) const;

private slots:
	void refreshLanguageList(QString v = QString());

signals:
	void languagesModified();

private:
	QFileSystemWatcher* mFsWatcher;
	QList <Language*> mLanguages;
	int mCurrentLanguage;
	bool mError;
	QString mErrorString;
	QString mLostLanguage;
	Translator* mTranslator;
};
