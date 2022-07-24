#pragma once

#include <string>

namespace ppvs
{

enum FilterType
{
	NearestFilter,
	LinearFilter
};

enum BlendingMode
{
	NoBlending,
	AlphaBlending,
	AdditiveBlending,
	MultiplyBlending
};

enum DepthFunction
{
	Always,
	LessOrEqual,
	GreaterOrEqual,
	Equal
};

struct ViewportGeometry
{
	ViewportGeometry() : w(0), h(0) { }
	ViewportGeometry(int w, int h) : w(w), h(h) { }
	int w, h;
};

struct RectGeometry
{
	RectGeometry() : x(0.), y(0.), w(0.), h(0.) { }
	RectGeometry(double x, double y, double w, double h) : x(x), y(y), w(w), h(h) { }
	double x, y, w, h;
};

struct FePixel { unsigned char a, r, g, b; FePixel(unsigned char a, unsigned char r, unsigned char g, unsigned char b) :a(a), r(r), g(g), b(b) {} };

class FeImage
{
public:
	virtual ~FeImage() { }

	virtual int width() = 0;
	virtual int height() = 0;
	virtual FePixel pixel(int x, int y) = 0;
	virtual bool error() = 0;

	virtual void setFilter(FilterType) = 0;
};

class FeShader
{
public:
	virtual ~FeShader() { }

	virtual bool setSource(const char* src) = 0;
	virtual bool setParameter(const char* param, double value) = 0;
	virtual bool setParameter(const char* param, double x, double y, double z, double w) = 0;
	virtual bool setCurrentTexture(const char* param) = 0;

	virtual bool compile() = 0;
	virtual void bind() = 0;
	virtual void unbind() = 0;
};

class FeSound
{
public:
	virtual ~FeSound() { }

	virtual void play() = 0;
	virtual void stop() = 0;
};

enum FeMusicEvent
{
	MusicCanStop,
	MusicContinue,
	MusicEnterFever,
	MusicExitFever
};

class FeText
{
public:
	virtual void draw(float x, float y) = 0;
	virtual ~FeText() { }
};

class FeFont
{
public:
	virtual ~FeFont() { }

	virtual FeText* render(const char* str) = 0;
};

struct FeInput
{
	bool up, down, left, right, a, b, start;
};

class FeRenderTarget
{
public:
	virtual ~FeRenderTarget() { }

	// - Graphics -
	virtual FeImage* loadImage(const char* nameu8) = 0;
	virtual FeImage* loadImage(const std::string& nameu8) = 0;
	virtual FeFont* loadFont(const char* nameu8, double fontSize) = 0;

	// Matrix
	virtual void pushMatrix() = 0;
	virtual void popMatrix() = 0;
	virtual void identity() = 0;
	virtual void translate(float x, float y, float z) = 0;
	virtual void rotate(float v, float x, float y, float z) = 0;
	virtual void scale(float x, float y, float z) = 0;

	// Viewport
	virtual ViewportGeometry viewport() = 0;

	// Shaders
	virtual bool hasShaders() = 0;
	virtual FeShader* loadShader(const char* source = nullptr) = 0;

	// Blending
	virtual void setBlendMode(BlendingMode) = 0;
	virtual void setColor(int r, int g, int b, int a) = 0;
	virtual void unsetColor() = 0;

	// Depth
	virtual void setDepthFunction(DepthFunction) = 0;
	virtual void clearDepth() = 0;

	// Alpha test
	virtual void enableAlphaTesting(float tolerance) = 0;
	virtual void disableAlphaTesting() = 0;

	// Drawing
	virtual void drawRect(FeImage* image, double subx, double suby, double subw, double subh) = 0;

	virtual void clear() = 0;
	virtual void swapBuffers() = 0;
};


class Frontend : public FeRenderTarget
{
public:
	virtual ~Frontend() { }

	// - Audio -
	virtual FeSound* loadSound(const char* nameu8) = 0;
	virtual FeSound* loadSound(const std::string& nameu8) = 0;
	virtual void musicEvent(FeMusicEvent event) = 0;
	virtual void musicVolume(float volume, bool fever) = 0;

	// - Input -
	virtual FeInput inputState(int pl) = 0;
};

}
