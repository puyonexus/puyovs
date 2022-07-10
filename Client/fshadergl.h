#pragma once

#include <QObject>
#include "glextensions.h"
#include "../Puyolib/frontend.h"

class FShaderGL : public ppvs::fshader, public QObject
{
public:
    FShaderGL(const char *source, GLExtensions &ext, QObject *parent);

    ~FShaderGL();

    bool setSource(const char *src);

    bool setParameter(const char *param, double value);
    bool setParameter(const char *param, double x, double y, double z, double w);
    bool setCurrentTexture(const char *param);

    bool compile();
    void bind();
    void unbind();

private:
    std::string source;
    GLExtensions ext;
    GLuint program;
    GLint currentTex;
};
