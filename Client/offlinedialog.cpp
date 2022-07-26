#include <QComboBox>
#include "offlinedialog.h"
#include "ui_offlinedialog.h"
#include "common.h"
#include "../Puyolib/Game.h"
#include "../Puyolib/GameSettings.h"
#include "settings.h"

OfflineDialog::OfflineDialog(ppvs::GameSettings* gameSettings, QWidget* parent) :
	QDialog(parent), ui(new Ui::OfflineDialog), mModeList(getModeList()), mGameSettings(gameSettings)
{
	Settings& settings = pvsApp->settings();
	ui->setupUi(this);

	GameModeListIterator modeIterator(mModeList);
	while (modeIterator.hasNext())
	{
		GameModePair mode = modeIterator.next();
		ui->ModeComboBox->addItem(mode.second, static_cast<int>(mode.first));
	}

	mGameSettings->playMusic = settings.boolean("launcher", "enablemusic", true);
	mGameSettings->playSound = settings.boolean("launcher", "enablesound", true);
	ui->ColorsSpinBox->setEnabled(ui->ColorsCheckBox->checkState() == Qt::Checked);
}

OfflineDialog::~OfflineDialog()
{
	delete ui;
}

void OfflineDialog::on_PlayButton_clicked()
{
	mGameSettings->ruleSetInfo.setRules(static_cast<ppvs::Rules>(ui->ModeComboBox->itemData(ui->ModeComboBox->currentIndex(), Qt::UserRole).toInt()));

	if (ui->ColorsCheckBox->checkState() == Qt::Checked)
		mGameSettings->ruleSetInfo.colors = ui->ColorsSpinBox->value();
	else
		mGameSettings->ruleSetInfo.colors = 0;

	mGameSettings->ruleSetInfo.numPlayers = mGameSettings->ruleSetInfo.ruleSetType == ppvs::Rules::ENDLESS ? 1 : ui->PlayersSpinBox->value();
	mGameSettings->numHumans = 1;
	mGameSettings->startWithCharacterSelect = true;
	mGameSettings->useCpuPlayers = true;

	accept();
}

void OfflineDialog::on_CancelButton_clicked()
{
	reject();
}

void OfflineDialog::on_ColorsCheckBox_toggled(bool checked) const
{
	ui->ColorsSpinBox->setEnabled(checked);
}
