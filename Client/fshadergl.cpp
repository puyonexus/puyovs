#include "fshadergl.h"
#include "glmanager.h"

FShaderGL::FShaderGL(const char* source, GLExtensions& ext, QObject* parent)
	: QObject(parent), ext(ext), program(0), currentTex(-1)
{
	if (source)
		setSource(source);
}

FShaderGL::~FShaderGL()
{
	// Note: this will probably crash if called from the widget dtor.
	if (program)
		ext.glDeleteProgram(program);
}

bool FShaderGL::setSource(const char* src)
{
	source = src;

	return compile();
}

bool FShaderGL::setParameter(const char* param, double value)
{
	bool result;
	if (program)
	{
		GLint currentProgram = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

		ext.glUseProgram(program);
		GLint location = ext.glGetUniformLocation(program, param);
		if (location != -1)
			ext.glUniform1f(location, value);
		else
			result = false;


		ext.glUseProgram(currentProgram);
	}
	else result = false;
	return result;
}

bool FShaderGL::setParameter(const char* param, double x, double y, double z, double w)
{
	bool result;
	if (program)
	{
		GLint currentProgram = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

		ext.glUseProgram(program);
		GLint location = ext.glGetUniformLocation(program, param);
		if (location != -1)
			ext.glUniform4f(location, x, y, z, w);
		else
			result = false;


		ext.glUseProgram(currentProgram);
	}
	else result = false;
	return result;
}

bool FShaderGL::setCurrentTexture(const char* param)
{
	if (!program) return false;

	currentTex = ext.glGetUniformLocation(program, param);
	return currentTex != -1;
}

bool FShaderGL::compile()
{
	static const char* vertexSource =
		"void main()"
		"{"
		"    gl_TexCoord[0] = gl_MultiTexCoord0;"
		"    gl_FrontColor = gl_Color;"
		"    gl_Position = ftransform();"
		"}";

	program = 0;
	currentTex = -1;
	GLint success = 0;
	GLuint vertexShader = ext.glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = ext.glCreateShader(GL_FRAGMENT_SHADER);

	// Create and compile GLSL shaders.
	const char* fragmentSource = source.c_str();
	ext.glShaderSource(vertexShader, 1, &vertexSource, nullptr);
	ext.glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
	ext.glCompileShader(vertexShader);
	ext.glCompileShader(fragmentShader);

	// Check for compilation errors.
	ext.glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		char log[4096] = { 0 };
		ext.glGetShaderInfoLog(vertexShader, sizeof log, nullptr, log);
		QMessageBox(QMessageBox::Warning, "Puyo VS", QString("GLSL Error: \n") + log + QString("\nGL error: ") + QString::number(glGetError()), QMessageBox::Ok).exec();

		ext.glDeleteShader(vertexShader);
		ext.glDeleteShader(fragmentShader);
		return false;
	}
	ext.glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		char log[4096] = { 0 };
		ext.glGetShaderInfoLog(fragmentShader, sizeof log, nullptr, log);
		QMessageBox(QMessageBox::Warning, "Puyo VS", QString("GLSL Error: \n") + log, QMessageBox::Ok).exec();

		ext.glDeleteShader(vertexShader);
		ext.glDeleteShader(fragmentShader);
		return false;
	}

	// Create and setup program.
	program = ext.glCreateProgram();
	ext.glAttachShader(program, vertexShader);
	ext.glAttachShader(program, fragmentShader);
	ext.glDeleteShader(vertexShader);
	ext.glDeleteShader(fragmentShader);
	ext.glLinkProgram(program);
	ext.glGetShaderiv(program, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		char log[4096] = { 0 };
		ext.glGetShaderInfoLog(program, sizeof log, nullptr, log);
		QMessageBox(QMessageBox::Warning, "Puyo VS", QString("GLSL Error: \n") + log, QMessageBox::Ok).exec();

		ext.glDeleteProgram(program);
		program = 0;
		return false;
	}

	return true;
}

void FShaderGL::bind()
{
	if (program)
	{
		ext.glUseProgram(program);

		if (currentTex != -1)
			ext.glUniform1i(currentTex, 0);
	}
}

void FShaderGL::unbind()
{
	ext.glUseProgram(0);
}
