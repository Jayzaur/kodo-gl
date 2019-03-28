#pragma once

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>
#include <array>
#include <vector>
#include <memory>
#include <unordered_map>
#include <cassert>

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
#include <glm/gtc/packing.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace kodogl
{
	class nocopy
	{
	protected:
		nocopy() = default;
		~nocopy() = default;

		nocopy( nocopy const & ) = delete;
		void operator=( nocopy const &x ) = delete;
	};

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
