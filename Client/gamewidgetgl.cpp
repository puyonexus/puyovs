// QtOpenGL and etc.
#include <QGLWidget>

// Audiolib and Inputlib
#include <alib/audiolib.h>
#include <alib/stream.h>
#include <ilib/driver.h>
#include <ilib/inputevent.h>

// Puyolib hook-back
#include "../Puyolib/Frontend.h"
#include "../Puyolib/Game.h"
#include "../Puyolib/global.h"
#include "../PVS_ENet/PVS_Client.h"

#include "gamewidgetgl.h"
#include "common.h"
#include "glextensions.h"
#include "glmanager.h"
#include "frontendgl.h"
#include "chatwindow.h"

class GLWidget : public QGLWidget { /* Just a QGLWidget that resizes the viewport with the widget. */
#if defined(Q_OS_MAC)
// Workaround for MacOSX resize. 
public: explicit GLWidget(QWidget* parent = 0, QGLWidget* shareWidget = 0) : QGLWidget(parent, shareWidget) {
	guard = false;
}
private:
	bool guard;
protected: void resizeEvent(QResizeEvent* e)
#else
public: explicit GLWidget(QWidget* parent = nullptr, QGLWidget* shareWidget = nullptr) : QGLWidget(parent, shareWidget) {}
protected: void resizeEvent(QResizeEvent*) override
#endif
{
#if defined(Q_OS_MAC)
	// Workaround for MacOSX resize. 
	if (guard)
		return;
#endif
	makeCurrent();
#if QT_VERSION >= 0x050600
	int scale = devicePixelRatio();
#else
	int scale = 1;
#endif
	int windowWidth = width() * scale;
	int windowHeight = height() * scale;
	float w = float(windowWidth);
	float h = float(windowHeight);
	float ratio = h / w; // =0.75
	if (ratio > 0.75f)
		h = 0.75f * w;
	else
		w = h / 0.75f;
	int diffh = windowHeight - int(h);
	int diffw = windowWidth - int(w);

	// Snap to 640 x 480
	if (abs(windowWidth - 640) < 32 && abs(windowHeight) < 32)
	{
		w = 640.0f;
		h = 480.0f;
		diffh = windowHeight - int(h);
		diffw = windowWidth - int(w);
	}

#if defined(Q_OS_MAC)
	/* On MacOSX, this Qt commit messes with glViewport() :
	https://qt.gitorious.org/qt/qtbase/commit/1caa0c023f4fa60446094e53f22ee79771130e2f?format=patch
	glViewport() is overridden by Cocoa, which will call it with
	glViewport(0,0,width(),height()), bypassing ratio correction.

	This fixes aspect ratio, but induces a slight flicker on resize events.
	And those events are triggered by entering text in the chat window...
	*/
	setGeometry(x() + diffw / 2, y() + diffh / 2, int(w), int(h));
#endif
	glViewport(diffw / 2, diffh / 2, int(w), int(h));
#if defined(Q_OS_MAC)
	// Workaround for MacOSX resize. 
	guard = false; // Prevents reentry from setGeometry()
#endif
}
};

struct GameWidgetGLPriv
{
	GLWidget* gl;
	GLExtensions ext;
	GameAudio* audio;
	ilib::Driver* inputDriver;
	ppvs::FeInput inputState;
	quint64 nextFrame;
	bool ready;
};

GameWidgetGL::GameWidgetGL(ppvs::Game* game, NetChannelProxy* proxy, GameAudio* audio, QWidget* parent) :
	GameWidget(game, proxy, parent), d(new GameWidgetGLPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);
	d->ready = false;
	d->audio = audio;
	d->inputDriver = ilib::getDriver();
	d->inputDriver->enableEvents();
	GlobalGLInit();

	if (glMan->globalWidget())
		d->gl = new GLWidget(this, glMan->globalWidget());
	else
		d->gl = new GLWidget(this);

	loadExtensions(d->gl->context(), &d->ext);

	setCentralWidget(d->gl);
	setFocusProxy(d->gl);
	d->gl->setFocusPolicy(Qt::StrongFocus);

	memset(&d->inputState, 0, sizeof(ppvs::FeInput));

#if defined(Q_OS_MAC)
	// Workaround for MacOSX resize. 
	QPalette p;
	setBackgroundRole(QPalette::Window);
	setAutoFillBackground(true);
	p.setColor(QPalette::Window, QColor(0, 0, 0, 0));
	setPalette(p);
#endif

	initialize();
}

GameWidgetGL::~GameWidgetGL()
{
	if (!mGame->m_channelName.empty())
	{
		// Before leaving the channel, check if you're the last player
		if (mGame->countBoundPlayers() < 2 && mGame->m_network->connected)
			mGame->m_network->requestChannelDescription(mGame->m_channelName, std::string(""));
		mGame->m_network->leaveChannel(mGame->m_channelName);
	}

	d->inputDriver->disableEvents();

	delete d->gl;
	delete d;

	delete mGame;
}

void GameWidgetGL::keyPressEvent(QKeyEvent* k)
{
	InputCondition::MatchResult r;
#define HANDLEKEY(i, button) r=mControls[i].match(k);if(r==InputCondition::MatchDown)d->inputState. button = true
	HANDLEKEY(0, up);
	HANDLEKEY(1, down);
	HANDLEKEY(2, left);
	HANDLEKEY(3, right);
	HANDLEKEY(4, a);
	HANDLEKEY(5, b);
	HANDLEKEY(6, start);
#undef HANDLEKEY

	// I'm putting this here for the moment.
	// This is not part of Puyolib, because it may be hard
	// to notify the client when a replay is changed
	if (mGame->m_settings->recording == ppvs::RecordState::REPLAYING)
	{
		if (k->key() == Qt::Key_Left)
			mGame->previousReplay();
		else if (k->key() == Qt::Key_Right)
			mGame->nextReplay();
	}

	// Pressing tab opens chatwindow
	if (chatWindow() && k->key() == Qt::Key_Space)
	{
		chatWindow()->setQuickChat(true);
	}

	// Pressing esc return window in normal state
	if (k->key() == Qt::Key_Escape)
	{
		showNormal();
	}

	// Replay
	if (mGame && mGame->m_settings->recording == ppvs::RecordState::REPLAYING && k->key() == Qt::Key_Space)
	{
		if (mGame->m_replayState != ReplayState::PAUSED) {
            mGame->m_replayState = ReplayState::PAUSED;
        } else if (mGame->m_replayState == ReplayState::PAUSED) {
            mGame->m_replayState = ReplayState::NORMAL;
        }
    }
	else if (mGame && mGame->m_settings->recording == ppvs::RecordState::REPLAYING && k->key() == Qt::Key_Up)
	{
		if (mGame->m_replayState == ReplayState::NORMAL) {
            mGame->m_replayState = ReplayState::FAST_FORWARD;
        } else if (mGame->m_replayState == ReplayState::REWIND) {
            mGame->m_replayState = ReplayState::NORMAL;
        } else if (mGame->m_replayState == ReplayState::FAST_FORWARD) {
            mGame->m_replayState = ReplayState::FAST_FORWARD_X4;
        }
    }
	else if (mGame && mGame->m_settings->recording == ppvs::RecordState::REPLAYING && k->key() == Qt::Key_Down)
	{
		if (mGame->m_replayState == ReplayState::NORMAL) {
			mGame->m_replayState = ReplayState::REWIND;
		} else if (mGame->m_replayState == ReplayState::FAST_FORWARD) {
            mGame->m_replayState = ReplayState::NORMAL;
        } else if (mGame->m_replayState == ReplayState::FAST_FORWARD_X4) {
            mGame->m_replayState = ReplayState::FAST_FORWARD;
        }
    }

}

void GameWidgetGL::keyReleaseEvent(QKeyEvent* k)
{
	InputCondition::MatchResult r;
#define HANDLEKEY(i, button) r=mControls[i].match(k);if (r == InputCondition::MatchUp) d->inputState. button = false
	HANDLEKEY(0, up);
	HANDLEKEY(1, down);
	HANDLEKEY(2, left);
	HANDLEKEY(3, right);
	HANDLEKEY(4, a);
	HANDLEKEY(5, b);
	HANDLEKEY(6, start);
#undef HANDLEKEY
}

void GameWidgetGL::paintEvent(QPaintEvent*)
{
	d->gl->makeCurrent();
	mGame->renderGame();
	d->gl->swapBuffers();
}

void GameWidgetGL::closeEvent(QCloseEvent* event)
{
	if (mGame && mGame->m_settings->rankedMatch && mGame->countBoundPlayers() > 1)
	{
		// Do not close
		event->ignore();
	}
	else
	{
		mGame->m_runGame = false;
		close();
	}
}

void GameWidgetGL::initialize()
{
	d->gl->makeCurrent();
	auto frontend = new FrontendGL(d->gl, d->audio, d->inputState, d->ext, this);
	mGame->initGame(frontend);
	connect(frontend, SIGNAL(musicStateChanged(int)), SLOT(setupMusic(int)));
	connect(frontend, SIGNAL(musicVolumeChanged(float, bool)), SLOT(changeMusicVolume(float, bool)));
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, width(), height());

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 640, 480, 0, -100, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	d->gl->doneCurrent();

	if (!mGame->m_channelName.empty())
		mGame->m_network->createChannel(mGame->m_channelName, "", false, true);

	d->ready = true;
	d->nextFrame = timeGetTime() + 1000 / 60;
}

void GameWidgetGL::process()
{
	if (!d->ready || !mGame->m_runGame)
		return;

	quint64 now = timeGetTime();

	if (now >= d->nextFrame)
	{
		d->gl->makeCurrent();
		d->nextFrame += 1000 / 60;
		mGame->playGame();

		if (qApp->activeWindow() == window())
		{
			InputCondition::MatchResult r;
			d->inputDriver->process();
			ilib::InputEvent e;
			while (d->inputDriver->getEvent(&e))
			{
#define HANDLEJOY(i, button) \
    r=mControls[i].match(e); \
    if(r==InputCondition::MatchUp) d->inputState. button = false; \
    else if(r==InputCondition::MatchDown) d->inputState. button = true;
				HANDLEJOY(0, up);
				HANDLEJOY(1, down);
				HANDLEJOY(2, left);
				HANDLEJOY(3, right);
				HANDLEJOY(4, a);
				HANDLEJOY(5, b);
				HANDLEJOY(6, start);
#undef HANDLEJOY
			}
		}

		// Frameskipping
		while (now > d->nextFrame)
		{
			mGame->playGame();
			d->nextFrame += 1000 / 60;
		}

		mGame->renderGame();

		if (!mGame->m_runGame)
		{
			close();
			return;
		}
	}
}
