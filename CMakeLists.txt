cmake_minimum_required(VERSION 2.8)
project(ogl)

file(GLOB src
	"*.h"
	"*.cpp"
)
add_executable(${PROJECT_NAME} ${src})
target_link_libraries(${PROJECT_NAME}
	glfw
	GL
	GLEW
)
