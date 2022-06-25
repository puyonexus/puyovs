#include "inputwidget.h"
#include "inputdialog.h"
#include "ui_inputwidget.h"

InputWidget::InputWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InputWidget)
{
    ui->setupUi(this);
}

InputWidget::~InputWidget()
{
    delete ui;
}

QString InputWidget::text()
{
    return ui->InputLineEdit->text();
}

void InputWidget::setText(const QString &text)
{
    ui->InputLineEdit->setText(text);
}

void InputWidget::on_SetButton_clicked()
{
    InputDialog dialog;

    dialog.exec();

    if(!dialog.inputStr().isEmpty())
        ui->InputLineEdit->setText(dialog.inputStr());
}
