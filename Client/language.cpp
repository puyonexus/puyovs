#include "language.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileSystemWatcher>
#include <QTranslator>
#include <json/json.h>

// Language
struct LanguagePriv {
	LanguagePriv(QString fn)
		: currentError(Language::Error::NoError)
		, filename(std::move(fn))
	{
	}

	Json::Value langRoot;
	Language::Error currentError;
	QString filename;
	Json::Reader reader;
};

Language::Language(const QString& filename, QObject* parent)
	: QObject(parent)
	, d(new LanguagePriv(filename))
{
	// Get a file handle.
	QFile file(filename);

	// Open file.
	if (!file.open(QFile::ReadOnly)) {
		d->currentError = Error::FileError;
		return;
	}

	// Read data.
	QByteArray data = file.readAll();

	// Handle empty file/file read error.
	if (data.isEmpty()) {
		d->currentError = Error::ReadError;
		return;
	}

	// Handle UTF-8 BOM.
	if (data.startsWith("\xEF\xBB\xBF")) {
		qDebug() << "Warning:" << filename << "contains a UTF-8 BOM. Discarded.";
		data.remove(0, 3);
	}

	// Parse JSON data.
	if (!d->reader.parse(data.data(), d->langRoot, false)) {
		d->currentError = Error::JsonError;
		return;
	}
}

Language::~Language()
{
	delete d;
}

Language::Error Language::error() const
{
	return d->currentError;
}

QString Language::errorString() const
{
	switch (d->currentError) {
	case Error::NoError:
		return QString("No error.");
	case Error::FileError:
		return QString("An error occured while trying to open the file for reading.");
	case Error::ReadError:
		return QString("An error occured while trying to read the file.");
	case Error::JsonError:
		return QString("An error occured while trying to parse the file.\n").append(QString::fromStdString(d->reader.getFormattedErrorMessages()));
	default:
		return QString("An unknown error occured.");
	}
}

QString Language::realName() const
{
	if (d->langRoot.isMember("LanguageNative"))
		return QString::fromUtf8(d->langRoot["LanguageNative"].asString().c_str());
	return QString();
}

QString Language::fileName() const
{
	return d->filename;
}

QString Language::translate(const QString& section, const QString& key) const
{
	QByteArray sectionu8 = section.toUtf8();

	if (!d->langRoot.isMember(sectionu8.data()))
		return QString();

	Json::Value& langSection = d->langRoot[sectionu8.data()];

	QByteArray keyu8 = key.toUtf8();
	if (!langSection.isMember(keyu8.data()))
		return QString();

	Json::Value& langStr = langSection[keyu8.data()];

	if (langStr.isString())
		return QString::fromUtf8(langStr.asString().c_str());
	return QString();
}

// Translator
class Translator : public QTranslator {
public:
	Translator(LanguageManager* manager)
		: QTranslator(manager)
		, mManager(manager)
	{
	}

	QString translatedmbg(const char* disambiguation) const
	{
		QString d = QString::fromUtf8(disambiguation);
		if (d.contains(':')) {
			QStringList dp = d.split(':');

			if (dp.count() != 2)
				return QString();
			return mManager->translate(dp.at(0), dp.at(1));
		}
		return mManager->translate("Launcher", QString(disambiguation));
	}

	QString translate(const char* /*context*/, const char* sourceText,
		const char* disambiguation = nullptr, int n = -1) const override
	{
		if (disambiguation != nullptr && strlen(disambiguation) != 0) {
			QString translation = translatedmbg(disambiguation);

			if (translation.isEmpty()) {
				return mManager->translate("Launcher", QString(sourceText));
			}
			return translation;
		}
		return mManager->translate("Launcher", QString(sourceText));
	}

private:
	LanguageManager* mManager;
};

// LanguageManager
LanguageManager::LanguageManager(QObject* parent)
	: QObject(parent)
{
	mFsWatcher = new QFileSystemWatcher(QStringList() << "./Language/", this);
	connect(mFsWatcher, SIGNAL(directoryChanged(QString)), SLOT(refreshLanguageList(QString)));
	connect(mFsWatcher, SIGNAL(fileChanged(QString)), SLOT(refreshLanguageList(QString)));
	mTranslator = new Translator(this);

	mCurrentLanguage = -1;

	refreshLanguageList();
}

bool LanguageManager::error() const
{
	return mError;
}

QString LanguageManager::errorString()
{
	return mErrorString;
}

int LanguageManager::languageCount() const
{
	return mLanguages.count();
}

Language* LanguageManager::language(int id) const
{
	return mLanguages.at(id);
}

Language* LanguageManager::currentLanguage() const
{
	if (mCurrentLanguage != -1)
		return mLanguages.at(mCurrentLanguage);
	return nullptr;
}

QString LanguageManager::currentLanguageRealName() const
{
	if (mCurrentLanguage != -1)
		return mLanguages.at(mCurrentLanguage)->realName();
	return QString();
}

QString LanguageManager::currentLanguageFileName() const
{
	if (mCurrentLanguage != -1)
		return mLanguages.at(mCurrentLanguage)->fileName();
	return QString();
}

int LanguageManager::currentLanguageIndex() const
{
	return mCurrentLanguage;
}

void LanguageManager::setCurrentLanguageIndex(int id)
{
	mLostLanguage = QString();
	mCurrentLanguage = qBound(-1, id, mLanguages.count() - 1);

	if (qApp) {
		qApp->removeTranslator(mTranslator);
		qApp->installTranslator(mTranslator);
	}

	emit languagesModified();
}

void LanguageManager::setLanguageFromFilename(const QString& filename)
{
	QString fn = filename;

	if (!fn.startsWith("Language/"))
		fn.prepend("Language/");

	for (int i = 0; i < mLanguages.size(); ++i) {
		if (mLanguages[i]->fileName() == fn)
			setCurrentLanguageIndex(i);
	}
}

QString LanguageManager::translate(const QString& section, const QString& key) const
{
	if (mCurrentLanguage != -1)
		return mLanguages.at(mCurrentLanguage)->translate(section, key);
	return QString();
}

void LanguageManager::refreshLanguageList(QString)
{
	qDebug() << "Reloading languages...";

	// Reset error flag.
	mError = false;
	mErrorString = QString();

	// Save current language realname
	QString languageName = currentLanguageRealName();

	// Did we lose the language before? Try to find it next time.
	if (languageName.isEmpty() && !mLostLanguage.isEmpty())
		languageName = mLostLanguage;

	// Delete currently loaded languages.
	foreach (Language* language, mLanguages)
		language->deleteLater();

	// Clear language list.
	mLanguages.clear();

	// Retrieve up-to-date language file list
	QString langDir = QString("Language/");
	QDir dir(langDir, "*.json", QDir::Name, QDir::Files | QDir::Readable);
	QStringList files = dir.entryList();

	// [Qt 4.7+] Reserve list space.
#if QT_VERSION >= 0x040700
	mLanguages.reserve(files.count());
#endif

	// Load languages.
	for (QString file : files) {
		// Add file to fswatch
		mFsWatcher->addPath(langDir + file);
		mLanguages.append(new Language(langDir + file, this));
	}

	// Determine if any loads failed.
	QList<Language*> loadFailures;
	for (Language* language : mLanguages) {
		if (language->error() != Language::Error::NoError) {
			loadFailures.append(language);
		}
	}

	bool lostLanguage = false;

	// Try to determine current language again.
	if (!languageName.isEmpty()) {
		mCurrentLanguage = -1;
		for (int i = 0; i < mLanguages.count(); ++i) {
			Language* language = mLanguages.at(i);

			if (language->realName() == languageName)
				mCurrentLanguage = i;
		}
		if (mCurrentLanguage == -1) {
			mLostLanguage = languageName;
			lostLanguage = true;
		}
	}

	// Ensure old id was in range.
	else
		mCurrentLanguage = qBound(-1, mCurrentLanguage, mLanguages.count() - 1);

	// Construct error message
	if (loadFailures.count() > 0) {
		mErrorString = QString("One or more language files failed to load:");

		foreach (Language* language, loadFailures) {
			mErrorString.append("\n\n").append(language->realName()).append(":\n");
			mErrorString.append(language->errorString());
		}

		if (lostLanguage) {
			mErrorString.append("\n\nAlso, the language you were using, ").append(languageName).append(", was lost.\n");
			mErrorString.append("It will be reloaded if found again.");
		}

		mError = true;
	} else if (lostLanguage) {
		mErrorString.append("The language you were using, ").append(languageName).append(", was lost.\n");
		mErrorString.append("It will be reloaded if found again.");

		mError = true;
	}

	if (!lostLanguage)
		mLostLanguage = QString();

	emit languagesModified();

	if (qApp) {
		qApp->removeTranslator(mTranslator);
		qApp->installTranslator(mTranslator);
	}
}
