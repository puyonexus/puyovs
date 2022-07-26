#include "searchdialog.h"
#include "netclient.h"
#include "ui_searchdialog.h"

SearchDialog::SearchDialog(QWidget* parent, NetClient* client)
	: QDialog(parent)
	, ui(new Ui::SearchDialog)
{
	ui->setupUi(this);
	searchState = 0;
	timerState = 0;
	connect(&timer, SIGNAL(timeout()), this, SLOT(on_TimeOut()));

	if (client) {
		connect(client, SIGNAL(searchResultReceived(QString)), this, SLOT(on_GetSearchResult(QString)));
	}
	netClient = client;
}

SearchDialog::~SearchDialog()
{
	delete ui;
}

void SearchDialog::on_pushButton_clicked()
{
	if (!ui->lineEdit->text().isEmpty() && netClient && netClient->isConnected()) {
		timer.start(2000);
		timerState = 1;
		searchState = 1;
		ui->pushButton->setEnabled(false);
		netClient->sendMessageToServer(SUBCHANNEL_SERVERREQ_SEARCH, ui->lineEdit->text());
	} else {
		ui->resultTextEdit->setText("Cannot search");
	}
}

void SearchDialog::on_GetSearchResult(QString str)
{
	searchState = 0;
	checkButton();
	ui->resultTextEdit->setText(str);
}

void SearchDialog::on_TimeOut()
{
	timerState = 0;
	checkButton();
}

void SearchDialog::checkButton() const
{
	if (searchState == 0 && timerState == 0) {
		ui->pushButton->setEnabled(true);
	}
}
