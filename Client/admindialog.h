#pragma once

#include <QDialog>

namespace Ui {
class AdminDialog;
}

class NetClient;

class AdminDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AdminDialog(NetClient *client,QWidget *parent = 0);
    ~AdminDialog();
private slots:
    void adminResponseReceived(QString);
    void on_SendButton_clicked();

    void on_CommandComboBox_currentIndexChanged(const QString &arg1);

private:
    bool isAdminCommand();
    NetClient *mClient;
    Ui::AdminDialog *ui;
};
