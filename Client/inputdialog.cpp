#include "inputdialog.h"
#include "common.h"
#include "ui_inputdialog.h"

#include <ilib/driver.h>

#include <QTimer>
#include <math.h>

InputDialog::InputDialog(QWidget* parent)
	: QDialog(parent)
	, ui(new Ui::InputDialog)
{
	ui->setupUi(this);

	drv = ilib::getDriver();
	drv->enableEvents();

	ilib::InputEvent e;

	// Drain events
	while (drv->getEvent(&e))
		;

	QTimer* processTimer = new QTimer(this);
	connect(processTimer, SIGNAL(timeout()), SLOT(process()));
	processTimer->start(100);

	QTimer::singleShot(1000 * 5, this, SLOT(timeout()));
}

InputDialog::~InputDialog()
{
	drv->disableEvents();

	ilib::InputEvent e;

	// Drain events
	drv->process();
	while (drv->getEvent(&e))
		;

	delete ui;
}

QString InputDialog::inputStr()
{
	return in;
}

void InputDialog::keyPressEvent(QKeyEvent* e)
{
	in = InputCondition(e).toString();
	close();
}

void InputDialog::timeout()
{
	in = QString();
	close();
}

void InputDialog::process()
{
	drv->process();

	ilib::InputEvent e {};
	while (drv->getEvent(&e)) {
		if (e.type == ilib::InputEvent::Type::AxisEvent && fabs(e.axis.value) < 0.5)
			continue;

		in = InputCondition(e).toString();
		close();
	}
}
