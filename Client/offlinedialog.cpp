#include <QComboBox>
#include "offlinedialog.h"
#include "ui_offlinedialog.h"
#include "common.h"
#include "../Puyolib/game.h"
#include "../Puyolib/gameSettings.h"
#include "settings.h"

OfflineDialog::OfflineDialog(ppvs::gameSettings *gameSettings, QWidget *parent) :
    QDialog(parent), ui(new Ui::OfflineDialog), mModeList(getModeList()), mGameSettings(gameSettings)
{
    Settings &settings = pvsApp->settings();
    ui->setupUi(this);

    GameModeListIterator modeIterator(mModeList);
    while(modeIterator.hasNext())
    {
        GameModePair mode = modeIterator.next();
        ui->ModeComboBox->addItem(mode.second, mode.first);
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
    mGameSettings->rulesetInfo.setRules(ui->ModeComboBox->itemData(ui->ModeComboBox->currentIndex(), Qt::UserRole).toInt());

    if(ui->ColorsCheckBox->checkState() == Qt::Checked)
        mGameSettings->rulesetInfo.colors = ui->ColorsSpinBox->value();
    else
        mGameSettings->rulesetInfo.colors = 0;

    mGameSettings->rulesetInfo.Nplayers = (mGameSettings->rulesetInfo.rulesetType==ENDLESS) ? 1 : ui->PlayersSpinBox->value();
    mGameSettings->Nhumans = 1;
    mGameSettings->startWithCharacterSelect = true;
    mGameSettings->useCPUplayers = true;

    accept();
}

void OfflineDialog::on_CancelButton_clicked()
{
    reject();
}

void OfflineDialog::on_ColorsCheckBox_toggled(bool checked)
{
    ui->ColorsSpinBox->setEnabled(checked);
}
