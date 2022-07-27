#pragma once

#include <QTextEdit>

class UserInfoTextBox : public QTextEdit {
	Q_OBJECT

public:
	explicit UserInfoTextBox(QWidget* parent = nullptr);

	QSize sizeHint() const override;
};
