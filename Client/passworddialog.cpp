#include "passworddialog.h"
#include "gamemanager.h"
#include "gamewidget.h"
#include "ui_passworddialog.h"

PasswordDialog::PasswordDialog(QWidget* parent, GameManager* gameManager, NetChannel chan, bool isSpectating)
	: QDialog(parent)
	, gameManager(gameManager)
	, chan(chan)
	, isSpectating(isSpectating)
	, ui(new Ui::PasswordDialog)
{
	ui->setupUi(this);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	ui->label_2->hide();
}

PasswordDialog::~PasswordDialog()
{
	delete ui;
}

void PasswordDialog::on_buttonBox_accepted()
{
	if (ui->lineEdit->text().isEmpty())
		return;

	QString passHash;
	QString myPassHash = getCryptographicHash(ui->lineEdit->text());
	QStringList items = chan.name.split(':');
	if (items.count() > 1) {
		passHash = items.at(1);
		if (passHash != myPassHash) {
			ui->label_2->show();
			return;
		}
	} else {
		reject();
		return;
	}
	GameWidget* existing = gameManager->findGame(chan.name);
	if (existing) {
		existing->raise();
		accept();
		return;
	}

	GameWidget* game = gameManager->createGame(chan.description, chan.name, isSpectating);

	if (!game) {
		reject();
		return;
	}

	game->show();
	game->raise();

	accept();
}

void PasswordDialog::on_buttonBox_rejected()
{
	reject();
}
