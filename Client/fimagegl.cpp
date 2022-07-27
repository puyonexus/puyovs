#include "fimagegl.h"
#include "glmanager.h"

FImageGL::FImageGL(const QString& fn, QGLWidget* gl, QObject* parent)
	: QObject(parent)
	, w(0)
	, h(0)
	, id(0)
	, fn(fn)
{
	GLTexture texture = glMan->loadTexture(gl, fn);
	gl->makeCurrent();

	if (texture.w > 0 && texture.h > 0 && texture.id != 0) {
		w = texture.w;
		h = texture.h;
		id = texture.id;
	}
}

FImageGL::~FImageGL()
{
}

int FImageGL::width()
{
	return w;
}

int FImageGL::height()
{
	return h;
}

void FImageGL::bind() const
{
	glBindTexture(GL_TEXTURE_2D, id);
}

ppvs::FePixel FImageGL::pixel(int x, int y)
{
	if (tx.isNull())
		tx = QImage(fn);
	const QColor px { tx.pixelColor(x, y) };
	return ppvs::FePixel(px.alpha(), px.red(), px.green(), px.blue());
}

bool FImageGL::error()
{
	return id == 0 || w == 0 || h == 0;
}

void FImageGL::setFilter(ppvs::FilterType)
{
}
