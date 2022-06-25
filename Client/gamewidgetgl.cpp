// QtOpenGL and etc.
#include <QGLWidget>
#include <QPainter>
#include <QApplication>
#include <QKeyEvent>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QCloseEvent>

// Audiolib and Inputlib
#include <alib/audiolib.h>
#include <alib/stream.h>
#include <ilib/driver.h>
#include <ilib/inputevent.h>

// Puyolib hook-back
#include "../Puyolib/frontend.h"
#include "../Puyolib/game.h"
#include "../Puyolib/global.h"
#include "../PVS_ENet/PVS_Client.h"

#include "gamewidgetgl.h"
#include "common.h"
#include "glextensions.h"
#include "glmanager.h"
#include "frontendgl.h"
#include "chatwindow.h"

#ifndef _WIN32
#include <unistd.h> // for sleep()
#endif

class GLWidget : public QGLWidget { /* Just a QGLWidget that resizes the viewport with the widget. */
#if defined(Q_OS_MAC)
// Workaround for MacOSX resize. 
public: explicit GLWidget(QWidget *parent = 0, QGLWidget *shareWidget = 0) : QGLWidget(parent, shareWidget) {
    guard = false;
}
private: 
    bool guard;
protected: void resizeEvent(QResizeEvent * e)
#else
public: explicit GLWidget(QWidget *parent = 0, QGLWidget *shareWidget = 0) : QGLWidget(parent, shareWidget) {}
protected: void resizeEvent(QResizeEvent *)
#endif
    {
#if defined(Q_OS_MAC)
// Workaround for MacOSX resize. 
        if(guard)
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
        float w=float(windowWidth);
        float h=float(windowHeight);
        float ratio=h/w; // =0.75
        if (ratio > 0.75f)
            h=0.75f*w;
        else
            w=h/0.75f;
        int diffh=windowHeight-int(h);
        int diffw=windowWidth-int(w);

        //snap to 640 x 480
        if (abs(windowWidth-640) < 32 && abs(windowHeight) < 32)
        {
            w=640.0f;
            h=480.0f;
            diffh=windowHeight-int(h);
            diffw=windowWidth-int(w);
        }
        
#if defined(Q_OS_MAC)
        /* On MacOSX, this Qt commit messes with glViewport() : 
        https://qt.gitorious.org/qt/qtbase/commit/1caa0c023f4fa60446094e53f22ee79771130e2f?format=patch
        glViewport() is overridden by Cocoa, which will call it with
        glViewport(0,0,width(),height()), bypassing ratio correction.
        
        This fixes aspect ratio, but induces a slight flicker on resize events.
        And those events are triggered by entering text in the chat window...
        */
        setGeometry(x()+diffw/2, y()+diffh/2, int(w), int(h));
#endif
        glViewport(diffw/2, diffh/2, int(w), int(h));
#if defined(Q_OS_MAC)
// Workaround for MacOSX resize. 
        guard = false; // prevents reentry from setGeometry()
#endif
    }
};

struct GameWidgetGLPriv
{
    GLWidget *gl;
    GLExtensions ext;
    GameAudio *audio;
    ilib::Driver *inputDriver;
    ppvs::finput inputState;
    quint64 nextFrame;
    bool ready;
};

GameWidgetGL::GameWidgetGL(ppvs::game *game, NetChannelProxy *proxy, GameAudio *audio, QWidget *parent) :
    GameWidget(game, proxy, parent), d(new GameWidgetGLPriv)
{
    d->ready = false;
    d->audio = audio;
    d->inputDriver = ilib::getDriver();
    d->inputDriver->enableEvents();
    GlobalGLInit();

    if(glMan->globalWidget())
        d->gl = new GLWidget(this, glMan->globalWidget());
    else
        d->gl = new GLWidget(this);

    loadExtensions(d->gl->context(), &d->ext);

    setCentralWidget(d->gl);
    setFocusProxy(d->gl);
    d->gl->setFocusPolicy(Qt::StrongFocus);

    memset(&d->inputState, 0, sizeof(ppvs::finput));

#if defined(Q_OS_MAC)
// Workaround for MacOSX resize. 
    QPalette p;
    setBackgroundRole(QPalette::Window);
    setAutoFillBackground(true);
    p.setColor(QPalette::Window,QColor(0,0,0,0));
    setPalette(p);
#endif
    initialize();
}

GameWidgetGL::~GameWidgetGL()
{
    if(!mGame->channelName.empty())
    {
        //before leaving the channel, check if you're the last player
        if (mGame->countBoundPlayers() < 2 && mGame->network->connected)
            mGame->network->requestChannelDescription(mGame->channelName,std::string(""));
        mGame->network->leaveChannel(mGame->channelName);
    }

    d->inputDriver->disableEvents();

    delete d->gl;
    delete d;

    delete mGame;
}

void GameWidgetGL::keyPressEvent(QKeyEvent *k)
{
    InputCondition::MatchResult r;
    #define handlekey(i, button) r=mControls[i].match(k);if(r==InputCondition::MatchDown)d->inputState. button = true
    handlekey(0, up);
    handlekey(1, down);
    handlekey(2, left);
    handlekey(3, right);
    handlekey(4, a);
    handlekey(5, b);
    handlekey(6, start);
    #undef handlekey

    //i'm putting this here for the moment
    //this is not part of puyolib, because it may be hard
    //to notify the client when a replay is changed
    if (mGame->settings->recording==PVS_REPLAYING)
    {
        if (k->key()==Qt::Key_Left)
            mGame->previousReplay();
        else if (k->key()==Qt::Key_Right)
            mGame->nextReplay();
    }

    //pressing tab opens chatwindow
    if (chatWindow() && k->key()==Qt::Key_Space)
    {
        chatWindow()->setQuickChat(true);
    }
    //pressing esc return window in normal state
    if (k->key()==Qt::Key_Escape)
    {
        showNormal();
    }
    //replay
    if (mGame && mGame->settings->recording==PVS_REPLAYING && k->key()==Qt::Key_Space)
    {
        if (mGame->replayState!=REPLAYSTATE_PAUSED)
            mGame->replayState=REPLAYSTATE_PAUSED;
        else if (mGame->replayState==REPLAYSTATE_PAUSED)
            mGame->replayState=REPLAYSTATE_NORMAL;
    }
    else if (mGame && mGame->settings->recording==PVS_REPLAYING && k->key()==Qt::Key_Up)
    {
        if (mGame->replayState==REPLAYSTATE_NORMAL)
            mGame->replayState=REPLAYSTATE_FASTFORWARD;
        else if (mGame->replayState==REPLAYSTATE_REWIND)
            mGame->replayState=REPLAYSTATE_NORMAL;
        else if (mGame->replayState==REPLAYSTATE_FASTFORWARD)
            mGame->replayState=REPLAYSTATE_FASTFORWARDX4;
    }
    else if (mGame && mGame->settings->recording==PVS_REPLAYING && k->key()==Qt::Key_Down)
    {
        if (mGame->replayState==REPLAYSTATE_NORMAL)
        {
            mGame->replayState=REPLAYSTATE_REWIND;
        }
        else if (mGame->replayState==REPLAYSTATE_FASTFORWARD)
            mGame->replayState=REPLAYSTATE_NORMAL;
        else if (mGame->replayState==REPLAYSTATE_FASTFORWARDX4)
            mGame->replayState=REPLAYSTATE_FASTFORWARD;
    }

}

void GameWidgetGL::keyReleaseEvent(QKeyEvent *k)
{
    InputCondition::MatchResult r;
    #define handlekey(i, button) r=mControls[i].match(k);if(r==InputCondition::MatchUp)d->inputState. button = false
    handlekey(0, up); handlekey(1, down); handlekey(2, left); handlekey(3, right);
    handlekey(4, a); handlekey(5, b); handlekey(6, start);
    #undef handlekey
}

void GameWidgetGL::paintEvent(QPaintEvent *)
{
    d->gl->makeCurrent();
    mGame->renderGame();
    d->gl->swapBuffers();
}

void GameWidgetGL::closeEvent(QCloseEvent *event)
{
    if (mGame && mGame->settings->rankedMatch && mGame->countBoundPlayers() > 1)
    {
        //do not close
        event->ignore();
    }
    else
    {
        //QMainWindow::closeEvent(event);
        delete this;
    }
}

void GameWidgetGL::initialize()
{
    d->gl->makeCurrent();
    FrontendGL *frontend = new FrontendGL(d->gl, d->audio, d->inputState, d->ext, this);
    mGame->initGame(frontend);
    connect(frontend, SIGNAL(musicStateChanged(int)), SLOT(setupMusic(int)));
    connect(frontend, SIGNAL(musicVolumeChanged(float,bool)), SLOT(changeMusicVolume(float,bool)));
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

    if(!mGame->channelName.empty())
        mGame->network->createChannel(mGame->channelName, "", false, true);

    d->ready = true;
    d->nextFrame = timeGetTime() + 1000/60;
}

void GameWidgetGL::process()
{
    if(!d->ready)
        return;

    quint64 now = timeGetTime();

    if(now >= d->nextFrame)
    {
        d->gl->makeCurrent();
        d->nextFrame += 1000/60;
        mGame->playGame();

        /*if (!mGame->players[1]->recordMessages.empty())
        {
            for (int i=0;i<mGame->players[1]->recordMessages.size();i++)
                qDebug(mGame->players[1]->recordMessages[i].message);
        }*/

        if(qApp->activeWindow() == window())
        {
            InputCondition::MatchResult r;
            d->inputDriver->process();
            ilib::InputEvent e;
            while(d->inputDriver->getEvent(&e))
            {
                #define handlejoy(i, button) \
                    r=mControls[i].match(e); \
                    if(r==InputCondition::MatchUp) d->inputState. button = false; \
                    else if(r==InputCondition::MatchDown) d->inputState. button = true;
                handlejoy(0, up); handlejoy(1, down); handlejoy(2, left); handlejoy(3, right);
                handlejoy(4, a); handlejoy(5, b); handlejoy(6, start);
                #undef handlejoy
            }
        }

        // frameskipping
        while(now > d->nextFrame)
        {
            mGame->playGame();
            d->nextFrame += 1000/60;
        }

        mGame->renderGame();

        if(!mGame->runGame)
        {
            delete this;
            return;
        }
    }
}
