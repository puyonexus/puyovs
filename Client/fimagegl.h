#pragma once

#include <QImage>
#include <QString>

#include "../Puyolib/frontend.h"

class QGLWidget;

class FImageGL : public ppvs::fimage, public QObject
{
public:
	FImageGL(const QString& fn, QGLWidget* gl, QObject* parent);
	~FImageGL() override;

	int width() override;
	int height() override;
	void bind() const;

	ppvs::fpixel pixel(int x, int y) override;

	bool error() override;
	void setFilter(ppvs::filterType) override;

private:

	QImage tx;
	int w, h, id;
	QString fn;
	QImage alpha;
};
