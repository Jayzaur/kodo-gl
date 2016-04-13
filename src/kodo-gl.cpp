#include "kodo-gl.hpp"

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "VertexBuffer.hpp"
#include "TextureAtlas.hpp"
#include "TextureFont.hpp"
#include "Shader.hpp"

using namespace kodogl;

enum class TextShaderUniforms
{
	Texture,
	Model,
	View,
	Projection,
};

std::unique_ptr<VertexBuffer<AtlasVertex>> textVBuffer;
std::unique_ptr<ShaderProgram> textProgram;

struct Vert
{
	float_t x, y, z;
	float_t r, g, b, a;
};

std::unique_ptr<ShaderProgram> testProgram;
std::unique_ptr<VertexBuffer<Vert>> testVBuffer;

std::unique_ptr<Atlas> atlas;

glm::mat4 textModelMat;
glm::mat4 textViewMat;
glm::mat4 textProjectionMat;

glm::vec4 clearColor{ 0, 0, 0, 1 };
glm::vec4 textColor{ 1, 1, 1, 1 };

void OnGLFWError( int error, const char* description )
{
}

void OnGLFWResize( GLFWwindow* window, int width, int height )
{
	if (width == 0 || height == 0)
		return;

	gl::Viewport( 0, 0, width, height );
	textProjectionMat = glm::ortho( 0.0f, static_cast<float_t>(width), 0.0f, static_cast<float_t>(height), -1.0f, 1.0f );
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
	gl::ClearColor( clearColor.r, clearColor.g, clearColor.b, clearColor.a );
	gl::Clear( gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT );

	gl::Enable( gl::BLEND );
	gl::BlendFunc( gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA );

	textProgram->Use();
	{
		gl::Uniform1i( textProgram->GetUniform( TextShaderUniforms::Texture ), 0 );
		gl::UniformMatrix4fv( textProgram->GetUniform( TextShaderUniforms::Model ), 1, 0, glm::value_ptr( textModelMat ) );
		gl::UniformMatrix4fv( textProgram->GetUniform( TextShaderUniforms::View ), 1, 0, glm::value_ptr( textViewMat ) );
		gl::UniformMatrix4fv( textProgram->GetUniform( TextShaderUniforms::Projection ), 1, 0, glm::value_ptr( textProjectionMat ) );

		textVBuffer->Render();
	}

	testProgram->Use();
	{
		gl::UniformMatrix4fv( testProgram->GetUniform( TextShaderUniforms::Model ), 1, 0, glm::value_ptr( textModelMat ) );
		gl::UniformMatrix4fv( testProgram->GetUniform( TextShaderUniforms::View ), 1, 0, glm::value_ptr( textViewMat ) );
		gl::UniformMatrix4fv( testProgram->GetUniform( TextShaderUniforms::Projection ), 1, 0, glm::value_ptr( textProjectionMat ) );

		testVBuffer->Render();
	}

	glfwSwapBuffers( window );
}

void CreateTestVBuffer()
{
	std::vector<Shader> shaders = {
		Shader::CreateVertex(
			"uniform mat4 model;"
			"uniform mat4 view;"
			"uniform mat4 projection;"
			"attribute vec3 vertex;"
			"attribute vec4 color;"
			"void main()"
			"{"
			"gl_FrontColor = color;"
			"gl_Position = projection*(view*(model*vec4( vertex,1.0 )));"
			"}"
			),
		Shader::CreateFragment(
			"void main()"
			"{"
			"gl_FragColor = vec4( gl_Color.rgb, gl_Color.a );"
			"}"
			)
	};

	std::vector<ShaderUniformDesc> uniforms = {
		ShaderUniformDesc{ TextShaderUniforms::Model, "model" },
		ShaderUniformDesc{ TextShaderUniforms::View, "view" },
		ShaderUniformDesc{ TextShaderUniforms::Projection, "projection" },
	};

	testProgram = std::make_unique<ShaderProgram>( shaders, uniforms );

	std::vector<VertexAttribute> attributes = {
		VertexAttribute::Create3f( "vertex" ),
		VertexAttribute::Create4f( "color" )
	};
	testVBuffer = std::make_unique<VertexBuffer<Vert>>( attributes );

	float_t x0 = 5;
	float_t x1 = 600;
	float_t y0 = 300;
	float_t y1 = 600;

	std::vector<GLuint> indices = { 0,1,2, 0,2,3 };
	std::vector<Vert> vertices = {
		{ x0, y0, 0, 0,0,1, 0.5f },
		{ x0, y1, 0, textColor.r, textColor.g, textColor.b, 0.5f },
		{ x1, y1, 0, textColor.r, textColor.g, textColor.b, 0.5f },
		{ x1, y0, 0, 0,0,1, 0.5f }
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

	gl::sys::LoadFunctions();

	CreateTestVBuffer();

	std::vector<Shader> textShaders = {
		Shader::CreateVertex(
			"uniform mat4 model;"
			"uniform mat4 view;"
			"uniform mat4 projection;"
			"attribute vec3 vertex;"
			"attribute vec2 tex_coord;"
			"attribute vec4 color;"
			"void main()"
			"{"
				"gl_TexCoord[0].xy = tex_coord.xy;"
				"gl_FrontColor = color;"
				"gl_Position = projection*(view*(model*vec4( vertex,1.0 )));"
			"}"
		),
		Shader::CreateFragment(
			"uniform sampler2D texture;"
			"void main()"
			"{"
				"float a = texture2D( texture, gl_TexCoord[0].xy ).r;"
				"gl_FragColor = vec4( gl_Color.rgb, gl_Color.a*a );"
			"}"
		)
	};

	std::vector<ShaderUniformDesc> textProgramUniforms = {
		ShaderUniformDesc{ TextShaderUniforms::Texture, "texture" },
		ShaderUniformDesc{ TextShaderUniforms::Model, "model" },
		ShaderUniformDesc{ TextShaderUniforms::View, "view" },
		ShaderUniformDesc{ TextShaderUniforms::Projection, "projection" },
	};

	textProgram = std::make_unique<ShaderProgram>( textShaders, textProgramUniforms );

	std::vector<VertexAttribute> textVBufferAttributes = {
		VertexAttribute::Create3f( "vertex" ),
		VertexAttribute::Create2f( "tex_coord" ),
		VertexAttribute::Create4f( "color" )
	};
	textVBuffer = std::make_unique<VertexBuffer<AtlasVertex>>( textVBufferAttributes );

	atlas = std::make_unique<Atlas>();

	const auto& patuaOne = atlas->AddFont( "PatuaOne", "assets/PatuaOne.ttf", 40 );

	std::string testText = u8"kodo-gl";
	float_t textX = 5;
	float_t textY = 400;

	auto glyphs = patuaOne.GlyphEnumerator( testText );

	while (glyphs)
	{
		auto glyph = glyphs.Next();

		auto x0 = textX + glyph.OffsetX;
		auto y0 = textY + glyph.OffsetY;
		auto x1 = x0 + glyph.Width;
		auto y1 = y0 - glyph.Height;
		auto s0 = glyph.Region.L;
		auto t0 = glyph.Region.T;
		auto s1 = glyph.Region.R;
		auto t1 = glyph.Region.B;

		std::vector<GLuint> indices = { 0,1,2, 0,2,3 };
		std::vector<AtlasVertex> vertices = {
			{ glm::vec3( x0, y0, 0 ), glm::vec2( s0, t0 ), textColor },
			{ glm::vec3( x0, y1, 0 ), glm::vec2( s0, t1 ), textColor },
			{ glm::vec3( x1, y1, 0 ), glm::vec2( s1, t1 ), textColor },
			{ glm::vec3( x1, y0, 0 ), glm::vec2( s1, t0 ), textColor }
		};

		textVBuffer->Push( vertices, indices );

		textX += glyph.AdvanceX;
	}

	atlas->Upload();
	atlas->Bind();

	glfwSetFramebufferSizeCallback( window, OnGLFWResize );
	glfwSetWindowRefreshCallback( window, OnGLFWRender );
	glfwSetKeyCallback( window, OnGLFWKeyboard );

	glfwSetWindowSize( window, 800, 500 );
	glfwShowWindow( window );
	//glfwSwapInterval( 1 );

	while (!glfwWindowShouldClose( window ))
	{
		OnGLFWRender( window );
		glfwWaitEvents();
		//glfwPollEvents();
	}

	glfwDestroyWindow( window );
	glfwTerminate();
}

int main()
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