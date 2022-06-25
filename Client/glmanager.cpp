#include "glmanager.h"

GLManager *glMan = 0;

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

GLManager::GLManager() : mGlobalContextSharingEnabled(false), mGlobalWidget(0)
{
    mGlobalWidget = new QGLWidget();
    if(!mGlobalWidget->isValid())
    {
        delete mGlobalWidget;
        qDebug("Error initializing OpenGL context sharing.");
        return;
    }
    mGlobalContextSharingEnabled = true;
    qDebug("Global context sharing enabled.");
}
GLManager::~GLManager()
{
    delete mGlobalWidget;
}

QGLWidget *GLManager::globalWidget()
{
    return mGlobalWidget;
}

bool GLManager::sharing()
{
    return mGlobalContextSharingEnabled;
}

GLuint GLManager::texID(QString tx)
{
    return mTextures[tx].id;
}

GLTexture GLManager::loadTexture(QGLWidget *context, QString name)
{
    GLTexture tx = {0, 0, 0};
    if(mGlobalContextSharingEnabled && context->isSharing())
    {
        if(mTextures.contains(name)) return mTextures[name];
        else
        {
            mGlobalWidget->makeCurrent();
            QImage image = QImage(name);
            tx.id = mGlobalWidget->bindTexture(image);
            glBindTexture(GL_TEXTURE_2D, tx.id);
#if defined(Q_OS_MAC)
            glGenerateMipmap(GL_TEXTURE_2D);
#endif
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            tx.w = image.width();
            tx.h = image.height();
            mTextures[name] = tx;
            mGlobalWidget->doneCurrent();
        }
    }
    else
    {
        QImage image = QImage(name);
        tx.w = image.width();
        tx.h = image.height();
        tx.id = context->bindTexture(image);
    }
    return tx;
}

QString GLManager::errorString(GLenum errorCode)
{
    switch(errorCode)
    {
    case GL_NO_ERROR: return "No error.";
    case GL_INVALID_ENUM: return "GL_INVALID_ENUM: An unacceptable value was specified for an enumerated argument.";
    case GL_INVALID_VALUE: return "GL_INVALID_VALUE: A numeric argument was out of range.";
    case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION: The specified operation was not allowed in the current state.";
    case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object was not complete.";
    case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY: There was not enough memory left to execute the command.";
    case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW: An attempt was made to perform an operation that would cause an internal stack underflow.";
    case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW: An attempt was made to perform an operation that would cause an internal stack overflow.";
    default: return "An unknown error was encountered.";
    }
}

bool GLManager::checkError(bool silent)
{
    bool haveErrors = false;
    GLenum errorCode = GL_NO_ERROR;
    while((errorCode = glGetError()) != GL_NO_ERROR)
    {
        if(!silent)
            QMessageBox(QMessageBox::Warning, "Puyo VS", QString("GL error: ") + errorString(glGetError()), QMessageBox::Ok).exec();
        haveErrors = true;
    }

    return haveErrors;
}

void GlobalGLInit()
{
    if(!glMan) glMan = new GLManager();
}

void GlobalGLDeInit()
{
    if(glMan) delete glMan;
}
