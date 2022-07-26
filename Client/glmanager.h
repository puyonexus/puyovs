#pragma once

#include <QMap>
#include <QString>
#include <QtOpenGL>

class GLManager;
class QGLWidget;

extern GLManager* glMan;

struct GLTexture {
	GLuint id, w, h;
};
class GLManager {
public:
	GLManager();
	~GLManager();

	QGLWidget* globalWidget() const;
	bool sharing() const;
	GLuint texID(QString tx);
	GLTexture loadTexture(QGLWidget* context, QString name);

	static QString errorString(GLenum errorCode);
	static bool checkError(bool silent = false);

private:
	bool mGlobalContextSharingEnabled;
	QGLWidget* mGlobalWidget;
	QMap<QString, GLTexture> mTextures;
};

void GlobalGLInit();
void GlobalGLDeInit();
