#include <QFileDialog>
#include "replaydialog.h"
#include "ui_replaydialog.h"
#include "gamemanager.h"
#include "settings.h"
#include "gamewidget.h"

ReplayDialog::ReplayDialog(QWidget *parent, GameManager *gameManager)
    : QDialog(parent), gameManager(gameManager),
    ui(new Ui::ReplayDialog)
{
    ui->setupUi(this);

    Settings &settings = pvsApp->settings();
    if (settings.integer("launcher","hidenames",0)==1)
        ui->ReplayOption1RadioButton->setChecked(true);
    else if (settings.integer("launcher","hidenames",0)==2)
        ui->ReplayOption2RadioButton->setChecked(true);
}

ReplayDialog::~ReplayDialog()
{
    delete ui;
}

void ReplayDialog::on_AddPushButton_clicked()
{
    QFileDialog *dialog=new QFileDialog(this,"Open replay files","User/Replays/","PuyoVS replay files (*.rvs)");
    dialog->setFileMode(QFileDialog::ExistingFiles);

    connect(dialog,SIGNAL(filesSelected(const QStringList&)),this,SLOT(on_FileDialog_Finished(const QStringList&)));

    dialog->show();

}
void ReplayDialog::on_DeletePushButton_clicked()
{
    QList<QListWidgetItem*> items=ui->PlaylistListWidget->selectedItems();
    if(!items.isEmpty())
        qDeleteAll(items);
}

void ReplayDialog::on_ClearAllPushButton_clicked()
{
    ui->PlaylistListWidget->clear();
}

void ReplayDialog::on_PlayPushButton_clicked()
{
    Settings &settings = pvsApp->settings();
    int hide=ui->ReplayOption1RadioButton->isChecked()*1+ui->ReplayOption2RadioButton->isChecked()*2;
    settings.setInteger("launcher","hidenames",hide);
    ppvs::gameSettings *gs=new ppvs::gameSettings();

    for (int i=0;i<ui->PlaylistListWidget->count();i++)
        gs->replayPlayList.push_back(ui->PlaylistListWidget->item(i)->text().toStdString());

    GameWidget *gw=gameManager->createGame(gs,QString(),true,true);
    gw->show();
    gw->raise();
}

void ReplayDialog::on_PlaySelectedPushButton_clicked()
{
    Settings &settings = pvsApp->settings();
    int hide=ui->ReplayOption1RadioButton->isChecked()*1+ui->ReplayOption2RadioButton->isChecked()*2;
    settings.setInteger("launcher","hidenames",hide);
    ppvs::gameSettings *gs=new ppvs::gameSettings();
    QList<QListWidgetItem*> selected= ui->PlaylistListWidget->selectedItems();

    for (int i=0;i<selected.count();i++)
        gs->replayPlayList.push_back(selected.at(i)->text().toStdString());

    GameWidget *gw=gameManager->createGame(gs,QString(),true,true);
    gw->show();
    gw->raise();
}

void ReplayDialog::on_CloseButton_clicked()
{
    close();
}
void ReplayDialog::on_FileDialog_Finished(const QStringList & selected)
{
    ui->PlaylistListWidget->addItems(selected);
}
