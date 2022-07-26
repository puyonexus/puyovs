#include "chattextentry.h"
#include <QFontMetrics>

ChatTextEntry::ChatTextEntry(QWidget* parent)
	: QTextEdit(parent)
	, documentHeight(0)
{
	fontLineHeight = QFontMetrics(QFont()).lineSpacing();

	connect(this, SIGNAL(textChanged()), SLOT(fitText()));

	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
}

QSize ChatTextEntry::minimumSizeHint() const
{
	return {};
}

QSize ChatTextEntry::sizeHint() const
{
	return {
		0, contentsMargins().top() + contentsMargins().bottom() + qBound(fontLineHeight * 3, documentHeight, fontLineHeight * 8)
	};
}

void ChatTextEntry::keyPressEvent(QKeyEvent* e)
{
	if (!e->modifiers().testFlag(Qt::ShiftModifier) && (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return))
		emit returnPressed();
	else
		QTextEdit::keyPressEvent(e);
}

void ChatTextEntry::resizeEvent(QResizeEvent*)
{
	fitText();
#if defined(Q_OS_MAC)
	// Workaround for MacOSX resize.
	// See GameWidgetGL and GLWidget classes.
	QWidget::updateGeometry();
#endif
}

void ChatTextEntry::fitText()
{
	document()->setTextWidth(viewport()->width());
	documentHeight = document()->size().height();
#ifndef Q_OS_MAC
	// Workaround for MacOSX resize.
	// See GameWidgetGL and GLWidget classes.
	// We want to avoid resizeEvents... if it has not been actually resized!
	QWidget::updateGeometry();
#endif
}
