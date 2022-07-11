#include "ffontgl.h"
#include "glmanager.h"
#include <QImage>
#include <QPainter>
#include <QFontMetrics>

FTextGL::FTextGL(GLuint id, GLuint w, GLuint h, QObject* parent)
	: QObject(parent), id(id), w(w), h(h)
{
}

FTextGL::~FTextGL()
{
	glDeleteTextures(1, &id);
}

void FTextGL::draw(float x, float y)
{
	glBindTexture(GL_TEXTURE_2D, id);
	glPushMatrix();
	glTranslatef(x, y, 0);
	glBegin(GL_QUADS);
	{
		glTexCoord2d(0, 0); glVertex2d(0, 0);
		glTexCoord2d(1, 0); glVertex2d(w, 0);
		glTexCoord2d(1, 1); glVertex2d(w, h);
		glTexCoord2d(0, 1); glVertex2d(0, h);
	}
	glEnd();
	glPopMatrix();
}

FFontGL::FFontGL(const QString& fn, double fontSize, QGLWidget* gl, QObject* parent)
	: QObject(parent), gl(gl), font(fn, fontSize)
{
}

QImage FFontGL::renderTextline(const QString& line) const
{
	QFontMetrics fm(font);
	QImage image;
	image = QImage(fm.width(line) + 4, fm.lineSpacing(), QImage::Format_ARGB32);
	image.fill(0);

	QPainter p;
	p.begin(&image);
	p.setFont(font);
	p.setPen(QColor(Qt::black));
	p.drawText(3, fm.ascent() + 3, line);
	p.setPen(QColor(Qt::white));
	p.drawText(1, fm.ascent() + 1, line);
	p.end();

	return image;
}

ppvs::ftext* FFontGL::render(const char* str)
{
	QFontMetrics fm(font);
	int lineSpacing = fm.lineSpacing();
	QStringList lines = QString(str).split('\n');
	QList<QImage> images;
	int blockHeight = lines.count() * lineSpacing;
	int maxWidth = 0;

	for (int i = 0; i < lines.count(); ++i)
	{
		QString line = lines.at(i);
		QImage image = renderTextline(line);
		if (image.width() > maxWidth) maxWidth = image.width();
		images.append(image);
	}

	QImage blockImage(maxWidth, blockHeight, QImage::Format_ARGB32);
	blockImage.fill(0);
	int blockCursor = 0;
	QPainter p;
	p.begin(&blockImage);
	for (int i = 0; i < images.count(); ++i)
	{
		p.drawImage(0, blockCursor, images.at(i));
		blockCursor += lineSpacing;
	}
	p.end();

	gl->makeCurrent();
	GLuint id = 0;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, blockImage.width(), blockImage.height(), 0, 0x80E1/* GL_BGRA */, GL_UNSIGNED_BYTE, blockImage.bits());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return new FTextGL(id, blockImage.width(), blockImage.height(), this);
}
