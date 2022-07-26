#pragma once

#include <QFont>
#include <QObject>
#include <QtOpenGL>

#include "../Puyolib/Frontend.h"

class FTextGL : public ppvs::FeText, public QObject {
public:
	FTextGL(GLuint id, GLuint w, GLuint h, QObject* parent = nullptr);
	~FTextGL() override;

	void draw(float x, float y) override;

private:
	GLuint id, w, h;
};

class FFontGL : public ppvs::FeFont, public QObject {
public:
	FFontGL(const QString& fn, double fontSize, QGLWidget* gl, QObject* parent = nullptr);

	QImage renderTextline(const QString& line) const;
	ppvs::FeText* render(const char* str) override;

private:
	QGLWidget* gl;
	QFont font;
};
