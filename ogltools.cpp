#include "ogltools.h"

#include <iostream>

void OGLCheck(std::string const & msg)
{
	GLenum const error = glGetError();
	if (error != GL_NO_ERROR)
		throw std::runtime_error(msg);
}

std::vector<GLenum> GetAllOGLErrors()
{
	std::vector<GLenum> errors;
	GLenum error = GL_NO_ERROR;
	while ((error = glGetError()) != GL_NO_ERROR)
		errors.push_back(error);
	return errors;
}

std::vector<char> GetFBTexture(size_t width, size_t height)
{
	std::vector<char> tex(3 * width * height);

	glReadPixels(0, 0, width, height, GL_RGB, GL_BYTE, tex.data());
	OGLCheck("Failed to get texture!");

	return tex;
}

GLuint CompileShader(std::string const & shaderCode, GLenum type, std::string const & name)
{
	GLuint shaderID = glCreateShader(type);

	std::cerr << "Compiling " << name << " shader..." << std::endl;
	char const * sourcePtr = shaderCode.c_str();
	glShaderSource(shaderID, 1, &sourcePtr, nullptr);
	glCompileShader(shaderID);

	return shaderID;
}

void CheckCompileStatus(GLuint shaderID)
{
	GLint res = GL_FALSE;
	int infoLogLength = 0;

	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &res);
	glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		std::string shaderErrorMessage(infoLogLength + 1, 0);
		glGetShaderInfoLog(shaderID, infoLogLength, nullptr, &shaderErrorMessage[0]);
		std::cerr << shaderErrorMessage << std::endl;
	}
}

GLuint LinkProgram(GLuint vertexShaderID, GLuint fragmentShaderID)
{
	GLint res = GL_FALSE;
	int infoLogLength = 0;

	std::cerr << "Linking program..." << std::endl;
	GLuint programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	// Check the program
	glGetProgramiv(programID, GL_LINK_STATUS, &res);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		std::string programErrorMessage(infoLogLength + 1, 0);
		glGetProgramInfoLog(programID, infoLogLength, nullptr, &programErrorMessage[0]);
		std::cerr << programErrorMessage << std::endl;
	}

	glDetachShader(programID, vertexShaderID);
	glDetachShader(programID, fragmentShaderID);

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	return programID;
}

GLuint LoadShaders(std::string const & vertexShaderCode, std::string const & fragmentShaderCode)
{
	GLuint vertexShaderID = CompileShader(vertexShaderCode, GL_VERTEX_SHADER, "vertex");
	CheckCompileStatus(vertexShaderID);

	GLuint fragmentShaderID = CompileShader(fragmentShaderCode, GL_FRAGMENT_SHADER, "fragment");
	CheckCompileStatus(fragmentShaderID);

	return LinkProgram(vertexShaderID, fragmentShaderID);
}

glm::mat4 CreateMPVMatrix()
{
	float const width = 600.;
	float const height = 600.;

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
//		glm::mat4 const projection = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);
	glm::mat4 const projection = glm::mat4(1.0f);

	// Or, for an ortho camera :
	//glm::mat4 const projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

	// Camera matrix
//		glm::mat4 const view = glm::lookAt(
//				glm::vec3(4,3,3), // Camera is at (4,3,3), in World Space
//				glm::vec3(0,0,0), // and looks at the origin
//				glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
//			);
	glm::mat4 const view = glm::mat4(1.0f);

	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 const model = glm::mat4(1.0f);

	// Our ModelViewProjection : multiplication of our 3 matrices
	return projection * view * model; // Remember, matrix multiplication is the other way around
}
glm::mat4 CreateSimpleMPVMatrix()
{
	return glm::mat4(1.0f);
}

std::pair<GLuint, GLuint> CreateFrameBuffer(size_t width, size_t height)
{
	GLuint frameBufferId = 0;
	glGenFramebuffers(1, &frameBufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);

	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLuint depthRenderBuffer;
	glGenRenderbuffers(1, &depthRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureId, 0);

	GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, drawBuffers);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw std::runtime_error("Failed to set up frame buffer!");

	return std::make_pair(frameBufferId, textureId);
}

