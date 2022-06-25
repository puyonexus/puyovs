#ifndef INPUTWIDGET_H
#define INPUTWIDGET_H

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

#endif // INPUTWIDGET_H
