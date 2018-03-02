#include "shaders.h"

std::string const g_vertexShaderCodeSimple = R"(
		#version 330 core

		layout(location = 0) in vec3 vertexPosition_modelspace;

		uniform mat4 MVP;

		void main()
		{
			gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
		}
	)";
std::string const g_fragmentShaderCodeSimple = R"(
		#version 330 core

		out vec3 color;
		void main()
		{
			color = vec3(1,0,0);
		}
	)";

std::string const g_vertexShaderCode360 = R"(
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
std::string const g_fragmentShaderCode360 = R"(
		#version 330 core

		in vec2 UV;

		out vec3 color;

		uniform sampler2D inSampler;

		void main()
		{
			color = texture(inSampler, UV).rgb;
		}
	)";

std::string const g_fragmentShaderCode360FB = R"(
		#version 330 core

		in vec2 UV;

		layout(location = 0) out vec3 color;

		uniform sampler2D inSampler;

		void main()
		{
			color = texture(inSampler, UV).rgb;
		}
	)";

std::string const g_fragmentShaderCode360FBCut = R"(
		#version 330 core

		in vec2 UV;

		layout(location = 0) out vec3 color;

		uniform sampler2D inSampler;

		void main()
		{
			if (all(lessThanEqual(UV, vec2(1.0f))) && all(greaterThanEqual(UV, vec2(0.0f))))
				color = texture(inSampler, UV).rgb;
			else
				color = vec3(0.0f);
		}
	)";
