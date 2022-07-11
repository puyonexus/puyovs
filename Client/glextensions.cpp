#include "glextensions.h"

void loadExtensions(const QGLContext* gl, GLExtensions* ext)
{
	/* GLSL extensions */
	ext->glCreateProgram = (glCreateProgramPtr)gl->getProcAddress("glCreateProgram");
	ext->glDeleteProgram = (glDeleteProgramPtr)gl->getProcAddress("glDeleteProgram");
	ext->glAttachShader = (glAttachShaderPtr)gl->getProcAddress("glAttachShader");
	ext->glLinkProgram = (glLinkProgramPtr)gl->getProcAddress("glLinkProgram");
	ext->glUseProgram = (glUseProgramPtr)gl->getProcAddress("glUseProgram");

	ext->glCreateShader = (glCreateShaderPtr)gl->getProcAddress("glCreateShader");
	ext->glDeleteShader = (glDeleteShaderPtr)gl->getProcAddress("glDeleteShader");
	ext->glShaderSource = (glShaderSourcePtr)gl->getProcAddress("glShaderSource");
	ext->glCompileShader = (glCompileShaderPtr)gl->getProcAddress("glCompileShader");

	ext->glGetShaderInfoLog = (glGetShaderInfoLogPtr)gl->getProcAddress("glGetShaderInfoLog");
	ext->glGetProgramInfoLog = (glGetProgramInfoLogPtr)gl->getProcAddress("glGetProgramInfoLog");
	ext->glGetShaderiv = (glGetShaderivPtr)gl->getProcAddress("glGetShaderiv");
	ext->glGetProgramiv = (glGetProgramivPtr)gl->getProcAddress("glGetProgramiv");

	ext->glGetUniformLocation = (glGetUniformLocationPtr)gl->getProcAddress("glGetUniformLocation");
	ext->glUniform1f = (glGetUniform1fPtr)gl->getProcAddress("glUniform1f");
	ext->glUniform4f = (glGetUniform4fPtr)gl->getProcAddress("glUniform4f");
	ext->glUniform1i = (glGetUniform1iPtr)gl->getProcAddress("glUniform1i");

	ext->haveGLSL = ext->glCreateProgram && ext->glDeleteProgram && ext->glAttachShader && ext->glLinkProgram &&
		ext->glUseProgram && ext->glCreateShader && ext->glDeleteShader && ext->glShaderSource && ext->glCompileShader &&
		ext->glGetShaderInfoLog && ext->glGetProgramInfoLog && ext->glGetShaderiv && ext->glGetProgramiv &&
		ext->glGetUniformLocation && ext->glUniform1f && ext->glUniform4f && ext->glUniform1i;

	if (ext->haveGLSL)
		qDebug("GLSL support detected.");
	else
		qDebug("GLSL support not detected.");
}
