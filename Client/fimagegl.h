#pragma once

#include <QImage>
#include <QString>

#include "../Puyolib/Frontend.h"

class QGLWidget;

class FImageGL : public ppvs::FeImage, public QObject {
public:
	FImageGL(const QString& fn, QGLWidget* gl, QObject* parent);
	~FImageGL() override;

	int width() override;
	int height() override;
	void bind() const;

	ppvs::FePixel pixel(int x, int y) override;

	bool error() override;
	void setFilter(ppvs::FilterType) override;

private:
	QImage tx;
	int w, h, id;
	QString fn;
};
