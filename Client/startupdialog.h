#pragma once

#include <QDesktopServices>
#include <QDialog>

namespace Ui {
class StartupDialog;
}

class StartupDialog : public QDialog {
	Q_OBJECT

public:
	explicit StartupDialog(QString motd, QWidget* parent = nullptr);
	~StartupDialog() override;

private slots:
	void on_LobbyButton_clicked();
	void on_TsuButton_clicked();
	void on_FeverButton_clicked();
	void on_textBrowser_anchorClicked(const QUrl& arg1);

private:
	Ui::StartupDialog* ui;
};
