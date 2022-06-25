#ifndef REPLAYDIALOG_H
#define REPLAYDIALOG_H

#include <QDialog>

namespace Ui {
class ReplayDialog;
}

class GameManager;
class Settings;

class ReplayDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ReplayDialog(QWidget *parent = 0,GameManager *gamemanager = 0);
    ~ReplayDialog();
    
private slots:
    void on_AddPushButton_clicked();
    void on_DeletePushButton_clicked();
    void on_ClearAllPushButton_clicked();
    void on_PlayPushButton_clicked();
    void on_PlaySelectedPushButton_clicked();
    void on_CloseButton_clicked();
    void on_FileDialog_Finished( const QStringList & selected );
private:
    Ui::ReplayDialog *ui;
    GameManager *gameManager;
};

#endif // REPLAYDIALOG_H
