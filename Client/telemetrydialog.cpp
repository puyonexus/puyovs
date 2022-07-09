#include "telemetrydialog.h"
#include "ui_telemetrydialog.h"
#include "language.h"
#include "settings.h"
#include "pvsapplication.h"

TelemetryDialog::TelemetryDialog(LanguageManager* lm, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TelemetryDialog),
	languageManager(lm)
{
    ui->setupUi(this);
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(ui->languageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(languageChanged(int)));
    connect(languageManager, SIGNAL(languagesModified()), this, SLOT(updateLanguagesBox()));

    updateLanguagesBox();
    load();
}

TelemetryDialog::~TelemetryDialog()
{
    delete ui;
}

bool TelemetryDialog::shouldShow()
{
    return shouldShowDialog;
}

void TelemetryDialog::load()
{
    Settings& settings = pvsApp->settings();
    ui->languageBox->setCurrentIndex(languageManager->currentLanguageIndex());

    shouldShowDialog = settings.boolean("telemetry", "showdialog", true);
	auto collectDebugData = settings.boolean("telemetry", "collectdebugdata", false);
	auto collectUsageData = settings.boolean("telemetry", "collectusagedata", false);

    ui->allowCrashDumpsCheckBox->setChecked(collectDebugData);
    ui->allowUsageStatisticsCheckBox->setChecked(collectUsageData);
}

void TelemetryDialog::save()
{
    auto collectDebugData = ui->allowCrashDumpsCheckBox->isChecked();
    auto collectUsageData = ui->allowUsageStatisticsCheckBox->isChecked();

    Settings& settings = pvsApp->settings();

    settings.setBoolean("telemetry", "showdialog", false);
    settings.setBoolean("telemetry", "collectdebugdata", collectDebugData);
    settings.setBoolean("telemetry", "collectusagedata", collectUsageData);

    QString langFn = languageManager->currentLanguageFileName();
    if(langFn.startsWith("Language/"))
        langFn.remove(0, 9);
    settings.setString("launcher", "language", langFn);

    settings.save();
}

void TelemetryDialog::languageChanged(int index)
{
    if (languageManager->currentLanguageIndex() != index)
        languageManager->setCurrentLanguageIndex(ui->languageBox->currentIndex());
}

void TelemetryDialog::updateLanguagesBox()
{
    ui->languageBox->blockSignals(true);
    ui->languageBox->clear();
    int languageCount = languageManager->languageCount();

    for (int i = 0; i < languageCount; ++i)
        ui->languageBox->addItem(languageManager->language(i)->realName(), i);

    int language = languageManager->currentLanguageIndex();
    if (language != -1 && language != ui->languageBox->currentIndex())
        ui->languageBox->setCurrentIndex(language);
    ui->languageBox->blockSignals(false);
    ui->retranslateUi(this);
}
