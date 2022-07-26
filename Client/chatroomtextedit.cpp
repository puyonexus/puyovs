#include "chatroomtextedit.h"

ChatroomTextEdit::ChatroomTextEdit(QWidget* parent)
	: QPlainTextEdit(parent)
{
	setMouseTracking(true);
}

void ChatroomTextEdit::mousePressEvent(QMouseEvent* event)
{
	QString url = anchorAt(event->pos());
	if (!url.isEmpty())
		emit anchorClicked(url);
	else
		QPlainTextEdit::mousePressEvent(event);
}

void ChatroomTextEdit::mouseMoveEvent(QMouseEvent* event)
{
	if (anchorAt(event->pos()).isEmpty())
		viewport()->setCursor(Qt::IBeamCursor);
	else
		viewport()->setCursor(Qt::PointingHandCursor);
	QPlainTextEdit::mouseMoveEvent(event);
}
