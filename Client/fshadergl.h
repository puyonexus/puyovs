#pragma once

#include "../Puyolib/Frontend.h"
#include "glextensions.h"

class FShaderGL : public ppvs::FeShader, public QObject {
public:
	FShaderGL(const char* source, GLExtensions& ext, QObject* parent);

	~FShaderGL() override;

	bool setSource(const char* src) override;

	bool setParameter(const char* param, double value) override;
	bool setParameter(const char* param, double x, double y, double z, double w) override;
	bool setCurrentTexture(const char* param) override;

	bool compile() override;
	void bind() override;
	void unbind() override;

private:
	std::string source;
	GLExtensions ext;
	GLuint program;
	GLint currentTex;
};
