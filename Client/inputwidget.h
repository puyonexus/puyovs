#pragma once

#include <QWidget>

namespace Ui {
class InputWidget;
}

class InputWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit InputWidget(QWidget *parent = 0);
    ~InputWidget();

    QString text();
    void setText(const QString &text);
    
private slots:
    void on_SetButton_clicked();

private:
    Ui::InputWidget *ui;
};
