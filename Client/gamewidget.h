#pragma once

#include <QMainWindow>
#include <QTimer>
#include "common.h"

namespace ppvs {
class game;
}

class ChatWindow;
class NetChannelProxy;
class MusicPlayer;

class GameWidget : public QMainWindow
{
	Q_OBJECT
public:
	explicit GameWidget(ppvs::game* game, NetChannelProxy* proxy, QWidget* parent = nullptr);
	~GameWidget() override;

	QSize sizeHint() const override;

	ppvs::game* game() const;
	ChatWindow* chatWindow() const;

	void setControls(InputCondition up, InputCondition down,
		InputCondition left, InputCondition right,
		InputCondition a, InputCondition b, InputCondition start);
	void replaceProxy(NetChannelProxy* newproxy);
	void playMusic();
signals:
	void exiting(GameWidget*);

public slots:
	virtual void initialize() = 0;
	virtual void process() = 0;
	void setupMusic(int evt);
	void changeMusicVolume(float volume, bool fever);
	void contextMenu(const QPoint& pos);
	void chatVisibilityChanged(bool visible) const;
	void chatToggled(bool visible);
	void fullScreenToggled(bool visible);
	void showReplayTimer();

protected:
	bool event(QEvent* event) override;
	void showEvent(QShowEvent*) override;
	void closeEvent(QCloseEvent* event) override;

	bool mMusicPlaying, mFeverMusic;
	ppvs::game* mGame;
	QDockWidget* mChatDockWidget;
	ChatWindow* mChatWindow;
	QAction* mToggleChat;
	QAction* mFullScreen;
	InputCondition mControls[7];
	NetChannelProxy* mProxy;
	bool moveOnce;
	QPoint parentPos;
};
