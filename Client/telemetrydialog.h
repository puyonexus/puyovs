#pragma once

#include <QDialog>
#include <QDesktopServices>

namespace Ui {
class TelemetryDialog;
}

class LanguageManager;

class TelemetryDialog : public QDialog
{
    Q_OBJECT
    
public:
    TelemetryDialog(LanguageManager* lm, QWidget *parent = 0);
    ~TelemetryDialog();

    bool shouldShow();
    void load();
    void save();

private slots:
    void languageChanged(int index);
    void updateLanguagesBox();

private:
    Ui::TelemetryDialog *ui;
    LanguageManager* languageManager;

    bool shouldShowDialog = true;
};
