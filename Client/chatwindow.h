#pragma once

#include <QWidget>

namespace Ui {
class ChatWindow;
}

namespace ppvs {
class game;
}

class NetChannelProxy;
class GameWidget;

class ChatWindow : public QWidget
{
	Q_OBJECT

public:
	explicit ChatWindow(NetChannelProxy* proxy, ppvs::game* game, GameWidget* widget, QWidget* parent = nullptr);
	~ChatWindow() override;

	void setQuickChat(bool);
	void statusMessage(const QString& message) const;
	void chatMessage(const QString& nick, const QString& message) const;
	void appendLine(QString html) const;
	void addUser(QString name, QString type) const;
	void removeUser(QString name) const;
	void replaceProxy(NetChannelProxy* newproxy);
	void clearUsers() const;
	void setChatEnabled(bool) const;

public slots:
	void on_EntryTextEdit_returnPressed();

private slots:
	void on_ShowNameCheckBox_stateChanged(int arg1) const;
	void on_PlaySoundCheckBox_stateChanged(int arg1) const;
	void on_PlayMusicCheckBox_stateChanged(int arg1) const;
	void on_EntryTextEdit_textChanged() const;

signals:
	void messageSubmitted(const QString& message);

protected:
	void focusInEvent(QFocusEvent*) override;

private:
	NetChannelProxy* mProxy;
	ppvs::game* mGame;
	GameWidget* mGameWidget;
	bool quickchat;
	Ui::ChatWindow* ui;
};
