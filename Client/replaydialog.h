#pragma once

#include <QDialog>

namespace Ui {
class ReplayDialog;
}

class GameManager;
class Settings;

class ReplayDialog : public QDialog {
	Q_OBJECT

public:
	explicit ReplayDialog(QWidget* parent = nullptr, GameManager* gamemanager = nullptr);
	~ReplayDialog() override;

private slots:
	void on_AddPushButton_clicked();
	void on_DeletePushButton_clicked() const;
	void on_ClearAllPushButton_clicked() const;
	void on_PlayPushButton_clicked() const;
	void on_PlaySelectedPushButton_clicked() const;
	void on_CloseButton_clicked();
	void on_FileDialog_Finished(const QStringList& selected) const;

private:
	Ui::ReplayDialog* ui;
	GameManager* gameManager;
};
