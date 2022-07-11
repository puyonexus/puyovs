#include "fimagegl.h"
#include "glmanager.h"

FImageGL::FImageGL(const QString& fn, QGLWidget* gl, QObject* parent)
	: QObject(parent), w(0), h(0), id(0), fn(fn)
{
	GLTexture texture = glMan->loadTexture(gl, fn);
	gl->makeCurrent();

	if (texture.w > 0 && texture.h > 0 && texture.id != 0)
	{
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

ppvs::fpixel FImageGL::pixel(int x, int y)
{
	if (tx.isNull()) tx = QImage(fn);
	if (alpha.isNull()) alpha = tx.alphaChannel();
	QColor px(tx.pixel(x, y));
	return ppvs::fpixel(QColor(alpha.pixel(x, y)).red(), px.red(), px.green(), px.blue());
}

bool FImageGL::error()
{
	return id == 0 || w == 0 || h == 0;
}

void FImageGL::setFilter(ppvs::filterType)
{
}
