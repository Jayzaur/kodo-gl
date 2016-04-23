#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <array>
#include <vector>
#include <memory>
#include <map>
#include <assert.h>

#pragma comment (lib, "opengl32")
#include <gl_core_4_4.hpp>

#pragma comment (lib, "deps/freetype/freetype263")
#include <ft2build.h>
#include FT_FREETYPE_H



#pragma comment (lib, "deps/GLFW/glfw3")
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>

#pragma comment(lib, "deps/libpng/libpng16")
#include <libpng/png.h>

#define GLM_FORCE_CXX11
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace kodogl
{
	class exception : public std::exception
	{
		std::string message;
	public:
		explicit exception( std::string message ) : std::exception( message.c_str() ), message( message ) {}
	};

	template<typename T>
	static T round( T value, int digits )
	{
		return static_cast<T>(glm::round( value * glm::pow( 10, digits ) ) / glm::pow( 10, digits ));
	}
}
