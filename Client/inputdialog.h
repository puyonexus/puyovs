#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QDialog>

namespace Ui {
class InputDialog;
}

namespace ilib {
class Driver;
}

class InputDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit InputDialog(QWidget *parent = 0);
    ~InputDialog();

    QString inputStr();

protected:
    void keyPressEvent(QKeyEvent *);

private slots:
    void timeout();
    void process();
    
private:
    Ui::InputDialog *ui;
    ilib::Driver *drv;
    QString in;
};

#endif // INPUTDIALOG_H
