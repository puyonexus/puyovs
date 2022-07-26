#pragma once

#include <string>

namespace ppvs {

enum FilterType {
	NearestFilter,
	LinearFilter
};

enum BlendingMode {
	NoBlending,
	AlphaBlending,
	AdditiveBlending,
	MultiplyBlending
};

enum DepthFunction {
	Always,
	LessOrEqual,
	GreaterOrEqual,
	Equal
};

struct ViewportGeometry {
	ViewportGeometry()
		: w(0)
		, h(0)
	{
	}
	ViewportGeometry(const int w, const int h)
		: w(w)
		, h(h)
	{
	}
	int w, h;
};

struct RectGeometry {
	RectGeometry()
		: x(0.)
		, y(0.)
		, w(0.)
		, h(0.)
	{
	}
	RectGeometry(double x, double y, double w, double h)
		: x(x)
		, y(y)
		, w(w)
		, h(h)
	{
	}
	double x, y, w, h;
};

struct FePixel {
	unsigned char a, r, g, b;
	FePixel(unsigned char a, unsigned char r, unsigned char g, unsigned char b)
		: a(a)
		, r(r)
		, g(g)
		, b(b)
	{
	}
};

class FeImage {
public:
	FeImage() = default;
	virtual ~FeImage() = default;

	FeImage(const FeImage&) = delete;
	FeImage& operator=(const FeImage&) = delete;
	FeImage(FeImage&&) = delete;
	FeImage& operator=(FeImage&&) = delete;

	virtual int width() = 0;
	virtual int height() = 0;
	virtual FePixel pixel(int x, int y) = 0;
	virtual bool error() = 0;

	virtual void setFilter(FilterType) = 0;
};

class FeShader {
public:
	FeShader() = default;
	virtual ~FeShader() = default;

	FeShader(const FeShader&) = delete;
	FeShader& operator=(const FeShader&) = delete;
	FeShader(FeShader&&) = delete;
	FeShader& operator=(FeShader&&) = delete;

	virtual bool setSource(const char* src) = 0;
	virtual bool setParameter(const char* param, double value) = 0;
	virtual bool setParameter(const char* param, double x, double y, double z, double w) = 0;
	virtual bool setCurrentTexture(const char* param) = 0;

	virtual bool compile() = 0;
	virtual void bind() = 0;
	virtual void unbind() = 0;
};

class FeSound {
public:
	FeSound() = default;
	virtual ~FeSound() = default;

	FeSound(const FeSound&) = delete;
	FeSound& operator=(const FeSound&) = delete;
	FeSound(FeSound&&) = delete;
	FeSound& operator=(FeSound&&) = delete;

	virtual void play() = 0;
	virtual void stop() = 0;
};

enum FeMusicEvent {
	MusicCanStop,
	MusicContinue,
	MusicEnterFever,
	MusicExitFever
};

class FeText {
public:
	FeText() = default;
	virtual ~FeText() = default;

	FeText(const FeText&) = delete;
	FeText& operator=(const FeText&) = delete;
	FeText(FeText&&) = delete;
	FeText& operator=(FeText&&) = delete;

	virtual void draw(float x, float y) = 0;
};

class FeFont {
public:
	FeFont() = default;
	virtual ~FeFont() = default;

	FeFont(const FeFont&) = delete;
	FeFont& operator=(const FeFont&) = delete;
	FeFont(FeFont&&) = delete;
	FeFont& operator=(FeFont&&) = delete;

	virtual FeText* render(const char* str) = 0;
};

struct FeInput {
	bool up, down, left, right, a, b, start;
};

class FeRenderTarget {
public:
	FeRenderTarget() = default;
	virtual ~FeRenderTarget() = default;

	FeRenderTarget(const FeRenderTarget&) = delete;
	FeRenderTarget& operator=(const FeRenderTarget&) = delete;
	FeRenderTarget(FeRenderTarget&&) = delete;
	FeRenderTarget& operator=(FeRenderTarget&&) = delete;

	// - Graphics -
	virtual FeImage* loadImage(const char* nameU8) = 0;
	virtual FeImage* loadImage(const std::string& nameU8) = 0;
	virtual FeFont* loadFont(const char* nameU8, double fontSize) = 0;

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
	virtual void drawRect(FeImage* image, double subX, double subY, double subW, double subH) = 0;

	virtual void clear() = 0;
	virtual void swapBuffers() = 0;
};

class Frontend : public FeRenderTarget {
public:
	Frontend() = default;
	~Frontend() override = default;

	Frontend(const Frontend&) = delete;
	Frontend& operator=(const Frontend&) = delete;
	Frontend(Frontend&&) = delete;
	Frontend& operator=(Frontend&&) = delete;

	// - Audio -
	virtual FeSound* loadSound(const char* nameU8) = 0;
	virtual FeSound* loadSound(const std::string& nameU8) = 0;
	virtual void musicEvent(FeMusicEvent event) = 0;
	virtual void musicVolume(float volume, bool fever) = 0;

	// - Input -
	virtual FeInput inputState(int pl) = 0;
};

}
