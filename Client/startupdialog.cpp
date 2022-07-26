#include "startupdialog.h"
#include "ui_startupdialog.h"

StartupDialog::StartupDialog(QString motd, QWidget* parent)
	: QDialog(parent)
	, ui(new Ui::StartupDialog)
{
	ui->setupUi(this);
	ui->textBrowser->setHtml(motd);
}

StartupDialog::~StartupDialog()
{
	delete ui;
}

void StartupDialog::on_LobbyButton_clicked()
{
	done(1);
}

void StartupDialog::on_TsuButton_clicked()
{
	done(2);
}

void StartupDialog::on_FeverButton_clicked()
{
	done(3);
}

void StartupDialog::on_textBrowser_anchorClicked(const QUrl& arg1)
{
	QDesktopServices::openUrl(arg1);
}
