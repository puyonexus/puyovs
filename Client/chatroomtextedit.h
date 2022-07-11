#pragma once

#include <QPlainTextEdit>
#include <QMouseEvent>

class ChatroomTextEdit : public QPlainTextEdit
{
	Q_OBJECT

public:
	ChatroomTextEdit(QWidget* parent = nullptr);

signals:
	void anchorClicked(QString);
	
protected:
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
};
