#pragma once

#include <QDialog>

#include "netclient.h"

class GameManager;

namespace Ui {
class PasswordDialog;
}

namespace ppvs {
class game;
struct gameSettings;
struct rulesetInfo_t;
}

class PasswordDialog : public QDialog
{
	Q_OBJECT

public:
	explicit PasswordDialog(QWidget* parent = nullptr, GameManager* gameManager = nullptr, NetChannel chan = NetChannel(), bool isSpectating = false);
	~PasswordDialog() override;

private slots:
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();

private:
	Ui::PasswordDialog* ui;
	GameManager* gameManager;
	NetChannel chan;
	bool isSpectating;
};
