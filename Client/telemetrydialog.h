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
	TelemetryDialog(LanguageManager* lm, QWidget* parent = nullptr);
	~TelemetryDialog() override;

	bool shouldShow() const;
	void load();
	void save() const;

private slots:
	void languageChanged(int index) const;
	void updateLanguagesBox();

private:
	Ui::TelemetryDialog* ui;
	LanguageManager* languageManager;

	bool shouldShowDialog = true;
};
