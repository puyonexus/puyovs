#ifndef CHATTEXTENTRY_H
#define CHATTEXTENTRY_H

#include <QPlainTextEdit>

class ChatTextEntry : public QTextEdit
{
    Q_OBJECT
public:
    explicit ChatTextEntry(QWidget *parent = 0);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected:
    void keyPressEvent(QKeyEvent *e);
    void resizeEvent(QResizeEvent *e);

public slots:
    void fitText();

signals:
    void returnPressed();
    
private:
    int documentHeight;
    int fontLineHeight;
};

#endif // CHATTEXTENTRY_H
