#pragma once

#include <QDialog>

#include "netclient.h"

class GameManager;

namespace Ui {
class MainWindow;
}

namespace ppvs {
class game;
struct gameSettings;
struct rulesetInfo_t;
}

namespace Ui {
class PasswordDialog;
}

class PasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PasswordDialog(QWidget *parent = 0,GameManager *gameManager = 0,NetChannel chan = NetChannel(), bool isSpectating = false);
    ~PasswordDialog();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::PasswordDialog *ui;
    GameManager *gameManager;
    NetChannel chan;
    bool isSpectating;
};
