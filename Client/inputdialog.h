#pragma once

#include <QDialog>

namespace Ui {
class InputDialog;
}

namespace ilib {
class Driver;
}

class InputDialog : public QDialog
{
	Q_OBJECT

public:
	explicit InputDialog(QWidget* parent = nullptr);
	~InputDialog() override;

	QString inputStr();

protected:
	void keyPressEvent(QKeyEvent*) override;

private slots:
	void timeout();
	void process();

private:
	Ui::InputDialog* ui;
	ilib::Driver* drv;
	QString in;
};
