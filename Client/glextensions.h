#pragma once

#include <QtOpenGL>

class QGLContext;
struct GLExtensions;
void loadExtensions(const QGLContext* gl, GLExtensions* ext);

typedef GLuint(*glCreateProgramPtr) (void);
typedef void (*glDeleteProgramPtr) (GLuint program);
typedef void (*glAttachShaderPtr) (GLuint program, GLuint shader);
typedef void (*glLinkProgramPtr) (GLuint program);
typedef void (*glUseProgramPtr) (GLuint program);
typedef GLuint(*glCreateShaderPtr) (GLenum type);
typedef void (*glDeleteShaderPtr) (GLuint shader);
typedef void (*glShaderSourcePtr) (GLuint shader, GLsizei count, const char* const* string, const GLint* length);
typedef void (*glCompileShaderPtr) (GLuint shader);
typedef void (*glGetShaderInfoLogPtr) (GLuint shader, GLsizei bufSize, GLsizei* length, char* infoLog);
typedef void (*glGetProgramInfoLogPtr) (GLuint program, GLsizei bufSize, GLsizei* length, char* infoLog);
typedef void (*glGetShaderivPtr) (GLuint shader, GLenum pname, GLint* params);
typedef void (*glGetProgramivPtr) (GLuint program, GLenum pname, GLint* params);
typedef GLint(*glGetUniformLocationPtr) (GLuint program, const char* name);
typedef void (*glGetUniform1fPtr) (GLint location, GLfloat v0);
typedef void (*glGetUniform4fPtr) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (*glGetUniform1iPtr) (GLint location, GLint v0);

struct GLExtensions
{
	bool haveGLSL;

	glCreateProgramPtr glCreateProgram;
	glDeleteProgramPtr glDeleteProgram;
	glAttachShaderPtr glAttachShader;
	glLinkProgramPtr glLinkProgram;
	glUseProgramPtr glUseProgram;
	glCreateShaderPtr glCreateShader;
	glDeleteShaderPtr glDeleteShader;
	glShaderSourcePtr glShaderSource;
	glCompileShaderPtr glCompileShader;
	glGetShaderInfoLogPtr glGetShaderInfoLog;
	glGetProgramInfoLogPtr glGetProgramInfoLog;
	glGetShaderivPtr glGetShaderiv;
	glGetProgramivPtr glGetProgramiv;
	glGetUniformLocationPtr glGetUniformLocation;
	glGetUniform1fPtr glUniform1f;
	glGetUniform4fPtr glUniform4f;
	glGetUniform1iPtr glUniform1i;
};
