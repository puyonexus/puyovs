#include "admindialog.h"
#include "netclient.h"
#include "ui_admindialog.h"
#include <QLineEdit>

AdminDialog::AdminDialog(NetClient* client, QWidget* parent)
	: QDialog(parent)
	, mClient(client)
	, ui(new Ui::AdminDialog)
{
	ui->setupUi(this);

	connect(mClient, &NetClient::adminResponseReceived, this, &AdminDialog::adminResponseReceived);
	ui->ArgumentLabel->setText("Arguments:\n username \nRetrieve the ip address the user used to register.");
}

AdminDialog::~AdminDialog()
{
	delete ui;
}

void AdminDialog::adminResponseReceived(QString message) const
{
	ui->AdminTextEdit->moveCursor(QTextCursor::End);
	ui->AdminTextEdit->insertPlainText(message + "\n");
	ui->AdminTextEdit->moveCursor(QTextCursor::End);
}

void AdminDialog::on_SendButton_clicked() const
{
	uchar subChannel;
	if (isAdminCommand()) {
		subChannel = SUBCHANNEL_SERVERREQ_ADMIN;
	} else {
		subChannel = SUBCHANNEL_SERVERREQ_MODERATOR;
	}
	if (ui->ArgumentsEdit->toPlainText().isEmpty()) {
		mClient->sendMessageToServer(subChannel, ui->CommandComboBox->lineEdit()->text());
	} else {
		mClient->sendMessageToServer(subChannel, ui->CommandComboBox->lineEdit()->text() + "|" + ui->ArgumentsEdit->toPlainText());
	}
	ui->ArgumentsEdit->clear();
}

bool AdminDialog::isAdminCommand() const
{
	const QString command = ui->CommandComboBox->lineEdit()->text();
	return command == "newmod" || command == "newadmin" || command == "demote" || command == "delete";
}

void AdminDialog::on_CommandComboBox_currentIndexChanged(const QString& arg1) const
{
	if (arg1 == "kick") {
		ui->ArgumentLabel->setText("Arguments:\n username|reason \nKick a user from the server.");
	} else if (arg1 == "ban") {
		ui->ArgumentLabel->setText("Arguments:\n username|reason \nBan the user\'s IP address.");
	} else if (arg1 == "banip") {
		ui->ArgumentLabel->setText("Arguments:\n ip address|reason \nBan a specific IP address. Use * as wildcard.");
	} else if (arg1 == "unban") {
		ui->ArgumentLabel->setText("Arguments:\n ip address \nUnban an IP address.");
	} else if (arg1 == "lock") {
		ui->ArgumentLabel->setText("Arguments:\n username \nLock a user\'s account.");
	} else if (arg1 == "unlock") {
		ui->ArgumentLabel->setText("Arguments:\n username \nUnlock a user\'s account.");
	} else if (arg1 == "banlist") {
		ui->ArgumentLabel->setText("Arguments:\n - \nView the list of banned IP addresses.");
	} else if (arg1 == "adminlist") {
		ui->ArgumentLabel->setText("Arguments:\n - \nView who is admin and moderator.");
	} else if (arg1 == "lockedlist") {
		ui->ArgumentLabel->setText("Arguments:\n - \nView the list of locked accounts.");
	} else if (arg1 == "newmod") {
		ui->ArgumentLabel->setText("Arguments:\n username \nAdd user as moderator (command for admins only).");
	} else if (arg1 == "newadmin") {
		ui->ArgumentLabel->setText("Arguments:\n username \nAdd user as administrator (command for admins only).");
	} else if (arg1 == "demote") {
		ui->ArgumentLabel->setText("Arguments:\n username \nRemove mod/admin status (command for admins only).");
	} else if (arg1 == "delete") {
		ui->ArgumentLabel->setText("Arguments:\n username \nDelete an account (command for admins only).");
	} else if (arg1 == "getip") {
		ui->ArgumentLabel->setText("Arguments:\n username \nRetrieve the ip address the user used to register.");
	} else if (arg1 == "maxwins") {
		ui->ArgumentLabel->setText("Arguments:\n number of wins \nThe number of wins for a ranked match to end.");
	} else if (arg1 == "motd") {
		ui->ArgumentLabel->setText("Arguments:\n html code \nSet the message of the day. Enter html code.");
	} else if (arg1 == "getmotd") {
		ui->ArgumentLabel->setText("Arguments:\n 0 \nRetrieve the message of the day in html (for easy editing)");
	} else {
		ui->ArgumentLabel->setText("Arguments");
	}
}
