#pragma once

#include <QDialog>

#include "netclient.h"

class GameManager;

namespace Ui {
class PasswordDialog;
}

namespace ppvs {
class Game;
struct GameSettings;
struct RuleSetInfo;
}

class PasswordDialog : public QDialog {
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
