#pragma once

#include "gameaudio.h"
#include "gamewidget.h"
#include <QImage>
#include <QWidget>

struct GameWidgetGLPriv;

namespace ppvs {
class Game;
};

class GameWidgetGL : public GameWidget {
	friend class QtFrontend;

	Q_OBJECT
public:
	explicit GameWidgetGL(ppvs::Game* game, NetChannelProxy* proxy, GameAudio* audio, QWidget* parent = nullptr);
	~GameWidgetGL() override;

protected:
	void keyPressEvent(QKeyEvent*) override;
	void keyReleaseEvent(QKeyEvent*) override;
	void paintEvent(QPaintEvent*) override;
	void closeEvent(QCloseEvent*) override;

public slots:
	void initialize() override;
	void process() override;

private:
	GameWidgetGLPriv* d;
};
