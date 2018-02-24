#pragma once

#include <vector>
#include <string>

#include <GL/glew.h>

#include <glm/glm.hpp>

void OGLCheck(std::string const & msg = {});

std::vector<char> GetTexture(GLuint id, size_t width, size_t height);

GLuint CompileShader(std::string const & shaderCode, GLenum type, std::string const & name);
void CheckCompileStatus(GLuint shaderID);
GLuint LinkProgram(GLuint vertexShaderID, GLuint fragmentShaderID);
GLuint LoadShaders(std::string const & vertexShaderCode, std::string const & fragmentShaderCode);

glm::mat4 CreateSimpleMPVMatrix();
glm::mat4 CreateMPVMatrix();

std::pair<GLuint, GLuint> CreateFrameBuffer(size_t width, size_t height);
