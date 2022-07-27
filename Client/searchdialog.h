#pragma once

#include <QDialog>
#include <QTimer>

namespace Ui {
class SearchDialog;
}

class NetClient;

class SearchDialog : public QDialog {
	Q_OBJECT

public:
	explicit SearchDialog(QWidget* parent = nullptr, NetClient* client = nullptr);
	~SearchDialog() override;

private slots:
	void on_pushButton_clicked();
	void on_GetSearchResult(QString);
	void on_TimeOut();

private:
	Ui::SearchDialog* ui;
	NetClient* netClient;
	QTimer timer;
	int timerState;
	int searchState;
	void checkButton() const;
};
