#include "mixerwindow.h"
#include "ui_mixerwindow.h"

MixerWindow::MixerWindow(QWidget* parent)
	: QDialog(parent)
	, ui(new Ui::MixerWindow)
{
	ui->setupUi(this);
}

MixerWindow::~MixerWindow()
{
	delete ui;
}
