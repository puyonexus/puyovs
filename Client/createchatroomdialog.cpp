#include "createchatroomdialog.h"
#include "ui_createchatroomdialog.h"

CreateChatroomDialog::CreateChatroomDialog(QWidget* parent)
	: QDialog(parent)
	, ui(new Ui::CreateChatroomDialog)
{
	ui->setupUi(this);
	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &CreateChatroomDialog::accept);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &CreateChatroomDialog::reject);
}

CreateChatroomDialog::~CreateChatroomDialog()
{
	delete ui;
}

QString CreateChatroomDialog::name()
{
	return mName;
}

QString CreateChatroomDialog::description()
{
	return mDescription;
}

void CreateChatroomDialog::on_buttonBox_accepted()
{
	mName = ui->CreateChatroomNameLineEdit->text();
	mDescription = ui->CreateChatroomDescriptionTextEdit->toPlainText();
	emit createChatroom(this);
}
