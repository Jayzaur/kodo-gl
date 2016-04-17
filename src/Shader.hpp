#pragma once

#include "kodo-gl.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace kodogl
{
	class ShaderException : public std::exception
	{
		std::string message;
	public:
		explicit ShaderException( std::string message ) : std::exception( message.c_str() ), message( message ) {}
	};

	class Shader
	{
		std::string source;
		GLuint shaderHandle;
		GLenum shaderType;

	public:

		GLuint Handle() const
		{
			return shaderHandle;
		}

		Shader( const Shader& other ) = delete;

		Shader( Shader&& other ) :
			source( std::move( other.source ) ),
			shaderHandle( other.shaderHandle ),
			shaderType( other.shaderType )
		{
			other.shaderHandle = 0;
			other.shaderType = 0;
		}

		explicit Shader( std::string source, GLenum type ) :
			source( source ),
			shaderHandle( gl::CreateShader( type ) ),
			shaderType( type )
		{
			Compile();
		}

		~Shader()
		{
			if (shaderHandle != 0)
			{
				gl::DeleteShader( shaderHandle );
				shaderHandle = 0;
			}
		}

		void Compile() const
		{
			auto sourcePtr = source.c_str();
			auto compileStatus = 0;

			gl::ShaderSource( shaderHandle, 1, &sourcePtr, nullptr );
			gl::CompileShader( shaderHandle );
			gl::GetShaderiv( shaderHandle, gl::COMPILE_STATUS, &compileStatus );

			if (compileStatus == gl::FALSE_)
			{
				auto infoLength = 0;
				gl::GetShaderiv( shaderHandle, gl::INFO_LOG_LENGTH, &infoLength );

				std::string infoLog( infoLength, '\0' );
				gl::GetShaderInfoLog( shaderHandle, infoLength, &infoLength, &infoLog[0] );

				throw ShaderException( infoLog );
			}
		}

		static Shader CreateVertex( std::string source )
		{
			return Shader{ source, gl::VERTEX_SHADER };
		}

		static Shader CreateFragment( std::string source )
		{
			return Shader{ source, gl::FRAGMENT_SHADER };
		}

		static std::string Source( const std::string& filename );
	};

	struct Uniform
	{
		//
		// Custom integral identifier of the uniform.
		//
		GLint Id;
		//
		// Name of the uniform.
		//
		std::string Name;
		//
		// Location of the uniform.
		//
		const GLint Location;

		//
		// Create a new uniform with the specified integral identifier and name.
		//
		template<typename TId>
		Uniform( TId id, std::string name, GLint location = -1 ) :
			Id( static_cast<GLint>(id) ), Name( name ), Location( location )
		{
		}

		//
		// glUniform1i
		//
		void Set( GLint v ) const
		{
			gl::Uniform1i( Location, v );
		}

		//
		// glUniformMatrix4fv
		//
		void Set( const glm::mat4& mat ) const
		{
			gl::UniformMatrix4fv( Location, 1, 0, glm::value_ptr( mat ) );
		}
	};

	class ShaderProgram
	{
		GLuint Id;

		std::vector<Shader> shaders;
		std::map<GLint, Uniform> uniforms;

	public:

		template<typename T>
		GLint operator [] ( T id )const { return at( id ); }

		template<typename T>
		GLint at( T id ) const
		{
			return uniforms.at( static_cast<GLint>(id) ).Location;
		}

		//
		// Get the Uniform with the specified identifier.
		//
		template<typename T>
		const Uniform& Get( T id ) const
		{
			return uniforms.at( static_cast<GLint>(id) );
		}

		explicit ShaderProgram( std::vector<Shader>&& shaders, const std::vector<Uniform>& unis ) :
			Id( gl::CreateProgram() ),
			shaders( std::move( shaders ) )
		{
			Link();
			LoadUniforms( unis );
		}

		~ShaderProgram()
		{
			if (Id != 0)
			{
				gl::DeleteProgram( Id );
				Id = 0;
			}
		}

		//
		// Alias for glUseProgram.
		//
		void Use() const
		{
			gl::UseProgram( Id );
		}

		void LoadUniforms( const std::vector<Uniform>& unis );
		void Link();
	};
}