#pragma once

#include <QDialog>
#include <QList>
#include <QString>
#include <QPair>

namespace Ui {
	class OfflineDialog;
}

namespace ppvs {
	struct GameSettings;
}

class Settings;

class OfflineDialog : public QDialog
{
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
	const QList<QPair<int, QString>> mModeList;
	ppvs::GameSettings* mGameSettings;
};
