#pragma once

#include <QWidget>

namespace Ui {
class InputWidget;
}

class InputWidget : public QWidget {
	Q_OBJECT

public:
	explicit InputWidget(QWidget* parent = nullptr);
	~InputWidget() override;

	QString text() const;
	void setText(const QString& text) const;

private slots:
	void on_SetButton_clicked() const;

private:
	Ui::InputWidget* ui;
};
