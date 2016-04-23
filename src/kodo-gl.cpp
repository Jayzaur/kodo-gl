#include "kodo-gl.hpp"

#include "Atlas.hpp"
#include "AtlasFont.hpp"

#include "VertexBuffer.hpp"
#include "Shader.hpp"
#include "Texture.h"

using namespace kodogl;

enum class UniformIds
{
	Texture,
	Model,
	View,
	Projection,
};

std::unique_ptr<ShaderProgram> textProgram;
std::unique_ptr<Texture> tex;

struct Vert
{
	glm::vec2 Vertex;
	glm::vec4 Color;

	Vert() {}
	Vert( float_t x, float_t y, const glm::vec4& c ) : Vert( glm::vec2( x, y ), c ) {}
	Vert( const glm::vec2& vp, const glm::vec4& c ) : Vertex( vp ), Color( c ) {}

	static std::array<VertexAttribute, 2> Attributes()
	{
		return{
			VertexAttribute{ 0, 2, gl::FLOAT, false, reinterpret_cast<GLvoid*>(offsetof( AtlasVertex, Vertex )) },
			VertexAttribute{ 1, 4, gl::FLOAT, false, reinterpret_cast<GLvoid*>(offsetof( AtlasVertex, Vertex )) }
		};
	}
};

class TextLayout
{
	VertexBuffer<AtlasVertex>& vertexBuffer;

	GLuint idOfVertices;

	glm::vec2 position;
	glm::vec4 color;
	glm::vec2 dimensions;

public:


	/// <summary>
	/// Gets the dimensions of the <see cref="TextLayout"/>.
	/// </summary>
	const glm::vec2& Dimensions() const
	{
		return dimensions;
	}

	/// <summary>
	/// Prevents copy-assignment of <see cref="TextLayout"/>s.
	/// </summary>
	/// <param name="other">The other.</param>
	/// <returns></returns>
	TextLayout& operator = ( const TextLayout& other ) = delete;

	/// <summary>
	/// Move-assigns the <see cref="TextLayout"/> from the right-hand side to this <see cref="TextLayout"/>.
	/// </summary>
	/// <param name="other">The other.</param>
	/// <returns></returns>
	TextLayout& operator = ( TextLayout&& other )
	{
		if (vertexBuffer.Name() != other.vertexBuffer.Name())
			throw kodogl::exception( "Can't move from a different vertex buffer." );

		Remove();

		color = other.color;
		position = other.position;
		dimensions = other.dimensions;
		idOfVertices = other.idOfVertices;

		other.idOfVertices = 0;

		return *this;
	}

	/// <summary>
	/// Prevents copy construction of <see cref="TextLayout"/>s.
	/// </summary>
	/// <param name="">The .</param>
	TextLayout( const TextLayout& ) = delete;

	/// <summary>
	/// Creates a new <see cref="TextLayout"/> moving from the other.
	/// </summary>
	/// <param name="other">The other.</param>
	TextLayout( TextLayout&& other ) :
		vertexBuffer( other.vertexBuffer )
	{
		color = other.color;
		position = other.position;
		dimensions = other.dimensions;
		idOfVertices = other.idOfVertices;

		other.idOfVertices = 0;
	}

	/// <summary>
	/// Creates a new <see cref="TextLayout"/>.
	/// </summary>
	/// <param name="text">The text.</param>
	/// <param name="font">The font.</param>
	/// <param name="vertexBuffer">The vertex buffer.</param>
	explicit TextLayout( const std::string& text, const AtlasFont& font, VertexBuffer<AtlasVertex>& vertexBuffer ) : TextLayout( text, font, vertexBuffer, glm::vec2(), glm::vec4() ) {}
	/// <summary>
	/// Creates a new <see cref="TextLayout"/>.
	/// </summary>
	/// <param name="text">The text.</param>
	/// <param name="font">The font.</param>
	/// <param name="vertexBuffer">The vertex buffer.</param>
	/// <param name="initialPosition">The initial position.</param>
	/// <param name="initialColor">The initial color.</param>
	explicit TextLayout( const std::string& text, const AtlasFont& font, VertexBuffer<AtlasVertex>& vertexBuffer,
						 const glm::vec2& initialPosition, const glm::vec4& initialColor ) :
		vertexBuffer( vertexBuffer ),
		position( initialPosition ), color( initialColor )
	{
		auto calculatedWidth = 0.0f;
		auto calculatedHeight = 0.0f;
		auto calculatedYOffset = 10000000.0f;

		auto textLocation = initialPosition;
		auto textColor = initialColor;

		std::vector<AtlasVertex> vertices;

		CodepointEnumerator codepoints{ text };

		while (codepoints)
		{
			auto codepoint = codepoints.Next();

			if (codepoint == 10) // Line Feed U+000A
			{
				textLocation.y -= font.BaselineToBaseline;
				textLocation.x = 0;
				continue;
			}

			auto glyph = font.GetGlyph( codepoint, true );
			auto x0 = textLocation.x + glyph.OffsetX;
			auto y0 = textLocation.y + glyph.OffsetY;
			auto x1 = x0 + glyph.Width;
			auto y1 = y0 - glyph.Height;
			auto s0 = glyph.Region.L;
			auto t0 = glyph.Region.T;
			auto s1 = glyph.Region.R;
			auto t1 = glyph.Region.B;

			vertices.emplace_back( x0, y0, s0, t0, textColor );
			vertices.emplace_back( x0, y1, s0, t1, textColor );
			vertices.emplace_back( x1, y1, s1, t1, textColor );
			vertices.emplace_back( x1, y0, s1, t0, textColor );

			textLocation.x += glyph.AdvanceX;
			textLocation.y += glyph.AdvanceY;

			calculatedWidth += glyph.AdvanceX;
			calculatedHeight = glm::max( glyph.Height, glyph.Height );
			calculatedYOffset = glm::min( calculatedYOffset, glyph.OffsetY - glyph.Height );
		}

		dimensions = glm::vec2( calculatedWidth, glm::abs( calculatedHeight ) + glm::abs( calculatedYOffset ) );

		idOfVertices = vertexBuffer.PushQuads( vertices );
	}

	/// <summary>
	/// Destroys the <see cref="TextLayout"/>.
	/// </summary>
	~TextLayout()
	{
		Remove();
	}

	/// <summary>
	/// Updates the position and color of the <see cref="TextLayout"/>.
	/// </summary>
	/// <param name="pos">The position.</param>
	/// <param name="col">The color.</param>
	void Update( const glm::vec2& pos, const glm::vec4& col )
	{
		if (position != pos || color != col)
		{
			auto vertices = vertexBuffer.At( idOfVertices );
			auto delta = pos - position;

			for (auto i = vertices.StartOfVertices; i < vertices.StartOfVertices + vertices.CountOfVertices; i++)
			{
				auto& v = vertexBuffer.VertexAt( i );
				v.Vertex += delta;
				v.Color = col;
			}

			position = pos;
			color = col;
		}
	}

private:

	void Remove()
	{
		vertexBuffer.Erase( idOfVertices );
		idOfVertices = 0;
	}
};

class FrameGeometry
{
	VertexBuffer<Vert> vertexBuffer;
};

class DrawingContext
{
	glm::vec4 area;

public:

	explicit DrawingContext( const glm::vec4& area ) :
		area( area )
	{
	}

	void Begin() const
	{
		gl::Scissor( static_cast<GLint>(area.x), static_cast<GLint>(area.y), static_cast<GLsizei>(area.z), static_cast<GLsizei>(area.w) );
	}

	void Clear()
	{

	}
};

std::unique_ptr<ShaderProgram> testProgram;
std::unique_ptr<VertexBuffer<AtlasVertex>> testVBuffer;

std::unique_ptr<Atlas> atlas;
std::unique_ptr<VertexBuffer<AtlasVertex>> textVertexBuffer;

glm::vec2 globalViewport;
glm::mat4 globalProjection;

glm::vec4 clearColor{ 0.1, 0.15, 0.2, 1 };
glm::vec4 textColor{ 1, 1, 1, 1 };

void OnGLFWResize( GLFWwindow*, int32_t width, int32_t height )
{
	if (width == 0 || height == 0)
		return;

	globalViewport = glm::vec2( width, height );
	globalProjection = glm::ortho( 0.0f, globalViewport.x, 0.0f, globalViewport.y );

	gl::Viewport( 0, 0, static_cast<GLsizei>(globalViewport.x), static_cast<GLsizei>(globalViewport.y) );
	gl::Scissor( 0, 0, static_cast<GLsizei>(globalViewport.x), static_cast<GLsizei>(globalViewport.y) );

	textProgram->Use();
	textProgram->Get( UniformIds::Projection ).Set( globalProjection );

	testProgram->Use();
	testProgram->Get( UniformIds::Projection ).Set( globalProjection );
}

void OnGLFWKeyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose( window, true );
	}
}

void OnGLFWRender( GLFWwindow* window )
{
	gl::Disable( gl::SCISSOR_TEST );

	gl::ClearColor( clearColor.r, clearColor.g, clearColor.b, clearColor.a );
	gl::Clear( gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT );

	gl::Enable( gl::SCISSOR_TEST );
	gl::Enable( gl::BLEND );
	gl::BlendFunc( gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA );

	atlas->Bind();
	textProgram->Use();
	textVertexBuffer->Render();

	tex->Bind();
	testProgram->Use();
	testVBuffer->Render();

	glfwSwapBuffers( window );
}

void CreateTestVBuffer()
{
	std::vector<Shader> shaders;
	shaders.emplace_back( Shader::CreateVertex( Shader::Source( "assets/vertex-texture.glsl" ) ) );
	shaders.emplace_back( Shader::CreateFragment( Shader::Source( "assets/fragment-texture.glsl" ) ) );

	std::vector<Uniform> uniforms;
	uniforms.emplace_back( UniformIds::Texture, "Texture" );
	uniforms.emplace_back( UniformIds::Model, "Model" );
	uniforms.emplace_back( UniformIds::View, "View" );
	uniforms.emplace_back( UniformIds::Projection, "Projection" );

	testProgram = std::make_unique<ShaderProgram>( std::move( shaders ), uniforms );
	testProgram->Use();
	testProgram->Get( UniformIds::Texture ).Set( 0 );
	testProgram->Get( UniformIds::Model ).Set( glm::mat4() );
	testProgram->Get( UniformIds::View ).Set( glm::mat4() );

	testVBuffer = std::make_unique<VertexBuffer<AtlasVertex>>();

	auto overlayStart = glm::vec4( 0.2, 0.25, 0.3, 0.8 );
	auto overlayEnd = glm::vec4( 0.2, 0.25, 0.3, 0.8 );

	glm::vec4 target( 0, 0, 256, 256 );

	std::array<AtlasVertex, 4> vertices{
		AtlasVertex{ target.x, target.y, 0, 0, textColor },
		AtlasVertex{ target.x, target.w, 0, 1, textColor },
		AtlasVertex{ target.z, target.w, 1, 1, textColor },
		AtlasVertex{ target.z, target.y, 1, 0, textColor } };

	testVBuffer->PushQuad( vertices );

	//tex = std::make_unique<Texture>( "E:/Projects/kodo/kodo-fm/Assets/refresh55.png" );
	//tex = std::make_unique<Texture>( "C:/Users/Jay/Desktop/kodo-fm-logo.png", false );
	tex = std::make_unique<Texture>( "E:/test2.png", true );
}

#ifdef _DEBUG
void OnGLFWError( int, const char* message )
{
	throw kodogl::exception( message );
}
void CALLBACK OnOpenGLError( GLenum src, GLenum type, GLuint id, GLenum severity, GLsizei len, const GLchar* message, const void* usr )
{
	throw kodogl::exception( message );
}
#endif 

typedef LRESULT( CALLBACK * Win32WndProc )(HWND, UINT, WPARAM, LPARAM);
LRESULT( CALLBACK* glfwWndProc ) (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK glfwWndProcOverride( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
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

	return CallWindowProc( glfwWndProc, hwnd, msg, wparam, lparam );
}

void KodoMain()
{
	GLFWwindow* window;

#ifdef _DEBUG
	glfwSetErrorCallback( OnGLFWError );
#endif

	if (!glfwInit())
	{
		exit( EXIT_FAILURE );
	}

#ifdef _DEBUG
	glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, _DEBUG );
#endif
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_VISIBLE, true );
	glfwWindowHint( GLFW_RESIZABLE, true );

	window = glfwCreateWindow( 1, 1, "kodo-gl", nullptr, nullptr );

	if (!window)
	{
		glfwTerminate();
		exit( EXIT_FAILURE );
	}

	glfwMakeContextCurrent( window );
	glfwSetKeyCallback( window, OnGLFWKeyboard );
	glfwSetFramebufferSizeCallback( window, OnGLFWResize );
	glfwSetWindowRefreshCallback( window, OnGLFWRender );

	gl::sys::LoadFunctions();

#ifdef _DEBUG
	gl::Enable( gl::DEBUG_OUTPUT_SYNCHRONOUS );
	gl::DebugMessageCallback( &OnOpenGLError, nullptr );
#endif

	{
		std::vector<Shader> shaders;
		shaders.emplace_back( Shader::CreateVertex( Shader::Source( "assets/vertex-font.glsl" ) ) );
		shaders.emplace_back( Shader::CreateFragment( Shader::Source( "assets/fragment-font.glsl" ) ) );

		std::vector<Uniform> uniforms;
		uniforms.emplace_back( UniformIds::Texture, "Texture" );
		uniforms.emplace_back( UniformIds::Model, "Model" );
		uniforms.emplace_back( UniformIds::View, "View" );
		uniforms.emplace_back( UniformIds::Projection, "Projection" );

		textProgram = std::make_unique<ShaderProgram>( std::move( shaders ), uniforms );
		textProgram->Use();
		textProgram->Get( UniformIds::Texture ).Set( 0 );
		textProgram->Get( UniformIds::Model ).Set( glm::mat4() );
		textProgram->Get( UniformIds::View ).Set( glm::mat4() );

		textVertexBuffer = std::make_unique<VertexBuffer<AtlasVertex>>();
	}

	{
		AtlasLoader atlasLoader{ 512, 512 };
		atlasLoader.Load( "assets/PatuaOne.ttf", 40 );
		atlasLoader.Load( "assets/PatuaOne.ttf", 14, "0123456789.,e+-" );
		atlas = atlasLoader.Finish();
	}

	CreateTestVBuffer();

	glfwSetWindowSize( window, 800, 500 );
	glfwSwapInterval( 1 );

	glfwShowWindow( window );
	glfwSetTime( 0 );

	//
	// 
	//
	glfwWndProc = reinterpret_cast<Win32WndProc>(GetWindowLongPtr( glfwGetWin32Window( window ), GWLP_WNDPROC ));
	SetWindowLongPtr( glfwGetWin32Window( window ), GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&glfwWndProcOverride) );

	auto timeLast = 0.0;
	auto timeFrames = 0;
	auto frameTimeLast = 0.0;

	auto r = 50.0f;
	auto f = 0.2f;

	TextLayout textLayout{ u8"kodo-gl", atlas->Get( 0 ), *textVertexBuffer };
	TextLayout frameTimeLayout{ u8"0", atlas->Get( 1 ), *textVertexBuffer };

	auto textSize = textLayout.Dimensions();

	while (!glfwWindowShouldClose( window ))
	{
		auto timeCurrent = glfwGetTime();
		timeFrames++;

		auto textCenter = glm::vec2( globalViewport.x / 2, globalViewport.y / 2 );
		textCenter.x += r * glm::sin( 2 * glm::pi<float_t>() * f * static_cast<float_t>(timeCurrent) );
		textCenter.y += r * glm::cos( 2 * glm::pi<float_t>() * f * static_cast<float_t>(timeCurrent) );
		auto textPosition = glm::vec2();
		textPosition.x = textCenter.x - textSize.x / 2;
		textPosition.y = textCenter.y;

		textLayout.Update( textPosition, textColor );

		if (timeCurrent - timeLast >= 1.0)
		{
			auto frameTime = 1000.0 / static_cast<double_t>(timeFrames);

			timeLast += 1.0;
			timeFrames = 0;

			if (frameTime != frameTimeLast)
			{
				frameTimeLast = frameTime;
				auto frameTimeString = std::to_string( kodogl::round( static_cast<float_t>(frameTime), 2 ) );
				frameTimeLayout = TextLayout{ frameTimeString, atlas->Get( 1 ), *textVertexBuffer };

				auto frameTimeSize = frameTimeLayout.Dimensions();

				frameTimeLayout.Update(
					glm::vec2( globalViewport.x - 10 - frameTimeSize.x, globalViewport.y - 10 - frameTimeSize.y ),
					textColor );
			}
		}

		OnGLFWRender( window );
		//glfwWaitEvents();
		glfwPollEvents();
	}

	glfwDestroyWindow( window );
	glfwTerminate();
}

#ifndef _DEBUG
int32_t WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int32_t )
#else
int32_t main()
#endif
{
	try
	{
		KodoMain();
	}
	catch (const std::exception& exp)
	{
		fprintf( stderr, "%s", exp.what() );
	}

	return 0;
}