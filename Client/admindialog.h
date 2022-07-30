#pragma once

#include <QDialog>

namespace Ui {
class AdminDialog;
}

class NetClient;

class AdminDialog final : public QDialog {
	Q_OBJECT

public:
	explicit AdminDialog(NetClient* client, QWidget* parent = nullptr);
	~AdminDialog() override;

	AdminDialog(const AdminDialog&) = delete;
	AdminDialog& operator=(const AdminDialog&) = delete;
	AdminDialog(AdminDialog&&) = delete;
	AdminDialog& operator=(AdminDialog&&) = delete;

private slots:
	void adminResponseReceived(QString) const;
	void on_SendButton_clicked() const;

	void on_CommandComboBox_currentIndexChanged(const QString& arg1) const;

private:
	[[nodiscard]] bool isAdminCommand() const;

	NetClient* mClient;
	Ui::AdminDialog* ui;
};
