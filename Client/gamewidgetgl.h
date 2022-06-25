#ifndef GAMEWIDGETGL_H
#define GAMEWIDGETGL_H

#include "gameaudio.h"
#include "gamewidget.h"
#include <QWidget>
#include <QImage>

struct GameWidgetGLPriv;

namespace ppvs
{
    class game;
};

class GameWidgetGL : public GameWidget
{
    friend class QtFrontend;

    Q_OBJECT
public:
    explicit GameWidgetGL(ppvs::game *game, NetChannelProxy *proxy, GameAudio *audio, QWidget *parent = 0);
    ~GameWidgetGL();

protected:
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void paintEvent(QPaintEvent *);
    void closeEvent(QCloseEvent *);
    
public slots:
    void initialize();
    void process();

private:
    GameWidgetGLPriv *d;
};

#endif // GAMEWIDGETGL_H
