#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class SearchDialog;
}

class NetClient;

class SearchDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SearchDialog(QWidget *parent = 0,NetClient* client=0);
    ~SearchDialog();
    
private slots:
    void on_pushButton_clicked();
    void on_GetSearchResult(QString);
    void on_TimeOut();
private:
    Ui::SearchDialog *ui;
    NetClient *netClient;
    QTimer timer;
    int timerState;
    int searchState;
    void checkButton();
};

#endif // SEARCHDIALOG_H
