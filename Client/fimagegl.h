#ifndef FIMAGEGL_H
#define FIMAGEGL_H

#include <QObject>
#include <QImage>
#include <QString>

#include "../Puyolib/frontend.h"

class QGLWidget;

class FImageGL : public ppvs::fimage, public QObject
{
public:
    FImageGL(const QString &fn, QGLWidget *gl, QObject *parent);
    ~FImageGL();

    int width();
    int height();
    void bind();

    ppvs::fpixel pixel(int x, int y);

    bool error();
    void setFilter(ppvs::filterType);

private:

    QImage tx;
    int w, h, id;
    QString fn;
    QImage alpha;
};

#endif // FIMAGEGL_H
