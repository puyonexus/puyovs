#pragma once

#include <QDialog>
#include <QList>
#include <QPair>
#include <QString>

namespace Ui {
class OfflineDialog;
}

namespace ppvs {
enum class Rules : int;
struct GameSettings;
}

class Settings;

class OfflineDialog : public QDialog {
	Q_OBJECT

public:
	explicit OfflineDialog(ppvs::GameSettings* gameSettings, QWidget* parent = nullptr);
	~OfflineDialog() override;

private slots:
	void on_PlayButton_clicked();
	void on_CancelButton_clicked();

	void on_ColorsCheckBox_toggled(bool checked) const;

private:
	Ui::OfflineDialog* ui;
	const QList<QPair<ppvs::Rules, QString>> mModeList;
	ppvs::GameSettings* mGameSettings;
};
