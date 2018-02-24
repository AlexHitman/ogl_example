#include <iostream>
#include <set>

// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW. Always include it before gl.h and glfw.h, since it's a bit magic.
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include "imgtools.h"

namespace std {
	bool operator<(const glm::vec2 & left, const glm::vec2 & right)
	{
		return left.x < right.x ? true : left.y < right.y;
	}
}

namespace {
	std::string const g_vertexShaderCode = R"(
			#version 330 core

			layout(location = 0) in vec3 vertexPosition_modelspace;

			uniform mat4 MVP;

			void main()
			{
				gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
			}
		)";
	const std::string g_fragmentShaderCode = R"(
			#version 330 core

			out vec3 color;
			void main()
			{
				color = vec3(1,0,0);
			}
		)";

	std::string const g_vertexShaderCode2 = R"(
			#version 330 core

			layout(location = 0) in vec3 vertexPosition_modelspace;
			layout(location = 1) in vec2 vertexUV;

			out vec2 UV;

			uniform mat4 MVP;

			void main()
			{
				gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
				UV = vertexUV;
			}
		)";
	const std::string g_fragmentShaderCode2 = R"(
			#version 330 core

			in vec2 UV;

			out vec3 color;

			uniform sampler2D inSampler;

			void main()
			{
				color = texture(inSampler, UV).rgb;
			}
		)";


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

	void GenerateBuffers(std::vector<glm::vec3> & vertexBufferData,
						 std::vector<glm::vec2> & uvBufferData,
						 std::vector<GLushort> & indexBufferData)
	{
		vertexBufferData.clear();
		uvBufferData.clear();
		indexBufferData.clear();

		const float xShift = 0.007f;

		const size_t xStepCount = 10;
		const size_t yStepCount = 10;

		const float xvStep = 2.0f / xStepCount;
		const float yvStep = 2.0f / yStepCount;

		const float xuvStep = 1.0f / xStepCount;
		const float yuvStep = 1.0f / yStepCount;

		for (size_t xIndex = 0; xIndex <= xStepCount; ++xIndex)
			for (size_t yIndex = 0; yIndex <= yStepCount; ++yIndex)
			{
				vertexBufferData.push_back({-1.0f + xIndex * xvStep, -1.0f + yIndex * yvStep, 0.0f});
				uvBufferData.push_back({0.0f + xIndex * xuvStep + xShift, 0.0f + yIndex * yuvStep});
			}

		for (size_t xIndex = 0; xIndex < xStepCount; ++xIndex)
			for (size_t yIndex = 0; yIndex < yStepCount; ++yIndex)
			{
				const GLushort tli = xIndex + yIndex * (xStepCount + 1);
				const GLushort tri = tli + 1;
				const GLushort bli = xIndex + (yIndex + 1) * (xStepCount + 1);
				const GLushort bri = bli + 1;
				indexBufferData.insert(indexBufferData.end(), {bli, tli, tri, bli, tri, bri});
			}
	}

	glm::vec2 sphere2fish(glm::vec2 coord)
	{
		const float FOV = glm::pi<float>(); // 180 degrees

		const float longitude = glm::two_pi<float>() * (coord.x / 2/* - 0.5f*/);
		const float latitude  = glm::pi<float>() * (coord.y / 2/* - 0.5f*/);

		const glm::vec3 vec3d = {
			glm::cos(latitude) * glm::sin(longitude),
			glm::cos(latitude) * glm::cos(longitude),
			glm::sin(latitude)
		};

		const float theta = glm::atan(vec3d.z, vec3d.x);
		const float phi = glm::atan(glm::sqrt(vec3d.x * vec3d.x + vec3d.z * vec3d.z), vec3d.y);
		const float r = phi / FOV;

		const glm::vec2 fishCoord {
			0.5 + r * glm::cos(theta),
			0.5 + r * glm::sin(theta)
		};

		return fishCoord;
	}

	void Generate360Buffers(std::vector<glm::vec3> & vertexBufferData,
						 std::vector<glm::vec2> & uvBufferData,
						 std::vector<GLushort> & indexBufferData)
	{
		vertexBufferData.clear();
		uvBufferData.clear();
		indexBufferData.clear();

		const float xShift = 0.007f;

		const size_t xStepCount = 250;
		const size_t yStepCount = 250;

		const float xvStep = 1.0f / xStepCount;
		const float yvStep = 2.0f / yStepCount;

		const float xuvStep = 1.0f / xStepCount;
		const float yuvStep = 1.0f / yStepCount;

//		std::set<glm::vec2> f;
//		std::set<glm::vec2> s;
		for (size_t xIndex = 0; xIndex <= xStepCount; ++xIndex)
			for (size_t yIndex = 0; yIndex <= yStepCount; ++yIndex)
			{
				const glm::vec2 sphereCoord{-0.5f + xIndex * xvStep, -1.0f + yIndex * yvStep};
				const glm::vec2 fishCoord = sphere2fish(sphereCoord);

//				f.insert(fishCoord);
//				s.insert(sphereCoord);

				vertexBufferData.push_back({sphereCoord.x, sphereCoord.y, 0.0f});
				uvBufferData.push_back({fishCoord.x + xShift, fishCoord.y});
			}

		for (size_t xIndex = 0; xIndex < xStepCount; ++xIndex)
			for (size_t yIndex = 0; yIndex < yStepCount; ++yIndex)
			{
				const GLushort tli = xIndex + yIndex * (xStepCount + 1);
				const GLushort tri = tli + 1;
				const GLushort bli = xIndex + (yIndex + 1) * (xStepCount + 1);
				const GLushort bri = bli + 1;
				indexBufferData.insert(indexBufferData.end(), {bli, tli, tri, bli, tri, bri});
			}
	}
}

int main(int, char**)
{
	RawImage const inTex = RawImage::LoadFromBMP("/home/alex/360/cube_orig.bmp");
//	RawImage const inTex = RawImage::LoadFromBMP("/home/alex/360/fish2sphere180.bmp");

	// Initialise GLFW
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL

	// Open a window and create its OpenGL context
	GLFWwindow * window; // (In the accompanying source code, this variable is global for simplicity)
	window = glfwCreateWindow(1200, 600, "Windows name", nullptr, nullptr);
	if (window == nullptr) {
		std::cerr << "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window); // Initialize GLEW
	glewExperimental = true; // Needed in core profile
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

//	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	GLuint vertexArrayId;
	glGenVertexArrays(1, &vertexArrayId);
	glBindVertexArray(vertexArrayId);

	GLuint programId = LoadShaders(g_vertexShaderCode2, g_fragmentShaderCode2);
//	const GLfloat vertexBufferData[] = {
//		-1.0f, -1.0f, 0.0f,
//		-1.0f,  1.0f, 0.0f,
//		 1.0f,  1.0f, 0.0f,
//		 1.0f, -1.0f, 0.0f
//	};

	std::vector<glm::vec3> vertexBufferData;
	std::vector<glm::vec2> uvBufferData;
	std::vector<GLushort> indexBufferData;

	Generate360Buffers(vertexBufferData, uvBufferData, indexBufferData);

	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexBufferData.size() * sizeof(glm::vec3), vertexBufferData.data(), GL_STATIC_DRAW);

//	float const xShift = 0.007f;
//	const GLfloat uvBufferData[] = {
//		0.0f + xShift, 0.0f,
//		0.0f + xShift, 1.0f,
//		1.0f + xShift, 1.0f,
//		1.0f + xShift, 0.0f
//	};

	GLuint inTexbuffer;
	glGenBuffers(1, &inTexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, inTexbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvBufferData.size() * sizeof(glm::vec2), uvBufferData.data(), GL_STATIC_DRAW);

//	const GLushort indexBufferData[] = {
//		0, 1, 3,
//		1, 3, 2
//	};

	GLuint indexBuffer;
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferData.size() * sizeof(GLushort), indexBufferData.data(), GL_STATIC_DRAW);

	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, inTex.GetWidth(), inTex.GetHeight(), 0, GL_BGR, GL_UNSIGNED_BYTE, inTex.GetData());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	GLuint samplerID = glGetUniformLocation(programId, "inSampler");

	glm::mat4 const mvp = CreateMPVMatrix();
	GLuint const mvpId = glGetUniformLocation(programId, "MVP");
	glUniformMatrix4fv(mvpId, 1, GL_FALSE, &mvp[0][0]);

	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(programId);

		glUniformMatrix4fv(mvpId, 1, GL_FALSE, &mvp[0][0]);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, inTexbuffer);
		glVertexAttribPointer(
			1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			2,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

		glDrawElements(
			GL_TRIANGLES,      // mode
			indexBufferData.size(),    // count
			GL_UNSIGNED_SHORT,   // type
			(void*)0           // element array buffer offset
);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	}
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &inTexbuffer);
	glDeleteBuffers(1, &indexBuffer);
	glDeleteProgram(programId);
	glDeleteTextures(1, &textureId);
	glDeleteVertexArrays(1, &vertexArrayId);

	glfwTerminate();
}
