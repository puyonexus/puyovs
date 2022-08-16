#include "frontendgl.h"
#include "ffontgl.h"
#include "fimagegl.h"
#include "fshadergl.h"
#include "fsoundalib.h"

FrontendGL::FrontendGL(QGLWidget* gl, GameAudio* audio, ppvs::FeInput& input, GLExtensions& ext, QObject* parent)
	: QObject(parent)
	, gl(gl)
	, audio(audio)
	, input(input)
	, ext(ext)
{
}

ppvs::FeImage* FrontendGL::loadImage(const char* nameu8)
{
	return new FImageGL(nameu8, gl, this);
}

ppvs::FeImage* FrontendGL::loadImage(const std::string& nameu8)
{
	return loadImage(nameu8.c_str());
}

ppvs::FeShader* FrontendGL::loadShader(const char* source)
{
	if (ext.haveGLSL)
		return new FShaderGL(source, ext, this);
	return nullptr;
}

ppvs::FeFont* FrontendGL::loadFont(const char* nameu8, double fontSize)
{
	return new FFontGL(nameu8, fontSize, gl, this);
}

ppvs::FeSound* FrontendGL::loadSound(const char* nameu8)
{
	return new FSoundAlib(nameu8, audio, this);
}

ppvs::FeSound* FrontendGL::loadSound(const std::string& nameu8)
{
	return loadSound(nameu8.c_str());
}

void FrontendGL::musicEvent(ppvs::FeMusicEvent event)
{
	emit musicStateChanged(int(event));
}

void FrontendGL::musicVolume(float volume, bool fever)
{
	emit musicVolumeChanged(volume, fever);
}

ppvs::FeInput FrontendGL::inputState(int pl)
{
	if (pl == 0)
		return input;
	return ppvs::FeInput();
}

void FrontendGL::pushMatrix()
{
	glPushMatrix();
}

void FrontendGL::popMatrix()
{
	glPopMatrix();
}

void FrontendGL::identity()
{
	glLoadIdentity();
}

void FrontendGL::translate(float x, float y, float z)
{
	glTranslatef(x, y, z);
}

void FrontendGL::rotate(float v, float x, float y, float z)
{
	glRotatef(v, x, y, z);
}

void FrontendGL::scale(float x, float y, float z)
{
	glScalef(x, y, z);
}

ppvs::ViewportGeometry FrontendGL::viewport()
{
	return ppvs::ViewportGeometry(gl->width(), gl->height());
}

bool FrontendGL::hasShaders()
{
	return ext.haveGLSL;
}

void FrontendGL::setBlendMode(ppvs::BlendingMode b)
{
	switch (b) {
	case ppvs::BlendingMode::NoBlending:
		glDisable(GL_BLEND);
		break;
	case ppvs::BlendingMode::AlphaBlending:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case ppvs::BlendingMode::AdditiveBlending:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case ppvs::BlendingMode::MultiplyBlending:
		glEnable(GL_BLEND);
		glBlendFunc(GL_DST_COLOR, GL_ZERO);
		break;
	}
}

void FrontendGL::setColor(int r, int g, int b, int a)
{
	glColor4f(float(r) / 255.f, float(g) / 255.f, float(b) / 255.f, float(a) / 255.f);
}

void FrontendGL::unsetColor()
{
	glColor4f(1.f, 1.f, 1.f, 1.f);
}

void FrontendGL::setDepthFunction(ppvs::DepthFunction d)
{
	switch (d) {
	case ppvs::DepthFunction::Always:
		glDisable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);
		break;
	case ppvs::DepthFunction::LessOrEqual:
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		break;
	case ppvs::DepthFunction::GreaterOrEqual:
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_GEQUAL);
		break;
	case ppvs::DepthFunction::Equal:
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_EQUAL);
		break;
	}
}

void FrontendGL::clearDepth()
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

void FrontendGL::enableAlphaTesting(float tolerance)
{
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, tolerance);
}

void FrontendGL::disableAlphaTesting()
{
	glDisable(GL_ALPHA_TEST);
}

void FrontendGL::drawRect(ppvs::FeImage* image, double subx, double suby, double subw, double subh)
{
	FImageGL* qimg = (FImageGL*)image;
	double u1, v1, u2, v2, w = subw, h = subh;

	u1 = v1 = u2 = v2 = 0.;

	if (qimg) {
		qimg->bind();
		double tw = qimg->width(), th = qimg->height();
		u1 = (subx + 0.25) / tw;
		v1 = 1. - (suby + subh - 0.5) / th;
		u2 = (subx + subw - 0.25) / tw;
		v2 = 1. - (suby + 0.5) / th;
	} else
		glBindTexture(GL_TEXTURE_2D, 0);

	double off = 0.0, toff = 0.0; // Bleed a bit
	glBegin(GL_QUADS);
	{
		glTexCoord2d(u1, v2);
		glVertex2d(0 + 0.25, 0 + 0.25);
		glTexCoord2d(u2, v2);
		glVertex2d(w - 0.25, 0 + 0.25);
		glTexCoord2d(u2, v1);
		glVertex2d(w - 0.25, h + 0.25);
		glTexCoord2d(u1, v1);
		glVertex2d(0 + 0.25, h + 0.25);
	}
	glEnd();
}

void FrontendGL::clear()
{
	glClear(GL_COLOR_BUFFER_BIT);
}

void FrontendGL::swapBuffers()
{
	gl->swapBuffers();
}
