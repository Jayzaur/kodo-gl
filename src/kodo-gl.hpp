#pragma once

#include <assert.h>

#include <stdlib.h>
#include <stdio.h>
#include <vector>

#pragma comment (lib, "/freetype/freetype")

#include <ft2build.h>
#include FT_FREETYPE_H

#pragma comment (lib, "opengl32")

#include <gl_core_4_4.hpp>

#pragma comment (lib, "GLFW/glfw3")
#include <GLFW\glfw3.h>
//#define GLFW_EXPOSE_NATIVE_WIN32
//#define GLFW_EXPOSE_NATIVE_WGL
//#include <GLFW\glfw3native.h>

#include <string>
#include <array>
#include <vector>
#include <memory>
#include <map>
#include <assert.h>

template<typename T>
struct Vec3
{
	T x;
	T y;
	T z;

	Vec3() : x( 0 ), y( 0 ), z( 0 ) {}
	Vec3( T x, T y, T z ) : x( x ), y( y ), z( z ) {}
};

template<typename T>
struct Vec4
{
	T x;
	T y;
	T z;
	T w;

	Vec4() : x( 0 ), y( 0 ), z( 0 ), w( 0 ) {}
	Vec4( T x, T y, T z, T w ) : x( x ), y( y ), z( z ), w( w ) {}
};

typedef Vec3<int> Vec3i;
typedef Vec3<float> Vec3f;

typedef Vec4<int> Vec4i;
typedef Vec4<float> Vec4f;
