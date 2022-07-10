#pragma once

#include <QtOpenGL>
#include <QObject>
#include <QString>
#include <QFont>

#include "../Puyolib/frontend.h"

class FTextGL : public ppvs::ftext, public QObject
{
public:
    FTextGL(GLuint id, GLuint w, GLuint h, QObject *parent = 0);
    ~FTextGL();

    void draw(float x, float y);

private:
    GLuint id, w, h;
};

class FFontGL : public ppvs::ffont, public QObject
{
public:
    FFontGL(const QString &fn, double fontSize, QGLWidget *gl, QObject *parent = 0);

    QImage renderTextline(const QString &line);
    ppvs::ftext *render(const char *str);

private:
    QGLWidget *gl;
    QFont font;
};
