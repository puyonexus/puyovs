#pragma once

#include <QDialog>

namespace Ui {
class CreateChatroomDialog;
}

class CreateChatroomDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit CreateChatroomDialog(QWidget *parent = 0);
    ~CreateChatroomDialog();

    QString name();
    QString description();

signals:
    void createChatroom(CreateChatroomDialog*);
    
private slots:
    void on_buttonBox_accepted();

private:
    Ui::CreateChatroomDialog *ui;
    QString mName;
    QString mDescription;
};
