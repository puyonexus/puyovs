#pragma once

#include "../Puyolib/Frontend.h"
#include "gameaudio.h"
#include "glextensions.h"
#include <QObject>
#include <QtOpenGL>

class FrontendGL : public QObject, public ppvs::Frontend {
	Q_OBJECT

public:
	FrontendGL(QGLWidget* gl, GameAudio* audio, ppvs::FeInput& input, GLExtensions& ext, QObject* parent = nullptr);
	ppvs::FeImage* loadImage(const char* nameu8) override;
	ppvs::FeImage* loadImage(const std::string& nameu8) override;
	ppvs::FeShader* loadShader(const char* source) override;
	ppvs::FeFont* loadFont(const char* nameu8, double fontSize) override;
	ppvs::FeSound* loadSound(const char* nameu8) override;
	ppvs::FeSound* loadSound(const std::string& nameu8) override;

	void musicEvent(ppvs::FeMusicEvent event) override;
	void musicVolume(float volume, bool fever) override;
	ppvs::FeInput inputState(int pl) override;

	void pushMatrix() override;
	void popMatrix() override;
	void identity() override;
	void translate(float x, float y, float z) override;
	void rotate(float v, float x, float y, float z) override;
	void scale(float x, float y, float z) override;

	ppvs::ViewportGeometry viewport() override;
	bool hasShaders() override;

	void setBlendMode(ppvs::BlendingMode b) override;
	void setColor(int r, int g, int b, int a) override;
	void unsetColor() override;
	void setDepthFunction(ppvs::DepthFunction d) override;
	void clearDepth() override;
	void enableAlphaTesting(float tolerance) override;
	void disableAlphaTesting() override;
	void drawRect(ppvs::FeImage* image, double subx, double suby, double subw, double subh) override;

	void clear() override;
	void swapBuffers() override;

signals:
	void musicStateChanged(int);
	void musicVolumeChanged(float, bool);

private:
	QGLWidget* gl;
	GameAudio* audio;
	ppvs::FeInput& input;
	GLExtensions& ext;
};
