#ifndef OFFLINEDIALOG_H
#define OFFLINEDIALOG_H

#include <QDialog>
#include <QList>
#include <QString>
#include <QPair>

namespace Ui {
class OfflineDialog;
}

namespace ppvs {
class gameSettings;
}

class Settings;

class OfflineDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit OfflineDialog(ppvs::gameSettings *gameSettings, QWidget *parent = 0);
    ~OfflineDialog();
    
private slots:
    void on_PlayButton_clicked();
    void on_CancelButton_clicked();

    void on_ColorsCheckBox_toggled(bool checked);

private:
    Ui::OfflineDialog *ui;
    const QList<QPair<int, QString> > mModeList;
    ppvs::gameSettings *mGameSettings;
};

#endif // OFFLINEDIALOG_H
