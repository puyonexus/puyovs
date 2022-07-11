#pragma once

#include <QtOpenGL>
#include <QObject>
#include "glextensions.h"
#include "gameaudio.h"
#include "../Puyolib/frontend.h"

class FrontendGL : public QObject, public ppvs::frontend
{
	Q_OBJECT

public:
	FrontendGL(QGLWidget* gl, GameAudio* audio, ppvs::finput& input, GLExtensions& ext, QObject* parent = nullptr);
	ppvs::fimage* loadImage(const char* nameu8) override;
	ppvs::fimage* loadImage(const std::string& nameu8) override;
	ppvs::fshader* loadShader(const char* source) override;
	ppvs::ffont* loadFont(const char* nameu8, double fontSize) override;
	ppvs::fsound* loadSound(const char* nameu8) override;
	ppvs::fsound* loadSound(const std::string& nameu8) override;

	void musicEvent(ppvs::fmusicevent event) override;
	void musicVolume(float volume, bool fever) override;
	ppvs::finput inputState(int pl) override;

	void pushMatrix() override;
	void popMatrix() override;
	void identity() override;
	void translate(float x, float y, float z) override;
	void rotate(float v, float x, float y, float z) override;
	void scale(float x, float y, float z) override;

	ppvs::viewportGeometry viewport() override;
	bool hasShaders() override;

	void setBlendMode(ppvs::blendingMode b) override;
	void setColor(int r, int g, int b, int a) override;
	void unsetColor() override;
	void setDepthFunction(ppvs::depthFunction d) override;
	void clearDepth() override;
	void enableAlphaTesting(float tolerance) override;
	void disableAlphaTesting() override;
	void drawRect(ppvs::fimage* image, double subx, double suby, double subw, double subh) override;

	void clear() override;
	void swapBuffers() override;

signals:
	void musicStateChanged(int);
	void musicVolumeChanged(float, bool);

private:
	QGLWidget* gl;
	GameAudio* audio;
	ppvs::finput& input;
	GLExtensions& ext;
};
