#pragma once

#include <string>

namespace ppvs
{

enum filterType
{
    nearestFilter,
    linearFilter
};

enum blendingMode
{
    noBlending,
    alphaBlending,
    additiveBlending,
    multiplyBlending
};

enum depthFunction
{
    always,
    lessOrEqual,
    greaterOrEqual,
    equal
};

struct viewportGeometry
{
    viewportGeometry() : w(0), h(0) { }
    viewportGeometry(int w, int h) : w(w), h(h) { }
    int w, h;
};

struct rectGeometry
{
    rectGeometry() : x(0.), y(0.), w(0.), h(0.) { }
    rectGeometry(double x, double y, double w, double h) : x(x), y(y), w(w), h(h) { }
    double x, y, w, h;
};

struct fpixel { unsigned char a,r,g,b; fpixel(unsigned char a, unsigned char r, unsigned char g, unsigned char b):a(a),r(r),g(g),b(b){} };

class fimage
{
public:
    virtual ~fimage() { }

    virtual int width() = 0;
    virtual int height() = 0;
    virtual fpixel pixel(int x, int y) = 0;
    virtual bool error() = 0;

    virtual void setFilter(filterType) = 0;
};

class fshader
{
public:
    virtual ~fshader() { }

    virtual bool setSource(const char *src) = 0;
    virtual bool setParameter(const char *param, double value) = 0;
    virtual bool setParameter(const char *param, double x, double y, double z, double w) = 0;
    virtual bool setCurrentTexture(const char *param) = 0;

    virtual bool compile() = 0;
    virtual void bind() = 0;
    virtual void unbind() = 0;
};

class fsound
{
public:
    virtual ~fsound() { }

    virtual void play() = 0;
    virtual void stop() = 0;
};

enum fmusicevent
{
    music_can_stop,
    music_continue,
    music_enter_fever,
    music_exit_fever
};

class ftext
{
public:
    virtual void draw(float x, float y) = 0;
    virtual ~ftext() { }
};

class ffont
{
public:
    virtual ~ffont() { }

    virtual ftext *render(const char *str) = 0;
};

struct finput
{
    bool up, down, left, right, a, b, start;
};

class frendertarget
{
public:
    virtual ~frendertarget() { }

    // - graphics -
    virtual fimage *loadImage(const char *nameu8) = 0;
    virtual fimage *loadImage(const std::string &nameu8) = 0;
    virtual ffont *loadFont(const char *nameu8, double fontSize) = 0;

    // matrix
    virtual void pushMatrix() = 0;
    virtual void popMatrix() = 0;
    virtual void identity() = 0;
    virtual void translate(float x, float y, float z) = 0;
    virtual void rotate(float v, float x, float y, float z) = 0;
    virtual void scale(float x, float y, float z) = 0;

    // viewport
    virtual viewportGeometry viewport() = 0;

    // shaders
    virtual bool hasShaders() = 0;
    virtual fshader *loadShader(const char *source = nullptr) = 0;

    // blending
    virtual void setBlendMode(blendingMode) = 0;
    virtual void setColor(int r, int g, int b, int a) = 0;
    virtual void unsetColor() = 0;

    // depth
    virtual void setDepthFunction(depthFunction) = 0;
    virtual void clearDepth() = 0;

    // alpha test
    virtual void enableAlphaTesting(float tolerance) = 0;
    virtual void disableAlphaTesting() = 0;

    // drawing
    virtual void drawRect(fimage *image, double subx, double suby, double subw, double subh) = 0;

    virtual void clear() = 0;
    virtual void swapBuffers() = 0;
};


class frontend : public frendertarget
{
public:
    virtual ~frontend() { }

    // - audio -
    virtual fsound *loadSound(const char *nameu8) = 0;
    virtual fsound *loadSound(const std::string &nameu8) = 0;
    virtual void musicEvent(fmusicevent event) = 0;
    virtual void musicVolume(float volume,bool fever) =0;

    // - input -
    virtual finput inputState(int pl) = 0;

private:

};

}
