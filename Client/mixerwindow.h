#pragma once

#include <QDialog>

namespace Ui {
class MixerWindow;
}

class MixerWindow : public QDialog {
	Q_OBJECT

public:
	explicit MixerWindow(QWidget* parent = nullptr);
	~MixerWindow() override;

private:
	Ui::MixerWindow* ui;
};
