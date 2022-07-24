#include <QStyle>
#include <QDockWidget>
#include <QMenu>
#include <QCloseEvent>
#include "gamewidget.h"
#include "chatwindow.h"
#include "netclient.h"
#include "playlist.h"
#include "../Puyolib/game.h"

GameWidget::GameWidget(ppvs::game* game, NetChannelProxy* proxy, QWidget* parent)
	: QMainWindow(parent), mGame(game)
{
	if (proxy)
		proxy->setParent(this);
	mProxy = proxy;

	mMusicPlaying = mFeverMusic = false;

	mChatDockWidget = nullptr;
	mChatWindow = nullptr;
	mToggleChat = nullptr;

	if (mGame->network)
	{
		mChatDockWidget = new QDockWidget(tr("Chat Window"));
		mChatWindow = new ChatWindow(proxy, game, this);
		mChatDockWidget->setWidget(mChatWindow);
		mChatDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
#if defined(Q_OS_MAC)
		// Fix for MacOSX resize. background-color would be inherited from GameWidgetGL otherwise.
		// See GameWidgetGL and GLWidget classes.
		mChatWindow->setStyleSheet(
			"QWidget#ChatTab { background-color: lightgrey;} "
			"QWidget#UsersTab { background-color: lightgrey;} "
			"QWidget#SettingsTab { background-color: lightgrey;} "
			"QTabWidget#tabWidget { background-color: lightgrey;}");
#endif

		mToggleChat = new QAction("Chat Window", this);
		mToggleChat->setCheckable(true);

		addDockWidget(Qt::LeftDockWidgetArea, mChatDockWidget, Qt::Vertical);
		connect(mChatDockWidget, SIGNAL(visibilityChanged(bool)), SLOT(chatVisibilityChanged(bool)));
	}
	mFullScreen = new QAction("FullScreen", this);
	mFullScreen->setCheckable(true);
	connect(mToggleChat, SIGNAL(toggled(bool)), SLOT(chatToggled(bool)));
	connect(mFullScreen, SIGNAL(toggled(bool)), SLOT(fullScreenToggled(bool)));

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), SLOT(contextMenu(QPoint)));
	if (mGame->settings->recording == PVS_REPLAYING)
	{
		QTimer* timer = new QTimer(this);
		connect(timer, SIGNAL(timeout()), this, SLOT(showReplayTimer()));
		timer->start(20);
	}
	setWindowIcon(QIcon(":/icons/redpuyo.ico"));
	updateGeometry();
	moveOnce = false;
	move(parent->pos());
	parentPos = parent->pos() + QPoint(parent->width(), 0);
	setParent(nullptr);
}

GameWidget::~GameWidget()
{
	if (isActiveWindow())
		pvsApp->setMusicMode(PVSApplication::MusicPause);

	emit exiting(this);
}

QSize GameWidget::sizeHint() const
{
	int width = 640, height = 480;

	if (mChatDockWidget && !mChatDockWidget->isFloating())
		width += mChatDockWidget->width() + style()->pixelMetric(QStyle::PM_SplitterWidth);

	return QSize(width, height);
}

ppvs::game* GameWidget::game() const
{
	return mGame;
}

ChatWindow* GameWidget::chatWindow() const
{
	return mChatWindow;
}

void GameWidget::setControls(InputCondition up, InputCondition down, InputCondition left, InputCondition right, InputCondition a, InputCondition b, InputCondition start)
{
	mControls[0] = up;
	mControls[1] = down;
	mControls[2] = left;
	mControls[3] = right;
	mControls[4] = a;
	mControls[5] = b;
	mControls[6] = start;
}

void GameWidget::replaceProxy(NetChannelProxy* newproxy)
{
	delete mProxy;
	if (mChatWindow)
		mChatWindow->replaceProxy(newproxy);
	mProxy = newproxy;
	newproxy->setParent(this);
}

void GameWidget::playMusic()
{
	mFeverMusic = mGame->isFever();
	pvsApp->setMusicMode(mFeverMusic ? PVSApplication::MusicFever : PVSApplication::MusicNormal);
}


void GameWidget::contextMenu(const QPoint& pos)
{
	QList <QAction*> actions;

	if (mToggleChat)
		actions.append(mToggleChat);
	if (mFullScreen)
		actions.append(mFullScreen);

	QMenu* menu = new QMenu(this);
	menu->addActions(actions);
	menu->popup(mapToGlobal(pos));
	menu->show();
}

void GameWidget::chatVisibilityChanged(bool visible) const
{
	mToggleChat->setChecked(visible);
}

void GameWidget::chatToggled(bool visible)
{
	mChatDockWidget->setVisible(visible);
}

void GameWidget::fullScreenToggled(bool visible)
{
	mFullScreen->setChecked(visible);
	if (visible)
		showFullScreen();
	else
		showNormal();
}

void GameWidget::showReplayTimer()
{
	if (mGame->settings->recording != PVS_REPLAYING)
		return;

	int& matchTimer = mGame->data->matchTimer;
	int ms = matchTimer * 2 % 100;
	int s = matchTimer / 60 % 60;
	int m = matchTimer / 60 / 60 % 3600;
	QString timeString = QString("Replay [%1:%2:%3]").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')).arg(ms, 2, 10, QChar('0'));
	if (mGame->replayTimer > 0)
		setWindowTitle("Replay [00:00:00]");
	else
	{
		switch (mGame->replayState)
		{
		case REPLAYSTATE_PAUSED:
			setWindowTitle(timeString + " Paused");
			break;
		case REPLAYSTATE_FASTFORWARD:
			setWindowTitle(timeString + " x2");
			break;
		case REPLAYSTATE_FASTFORWARDX4:
			setWindowTitle(timeString + " x4");
			break;
		default:
		case REPLAYSTATE_NORMAL:
			setWindowTitle(timeString);
			break;
		}
	}
}

bool GameWidget::event(QEvent* event)
{
	switch (event->type())
	{
	case QEvent::WindowActivate:
		if (mGame)
		{
			mGame->setWindowFocus(true);
			mMusicPlaying = true;
			mGame->currentVolumeFever -= 1;
			mGame->currentVolumeNormal -= 1;
		}
		if (mMusicPlaying)
			playMusic();
		break;
	case QEvent::WindowDeactivate:
		if (pvsApp->activeWindow() && pvsApp->activeWindow()->inherits("GameWidget") || pvsApp->activeWindow() == nullptr)
		{
			if (mGame)
				mGame->setWindowFocus(false);
		}
		break;
	}

	return QMainWindow::event(event);
}

void GameWidget::showEvent(QShowEvent*)
{
	if (!moveOnce)
	{
		moveOnce = true;
		move(parentPos - QPoint(width(), 0));
		raise();
		activateWindow();
	}
	if (mChatDockWidget)
		mChatDockWidget->show();
}

void GameWidget::closeEvent(QCloseEvent* event)
{
	if (mGame && mGame->settings->rankedMatch
		&& mGame->countBoundPlayers() > 1)
	{
		// Do not close
		event->ignore();
	}
	else
	{
		QMainWindow::closeEvent(event);
	}
}


void GameWidget::setupMusic(int evt)
{
	bool focused = isActiveWindow();
	switch (ppvs::fmusicevent(evt))
	{
	case ppvs::music_can_stop:
		if (!pvsApp->settings().boolean("music", "stopmusicafterround", false))
			return;

		mMusicPlaying = false;
		mFeverMusic = false;

		if (focused)
		{
			if (pvsApp->settings().boolean("music", "looponce", true))
				pvsApp->setMusicMode(PVSApplication::MusicPause);
			else
				pvsApp->setMusicMode(PVSApplication::MusicOff);
		}
		break;
	case ppvs::music_continue:
		mMusicPlaying = true;

		if (focused) pvsApp->setMusicMode(PVSApplication::MusicNormal, pvsApp->settings().boolean("music", "advance", true));
		break;
	case ppvs::music_enter_fever:
		if (focused) pvsApp->setMusicMode(PVSApplication::MusicFever, true);

		mFeverMusic = true;
		break;
	case ppvs::music_exit_fever:
		if (focused) pvsApp->setMusicMode(PVSApplication::MusicNormal, false);

		mFeverMusic = false;
		break;
	}
}

void GameWidget::changeMusicVolume(float volume, bool fever)
{
	pvsApp->setMusicVolume(volume, fever);
}
