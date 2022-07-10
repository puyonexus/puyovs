#pragma once

#include <QtOpenGL>
#include <QObject>
#include <alib/mixer.h>
#include "glextensions.h"
#include "gameaudio.h"
#include "../Puyolib/frontend.h"

class FrontendGL : public QObject, public ppvs::frontend
{
    Q_OBJECT
public:
    FrontendGL(QGLWidget *gl, GameAudio *audio, ppvs::finput &input, GLExtensions &ext, QObject *parent = 0);
    ppvs::fimage *loadImage(const char *nameu8);
    ppvs::fimage *loadImage(const std::string &nameu8);
    ppvs::fshader *loadShader(const char *source);
    ppvs::ffont *loadFont(const char *nameu8, double fontSize);
    ppvs::fsound *loadSound(const char *nameu8);
    ppvs::fsound *loadSound(const std::string &nameu8);

    void musicEvent(ppvs::fmusicevent event);
    void musicVolume(float volume,bool fever);
    ppvs::finput inputState(int pl);

    void pushMatrix();
    void popMatrix();
    void identity();
    void translate(float x, float y, float z);
    void rotate(float v, float x, float y, float z);
    void scale(float x, float y, float z);

    ppvs::viewportGeometry viewport();
    bool hasShaders();

    void setBlendMode(ppvs::blendingMode b);
    void setColor(int r, int g, int b, int a);
    void unsetColor();
    void setDepthFunction(ppvs::depthFunction d);
    void clearDepth();
    void enableAlphaTesting(float tolerance);
    void disableAlphaTesting();
    void drawRect(ppvs::fimage *image, double subx, double suby, double subw, double subh);

    void clear();
    void swapBuffers();

signals:
    void musicStateChanged(int);
    void musicVolumeChanged(float,bool);

private:
    QGLWidget *gl;
    GameAudio *audio;
    ppvs::finput &input;
    GLExtensions &ext;
};
