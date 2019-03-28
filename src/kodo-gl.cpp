#include "kodo-gl.hpp"

#include "..\assets\PatuaOne.hpp"

#include "VertexBuffer.hpp"

#include "Shaders.hpp"
#include "Shader.hpp"
#include "Texture.h"
#include "Brush.hpp"

#include "WindowContext.hpp"
#include "Window.hpp"

using namespace kodogl;

//std::unique_ptr<Atlas> atlas;

std::unique_ptr<Texture> tex;

std::vector<std::unique_ptr<Brush>> brushes;
std::vector<std::unique_ptr<Texture>> textures;
std::vector<std::unique_ptr<Window>> windows;

typedef void(*KodoGLErrorCallback)(const char*);

KodoGLErrorCallback kodoglError = nullptr;

bool isKodoGLInitialized = false;

typedef LRESULT(CALLBACK * Win32WndProc)(HWND, UINT, WPARAM, LPARAM);
LRESULT(CALLBACK* glfwWndProc) (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK glfwWndProcOverride(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	/*if (msg == WM_NCCALCSIZE && wparam == 1)
	{
	MONITORINFO info{ sizeof( MONITORINFO ) };
	GetMonitorInfo( MonitorFromWindow( hwnd, MONITOR_DEFAULTTONEAREST ), &info );

	auto calcSizeParams = reinterpret_cast<NCCALCSIZE_PARAMS*>(lparam);

	if (calcSizeParams->rgrc[0].left < info.rcWork.left)
	calcSizeParams->rgrc[0] = info.rcWork;

	return WVR_ALIGNLEFT | WVR_ALIGNTOP;
	}*/

	return CallWindowProc(glfwWndProc, hwnd, msg, wparam, lparam);
}

#ifdef _DEBUG

void OnGLFWError(int, const char* message)
{
	printf_s("%s", message);
	throw kodogl::exception(message);
}

void CALLBACK OnOpenGLError(GLenum src, GLenum type, GLuint id, GLenum severity, GLsizei len, const GLchar* message, const void* usr)
{
	printf_s("%s", message);
	throw kodogl::exception(message);
}

#endif

static Window* ToWindow(GLFWwindow* glfwWindow)
{
	return reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
}

void OnWindowRefresh(GLFWwindow* glfwWindow)
{
	ToWindow(glfwWindow)->OnRefresh();
}

void OnWindowMouseContained(GLFWwindow* glfwWindow, glm::int32 contained)
{
	ToWindow(glfwWindow)->OnMouseContained(contained > 0);
}

void OnWindowMouseMove(GLFWwindow* glfwWindow, double x, double y)
{
	ToWindow(glfwWindow)->OnMouseMove(static_cast<glm::float32>(x), static_cast<glm::float32>(y));
}

void OnWindowPositionChanged(GLFWwindow* glfwWindow, int32_t x, int32_t y)
{
	ToWindow(glfwWindow)->OnPositionChanged(x, y);
}

void OnWindowSizeChanged(GLFWwindow* glfwWindow, int32_t width, int32_t height)
{
	if (width == 0 || height == 0)
		return;

	ToWindow(glfwWindow)->OnSizeChanged(width, height);
}

#define EXPORT __declspec(dllexport)

extern "C"
{
	// --------------------------------------------------------------------------------
	//
	// System exports.
	//
	// --------------------------------------------------------------------------------

	EXPORT void KodoGLSystemCreate(KodoGLErrorCallback cb)
	{
		kodoglError = cb;

		if (!glfwInit())
		{
		}
	}

	EXPORT void KodoGLTerminate()
	{
		for (auto& window : windows)
		{
			glfwDestroyWindow(window->GLFWPointer());
		}

		glfwTerminate();
	}

	EXPORT double KodoGLGetTime() { return glfwGetTime(); }
	EXPORT void KodoGLSetTime(double time) { glfwSetTime(time); }
	EXPORT void KodoGLPollEvents() { glfwPollEvents(); }
	EXPORT void KodoGLWaitEvents() { glfwWaitEvents(); }

	// --------------------------------------------------------------------------------
	//
	// Window exports.
	//
	// --------------------------------------------------------------------------------

	EXPORT Window* KodoGLWindowCreate(int width, int height, int flags, const char* title)
	{
		constexpr auto KodoGLWindowVisible = 1;
		constexpr auto KodoGLWindowDecorated = 2;
		constexpr auto KodoGLWindowResizable = 4;

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
#ifdef _DEBUG
		//glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 0);
#else 
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 0);
#endif

		glfwWindowHint(GLFW_VISIBLE, 0);
		glfwWindowHint(GLFW_DECORATED, flags & KodoGLWindowDecorated);
		glfwWindowHint(GLFW_RESIZABLE, flags & KodoGLWindowResizable);

		auto glfwWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);

		if (!glfwWindow)
		{
			return nullptr;
		}

		glfwMakeContextCurrent(glfwWindow);
		glfwSetFramebufferSizeCallback(glfwWindow, OnWindowSizeChanged);
		glfwSetWindowPosCallback(glfwWindow, OnWindowPositionChanged);
		glfwSetCursorEnterCallback(glfwWindow, OnWindowMouseContained);
		glfwSetWindowRefreshCallback(glfwWindow, OnWindowRefresh);
		glfwSetCursorPosCallback(glfwWindow, OnWindowMouseMove);

		glfwWndProc = (Win32WndProc)GetWindowLongPtr(glfwGetWin32Window(glfwWindow), GWL_WNDPROC);
		SetWindowLongPtr(glfwGetWin32Window(glfwWindow), GWL_WNDPROC, (LONG)glfwWndProcOverride);

		if (!isKodoGLInitialized)
		{
			gl::sys::LoadFunctions();
			isKodoGLInitialized = true;
		}

		windows.emplace_back(std::make_unique<Window>(glfwWindow));

		glfwSetWindowUserPointer(glfwWindow, windows.back().get());

#ifdef _DEBUG
		//gl::Enable(gl::DEBUG_OUTPUT_SYNCHRONOUS);
		//gl::DebugMessageCallback(&OnOpenGLError, nullptr);
#endif

		/*AtlasLoader atlasLoader{ 512, 512 };
		atlasLoader.Load( PatuaOneDotTTF, sizeof( PatuaOneDotTTF ), 20 );
		atlasLoader.Load( PatuaOneDotTTF, sizeof( PatuaOneDotTTF ), 14, "0123456789.,e+-" );
		auto usedPercentage = atlasLoader.UsedOfTexture();
		atlas = atlasLoader.Finish();*/

		if (flags & KodoGLWindowVisible)
		{
			glfwShowWindow(glfwWindow);
		}

		return windows.back().get();
	}

	EXPORT void KodoGLWindowSwapInterval(Window* window, int interval) { glfwSwapInterval(interval); }
	EXPORT void KodoGLWindowSetRefreshCallback(Window* window, RefreshCallback cb) { window->SetRefreshCallback(cb); }
	EXPORT void KodoGLWindowSetMouseContainedCallback(Window* window, MouseContainedCallback cb) { window->SetMouseContainedCallback(cb); }
	EXPORT void KodoGLWindowSetMousePositionCallback(Window* window, MouseMoveCallback cb) { window->SetMouseMoveCallback(cb); }
	EXPORT void KodoGLWindowSetSizeCallback(Window* window, SizeChangedCallback cb) { window->SetSizeChangedCallback(cb); }
	EXPORT void KodoGLWindowSetSize(Window* window, int width, int height) { glfwSetWindowSize(window->GLFWPointer(), width, height); }

	EXPORT void KodoGLWindowSetVisible(Window* window, int visible)
	{
		if (visible)
		{
			glfwShowWindow(window->GLFWPointer());
		}
		else
		{
			glfwHideWindow(window->GLFWPointer());
		}
	}

	EXPORT int KodoGLWindowShouldClose(Window* window) { return glfwWindowShouldClose(window->GLFWPointer()); }
	EXPORT void KodoGLWindowDestroy(Window* window) { glfwDestroyWindow(window->GLFWPointer()); }
	EXPORT void KodoGLWindowFrameBegin(Window* window) { window->BeginFrame(); }
	EXPORT void KodoGLWindowFrameEnd(Window* window) { window->EndFrame(); }

	// --------------------------------------------------------------------------------
	//
	// WindowContext exports.
	//
	// --------------------------------------------------------------------------------

	EXPORT WindowContext* KodoGLDrawingContextCreate(Window* window)
	{
		auto newContext = std::make_unique<WindowContext>(window);
		return window->AddContext(std::move(newContext));
	}

	EXPORT void KodoGLDrawingContextGetArea(WindowContext* ctx, glm::vec4* bounds) { *bounds = ctx->Area(); }
	EXPORT void KodoGLDrawingContextSetArea(WindowContext* ctx, glm::vec4 bounds) { ctx->Area(bounds); }
	EXPORT void KodoGLDrawingContextDrawQuads(WindowContext* ctx, glm::vec4* quads, int quadsLength, Brush* brush) { ctx->DrawQuads(quads, quadsLength, brush); }
	EXPORT void KodoGLDrawingContextDrawQuad(WindowContext* ctx, glm::vec4 quad, Brush* brush) { ctx->DrawQuad(quad, brush); }
	EXPORT void KodoGLDrawingContextPopLayer(WindowContext* ctx) { ctx->PopLayer(); }
	EXPORT void KodoGLDrawingContexPushLayer(WindowContext* ctx) { ctx->PushLayer(); }

	// --------------------------------------------------------------------------------
	//
	// Texture exports.
	//
	// --------------------------------------------------------------------------------

	EXPORT Texture* KodoGLTextureCreate(const char* filename, int opacityOnly)
	{
		textures.emplace_back(std::make_unique<Texture>(filename, opacityOnly > 0));
		return textures.back().get();
	}

	// --------------------------------------------------------------------------------
	//
	// Brush exports.
	//
	// --------------------------------------------------------------------------------

	EXPORT Brush* KodoGLBrushCreateColor(glm::vec4 color)
	{
		auto newBrush = std::make_unique<ColorBrush>(color);
		brushes.emplace_back(std::move(newBrush));
		return brushes.back().get();
	}

	EXPORT Brush* KodoGLBrushCreateGradient(glm::vec4 color0, glm::vec4 color1)
	{
		auto newBrush = std::make_unique<ColorBrush>(color0, color1);
		brushes.emplace_back(std::move(newBrush));
		return brushes.back().get();
	}
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
	}

	return TRUE;
}