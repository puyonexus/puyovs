#pragma once

#include <QPlainTextEdit>

class ChatTextEntry : public QTextEdit
{
	Q_OBJECT

public:
	explicit ChatTextEntry(QWidget* parent = nullptr);

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

protected:
	void keyPressEvent(QKeyEvent* e) override;
	void resizeEvent(QResizeEvent* e) override;

public slots:
	void fitText();

signals:
	void returnPressed();

private:
	int documentHeight;
	int fontLineHeight;
};
