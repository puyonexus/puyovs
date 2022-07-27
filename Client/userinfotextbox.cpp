#include "userinfotextbox.h"

UserInfoTextBox::UserInfoTextBox(QWidget* parent)
	: QTextEdit(parent)
{
	setTextInteractionFlags(Qt::TextBrowserInteraction);
	setReadOnly(true);
}

QSize UserInfoTextBox::sizeHint() const
{
	return QSize(0, 100);
}
