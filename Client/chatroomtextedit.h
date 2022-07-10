#pragma once

#include <QPlainTextEdit>
#include <QMouseEvent>

class ChatroomTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    ChatroomTextEdit(QWidget *parent = 0);
    
signals:
    void anchorClicked(QString);

public slots:
    
protected:
    virtual void mousePressEvent( QMouseEvent * event );
    virtual void mouseMoveEvent( QMouseEvent * event );
};
