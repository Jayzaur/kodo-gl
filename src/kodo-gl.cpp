#include "kodo-gl.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "VertexBuffer.hpp"
#include "TextureAtlas.hpp"
#include "TextureFont.hpp"
#include "Shader.hpp"

using namespace kodogl;

enum class UniformIds
{
	Texture,
	Model,
	View,
	Projection,
};

std::unique_ptr<ShaderProgram> textProgram;

struct Vert
{
	float_t x, y;
	float_t r, g, b, a;
};

class DrawingContext
{
public:

	void Clear()
	{

	}
};

std::unique_ptr<ShaderProgram> testProgram;
std::unique_ptr<VertexBuffer<Vert>> testVBuffer;

std::unique_ptr<Atlas> atlas;
std::unique_ptr<AtlasVertexBuffer> textVertexBuffer;

glm::vec2 globalViewport;
glm::mat4 globalProjection;

glm::vec4 clearColor{ 0.1, 0.15, 0.2, 1 };
glm::vec4 textColor{ 1, 1, 1, 1 };

double globalFrameTime;
bool frameTimeStringChanged;
std::string frameTimeString;

void OnGLFWError( int, const char* description )
{
	fprintf( stderr, "%s", description );
}

void OnGLFWResize( GLFWwindow*, int32_t width, int32_t height )
{
	if (width == 0 || height == 0)
		return;

	globalViewport = glm::vec2( width, height );
	globalProjection = glm::ortho( 0.0f, globalViewport.x, 0.0f, globalViewport.y, -1.0f, 1.0f );

	gl::Viewport( 0, 0, static_cast<GLsizei>(globalViewport.x), static_cast<GLsizei>(globalViewport.y) );

	textProgram->Use();
	textProgram->Get( UniformIds::Projection ).Set( globalProjection );
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
	if (frameTimeStringChanged)
	{
		frameTimeStringChanged = false;

		textVertexBuffer->Clear();

		const auto& patuaOne = atlas->Get( "PatuaOne" );
		auto textSize = patuaOne.Measure( frameTimeString );
		auto textLocation = glm::vec2( globalViewport.x / 2.0f - textSize.x / 2.0f, globalViewport.y / 2.0f - textSize.y / 2.0f );
		auto textId = 0;

		textVertexBuffer->Add( patuaOne, frameTimeString, textLocation, textColor, textId );

		textLocation.y += patuaOne.BaselineToBaseline;

		textVertexBuffer->Add( patuaOne, "kodo-gl", textLocation, textColor, textId );
	}

	gl::ClearColor( clearColor.r, clearColor.g, clearColor.b, clearColor.a );
	gl::Clear( gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT );

	gl::Enable( gl::BLEND );
	gl::BlendFunc( gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA );

	textProgram->Use();
	textVertexBuffer->Render();

	glfwSwapBuffers( window );
}

void CreateTestVBuffer( const glm::vec4& target )
{
	std::vector<Shader> shaders;
	shaders.emplace_back(
		Shader::CreateVertex(
			"uniform mat4 model;"
			"uniform mat4 view;"
			"uniform mat4 projection;"
			"attribute vec2 vertex;"
			"attribute vec4 color;"
			"void main()"
			"{"
			"gl_FrontColor = color;"
			"gl_Position = projection*(view*(model*vec4( vec3(vertex,0),1.0 )));"
			"}"
			) );
	shaders.emplace_back( Shader::CreateFragment(
		"void main()"
		"{"
		"gl_FragColor = gl_Color;"
		"}"
		) );

	std::vector<Uniform> uniforms = {
		Uniform{ UniformIds::Model, "model" },
		Uniform{ UniformIds::View, "view" },
		Uniform{ UniformIds::Projection, "projection" },
	};

	testProgram = std::make_unique<ShaderProgram>( std::move( shaders ), uniforms );

	std::vector<VertexAttribute> attributes = {
		VertexAttribute::Create2f( "vertex" ),
		VertexAttribute::Create4f( "color" )
	};
	testVBuffer = std::make_unique<VertexBuffer<Vert>>( attributes );

	auto overlayStart = glm::vec4( 0.2, 0.25, 0.3, 0.8 );
	auto overlayEnd = glm::vec4( 0.2, 0.25, 0.3, 0.8 );

	std::vector<GLuint> indices = { 0,1,2, 0,2,3 };
	std::vector<Vert> vertices = {
		{ target.x, target.w, overlayEnd.r, overlayEnd.g, overlayEnd.b, overlayEnd.a },
		{ target.x, target.y, overlayStart.r, overlayStart.g, overlayStart.b, overlayStart.a },
		{ target.z, target.y, overlayStart.r, overlayStart.g, overlayStart.b, overlayStart.a },
		{ target.z, target.w, overlayEnd.r, overlayEnd.g, overlayEnd.b, overlayEnd.a },
	};

	testVBuffer->Push( vertices, indices );
}

void KodoMain()
{
	GLFWwindow* window;

	glfwSetErrorCallback( OnGLFWError );

	if (!glfwInit())
	{
		exit( EXIT_FAILURE );
	}

	glfwWindowHint( GLFW_VISIBLE, true );
	glfwWindowHint( GLFW_RESIZABLE, true );

	window = glfwCreateWindow( 1, 1, "kodo-gl", nullptr, nullptr );

	if (!window)
	{
		glfwTerminate();
		exit( EXIT_FAILURE );
	}

	glfwMakeContextCurrent( window );
	glfwSetFramebufferSizeCallback( window, OnGLFWResize );
	glfwSetWindowRefreshCallback( window, OnGLFWRender );
	glfwSetKeyCallback( window, OnGLFWKeyboard );

	gl::sys::LoadFunctions();

	{
		std::vector<Shader> shaders;
		shaders.emplace_back( Shader::CreateVertex( Shader::Source( "assets/vertex-font.glsl" ) ) );
		shaders.emplace_back( Shader::CreateFragment( Shader::Source( "assets/fragment-font.glsl" ) ) );

		std::vector<Uniform> uniforms;
		uniforms.emplace_back( UniformIds::Texture, "texture" );
		uniforms.emplace_back( UniformIds::Model, "model" );
		uniforms.emplace_back( UniformIds::View, "view" );
		uniforms.emplace_back( UniformIds::Projection, "projection" );

		textProgram = std::make_unique<ShaderProgram>( std::move( shaders ), uniforms );
	}

	{
		textVertexBuffer = std::make_unique<AtlasVertexBuffer>();
	}

	atlas = std::make_unique<Atlas>();
	atlas->Add( "PatuaOne", "assets/PatuaOne.ttf", 40 );
	auto atlasUsed = atlas->Used();
	atlas->Upload();

	textProgram->Use();
	textProgram->Get( UniformIds::Texture ).Set( 0 );
	textProgram->Get( UniformIds::Model ).Set( glm::mat4() );
	textProgram->Get( UniformIds::View ).Set( glm::mat4() );

	glfwSetWindowSize( window, 800, 500 );
	glfwShowWindow( window );
	glfwSwapInterval( 1 );

	glfwSetTime( 0 );

	auto timeLast = 0.0;
	auto timeFrames = 0;

	frameTimeString = "16.6666";
	frameTimeStringChanged = true;

	while (!glfwWindowShouldClose( window ))
	{
		auto timeCurrent = glfwGetTime();
		timeFrames++;

		if (timeCurrent - timeLast >= 1.0)
		{
			auto frameTime = 1000.0 / static_cast<double_t>(timeFrames);
			//auto framesPerSeconds = timeFrames;

			timeLast += 1.0;
			timeFrames = 0;

			if (frameTime != globalFrameTime)
			{
				globalFrameTime = frameTime;
				frameTimeStringChanged = true;
				frameTimeString = std::to_string( frameTime );
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
int32_t WINAPI WinMain( _In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int32_t )
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