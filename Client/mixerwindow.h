#pragma once

#include <QDialog>

namespace Ui {
class MixerWindow;
}

class MixerWindow : public QDialog
{
    Q_OBJECT
    
public:
    explicit MixerWindow(QWidget *parent = 0);
    ~MixerWindow();
    
private:
    Ui::MixerWindow *ui;
};
