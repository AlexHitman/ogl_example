#include <iostream>

// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW. Always include it before gl.h and glfw.h, since it's a bit magic.
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include "imgtools.h"
#include "shaders.h"
#include "ogltools.h"

//#define ONE_FISH
//#define SAVE_TO_FB

namespace std {
	bool operator<(const glm::vec2 & left, const glm::vec2 & right)
	{
		return left.x < right.x ? true : left.y < right.y;
	}
}

namespace {
	struct FishInfo
	{
		glm::vec2 m_center;
		glm::vec3 m_rotation;
		float m_fov;
		glm::vec2 m_ratio;
	};

	glm::vec2 sphere2fish(glm::vec2 coord, FishInfo const & fishInfo)
	{
		const float longitude = glm::two_pi<float>() * (coord.x / 2 - 0.5f);
		const float latitude  = glm::pi<float>() * coord.y / 2;

		const glm::vec3 vec3d = {
			glm::cos(latitude) * glm::sin(longitude),
			glm::cos(latitude) * glm::cos(longitude),
			glm::sin(latitude)
		};

		const float theta = glm::atan(vec3d.z, vec3d.x);
		const float phi = glm::atan(glm::sqrt(vec3d.x * vec3d.x + vec3d.z * vec3d.z), vec3d.y);
		const float r = phi / fishInfo.m_fov;

		if (r > 0.505f)
			return glm::vec2(2.0f, 2.0f);

		const glm::vec2 fishCoord {
			r * glm::cos(theta) * fishInfo.m_ratio.x,
			r * glm::sin(theta) * fishInfo.m_ratio.y
		};

		return fishCoord + fishInfo.m_center;
	}

	void GenerateOneFishBuffers(std::vector<glm::vec3> & vertexBufferData,
								std::vector<glm::vec2> & uvBufferData,
								std::vector<GLushort> & indexBufferData,
								FishInfo const & fishInfo)
	{
		vertexBufferData.clear();
		uvBufferData.clear();
		indexBufferData.clear();

		const size_t xStepCount = 240;
		const size_t yStepCount = 240;

		const float xvStep = 2.0f / xStepCount;
		const float yvStep = 2.0f / yStepCount;

		const float xuvStep = 1.0f / xStepCount;
		const float yuvStep = 1.0f / yStepCount;

		for (size_t xIndex = 0; xIndex <= xStepCount; ++xIndex)
			for (size_t yIndex = 0; yIndex <= yStepCount; ++yIndex)
			{
				const glm::vec2 sphereCoord{-1.0f + xIndex * xvStep, -1.0f + yIndex * yvStep};
				const glm::vec2 fishCoord = sphere2fish(sphereCoord, fishInfo);

				vertexBufferData.push_back({sphereCoord.x, sphereCoord.y, 0.0f});
				uvBufferData.push_back({fishCoord.x, fishCoord.y});
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

	glm::vec2 sphere2fish2(glm::vec2 const & coord, FishInfo const & fishInfo)
	{
		/*
		 *
		 * Z   Y
		 * |  /
		 * | /
		 * |/
		 * ------- X
		 *
		 */

		const float longitude = glm::two_pi<float>() * coord.x / 2.0f + fishInfo.m_rotation.z;
		const float latitude  = glm::pi<float>() * coord.y / -2.0f;

		glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f), fishInfo.m_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		rotateMat = glm::rotate(rotateMat, fishInfo.m_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));

		const glm::vec3 vec3d = rotateMat * glm::vec4(
			glm::cos(latitude) * glm::sin(longitude),
			glm::cos(latitude) * glm::cos(longitude),
			glm::sin(latitude),
			1
		);

		const float theta = glm::atan(vec3d.z, vec3d.x);
		const float phi = glm::atan(glm::sqrt(vec3d.x * vec3d.x + vec3d.z * vec3d.z), vec3d.y);
		const float r = phi / fishInfo.m_fov;

		if (r > 0.5001f)
			return glm::vec2(2.0f, 2.0f);

		const glm::vec2 fishCoord {
			r * glm::cos(theta) * fishInfo.m_ratio.x,
			r * glm::sin(theta) * fishInfo.m_ratio.y
		};

		return fishCoord + fishInfo.m_center;
	}


	void GenerateDualFishBuffers(std::vector<glm::vec3> & vertexBufferData,
								 std::vector<glm::vec2> & uvBufferData0,
								 std::vector<glm::vec2> & uvBufferData1,
								 std::vector<GLushort> & indexBufferData,
								 FishInfo const & fishInfo0,
								 FishInfo const & fishInfo1)
	{
		vertexBufferData.clear();
		uvBufferData0.clear();
		uvBufferData0.clear();
		indexBufferData.clear();

		const size_t xStepCount = 240;
		const size_t yStepCount = 240;

		const float xvStep = 2.0f / xStepCount;
		const float yvStep = 2.0f / yStepCount;

		const float xuvStep = 1.0f / xStepCount;
		const float yuvStep = 1.0f / yStepCount;

		for (size_t xIndex = 0; xIndex <= xStepCount; ++xIndex)
			for (size_t yIndex = 0; yIndex <= yStepCount; ++yIndex)
			{
				const glm::vec2 sphereCoord{-1.0f + xIndex * xvStep, -1.0f + yIndex * yvStep};
				const glm::vec2 fishCoord0 = sphere2fish2(sphereCoord, fishInfo0);
				const glm::vec2 fishCoord1 = sphere2fish2(sphereCoord, fishInfo1);

				vertexBufferData.push_back({sphereCoord.x, sphereCoord.y, 0.0f});
				uvBufferData0.push_back(fishCoord0);
				uvBufferData1.push_back(fishCoord1);
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
#ifdef ONE_FISH
//	RawImage const inTex = RawImage::LoadFromFile("/home/alex/360/cube_orig.bmp", 4096, 4096, glm::pi<float>());
//	FishInfo fishInfo0 = {glm::vec2(0.5f, 0.5f), glm::vec3(0.0f), glm::pi<float>(), glm::vec2(1.0f, 1.0f)};

	RawImage const inTex = RawImage::LoadFromFile("/home/alex/360/fish2sphere220.jpg", 4096, 4096);
	FishInfo fishInfo0 = {glm::vec2(0.5f, 0.5f), glm::vec3(0.0f), 11.0f * glm::pi<float>() / 9.0f, glm::vec2(1.0f, 1.0f)};
#else
//	RawImage const inTex = RawImage::LoadFromFile("/home/alex/360/dual.bmp", 8192, 4096);
//	FishInfo fishInfo0 = {glm::vec2(0.25f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::pi<float>(), glm::vec2(0.5f, 1.0f)};
//	FishInfo fishInfo1 = {glm::vec2(0.75f, 0.5f), glm::vec3(0.0f, 0.0f, glm::pi<float>()), glm::pi<float>(), glm::vec2(0.5f, 1.0f)};

	RawImage const inTex = RawImage::LoadFromFile("/home/alex/360/example.jpg", 4296, 2148);
	FishInfo fishInfo0 = {
		glm::vec2(1024.0f / 4296.0f, 1024.0f / 2148.0f),
		glm::vec3(glm::radians(25.0f), 0.0f, 0.0f),
		glm::radians(210.0f),
		glm::vec2(2048.0f / 4296.0f, 2048.0f / 2148.0f)};
	FishInfo fishInfo1 = {
		glm::vec2(3272.0f / 4296.0f, 1124.0f / 2148.0f),
		glm::vec3(0.0f, glm::radians(-5.0f), glm::pi<float>()),
		glm::radians(210.0f),
		glm::vec2(2048.0f / 4296.0f, 2048.0f / 2148.0f)};

#endif


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

	GLuint vertexArrayId;
	glGenVertexArrays(1, &vertexArrayId);
	glBindVertexArray(vertexArrayId);

#ifdef ONE_FISH
	GLuint programId = LoadShaders(g_vertexShaderCode360, g_fragmentShaderCode360FBCut);
#else
	GLuint programId = LoadShaders(g_vertexShaderCode360DualFish, g_fragmentShaderCode360FBCutDualFish);
#endif

	std::vector<glm::vec3> vertexBufferData;
	std::vector<glm::vec2> uvBufferData0;
#ifndef ONE_FISH
	std::vector<glm::vec2> uvBufferData1;
#endif
	std::vector<GLushort> indexBufferData;

#ifdef ONE_FISH
	GenerateOneFishBuffers(vertexBufferData, uvBufferData0, indexBufferData, fishInfo0);
#else
	GenerateDualFishBuffers(vertexBufferData, uvBufferData0, uvBufferData1, indexBufferData, fishInfo0, fishInfo1);
#endif

	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexBufferData.size() * sizeof(glm::vec3), vertexBufferData.data(), GL_STATIC_DRAW);

	GLuint inTexbuffer0;
	glGenBuffers(1, &inTexbuffer0);
	glBindBuffer(GL_ARRAY_BUFFER, inTexbuffer0);
	glBufferData(GL_ARRAY_BUFFER, uvBufferData0.size() * sizeof(glm::vec2), uvBufferData0.data(), GL_STATIC_DRAW);

#ifndef ONE_FISH
	GLuint inTexbuffer1;
	glGenBuffers(1, &inTexbuffer1);
	glBindBuffer(GL_ARRAY_BUFFER, inTexbuffer1);
	glBufferData(GL_ARRAY_BUFFER, uvBufferData1.size() * sizeof(glm::vec2), uvBufferData1.data(), GL_STATIC_DRAW);
#endif

	GLuint indexBuffer;
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferData.size() * sizeof(GLushort), indexBufferData.data(), GL_STATIC_DRAW);

	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, inTex.GetWidth(), inTex.GetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, inTex.GetData());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	GLuint samplerID = glGetUniformLocation(programId, "inSampler");

	glm::mat4 const mvp = CreateSimpleMPVMatrix();
	GLuint const mvpId = glGetUniformLocation(programId, "MVP");

#ifdef SAVE_TO_FB
	size_t const fbWidth = 1200;
	size_t const fbHeight = 600;

	auto const fbParams = CreateFrameBuffer(fbWidth, fbHeight);
#endif

	do {
#ifdef SAVE_TO_FB
		glBindFramebuffer(GL_FRAMEBUFFER, fbParams.first);
		glViewport(0, 0, fbWidth, fbHeight);
#endif

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(programId);

		glUniformMatrix4fv(mvpId, 1, GL_FALSE, &mvp[0][0]);

		// Don't forget to bind input texture back after working with fb
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glUniform1i(samplerID, 0);

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
		glBindBuffer(GL_ARRAY_BUFFER, inTexbuffer0);
		glVertexAttribPointer(
			1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			2,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

#ifndef ONE_FISH
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, inTexbuffer1);
		glVertexAttribPointer(
			2,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			2,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
#endif

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

		glDrawElements(
			GL_TRIANGLES,           // mode
			indexBufferData.size(), // count
			GL_UNSIGNED_SHORT,      // type
			(void*)0                // element array buffer offset
		);

#ifdef SAVE_TO_FB
		RawImage(GetFBTexture(fbWidth, fbHeight), "rgb24", fbWidth, fbHeight).SaveToFile("1.png");
#endif

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
#ifndef ONE_FISH
		glDisableVertexAttribArray(2);
#endif

#ifdef SAVE_TO_FB
		break;
#endif

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	}
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

	//TODO Delete fb stuff
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &inTexbuffer0);
#ifndef ONE_FISH
	glDeleteBuffers(1, &inTexbuffer1);
#endif
	glDeleteBuffers(1, &indexBuffer);
	glDeleteProgram(programId);
	glDeleteTextures(1, &textureId);
	glDeleteVertexArrays(1, &vertexArrayId);

	glfwTerminate();
}
