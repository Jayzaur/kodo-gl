#pragma once

#include "kodo-gl.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace kodogl
{
	class ShaderException : public exception
	{
	public:
		explicit ShaderException( std::string message ) : exception( message ) {}
	};

	class Shader
	{
		std::string source;
		GLuint nameOfShader;
		GLenum typeOfShader;

	public:

		GLuint Name() const
		{
			return nameOfShader;
		}

		Shader( const Shader& other ) = delete;

		Shader( Shader&& other ) :
			source( std::move( other.source ) ),
			nameOfShader( other.nameOfShader ),
			typeOfShader( other.typeOfShader )
		{
			other.nameOfShader = 0;
			other.typeOfShader = 0;
		}

		explicit Shader( std::string source, GLenum type ) :
			source( source ),
			nameOfShader( gl::CreateShader( type ) ),
			typeOfShader( type )
		{
			Compile();
		}

		~Shader()
		{
			if (nameOfShader != 0)
			{
				gl::DeleteShader( nameOfShader );
				nameOfShader = 0;
			}
		}

		void Compile() const
		{
			auto sourcePtr = source.c_str();
			auto compileStatus = 0;

			gl::ShaderSource( nameOfShader, 1, &sourcePtr, nullptr );
			gl::CompileShader( nameOfShader );
			gl::GetShaderiv( nameOfShader, gl::COMPILE_STATUS, &compileStatus );

			if (compileStatus == gl::FALSE_)
			{
				auto infoLength = 0;
				gl::GetShaderiv( nameOfShader, gl::INFO_LOG_LENGTH, &infoLength );

				std::string infoLog( infoLength, '\0' );
				gl::GetShaderInfoLog( nameOfShader, infoLength, &infoLength, &infoLog[0] );

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

		/// <summary>
		/// glUniform1i
		/// </summary>
		/// <param name="v">The integer.</param>
		void Set( GLint v ) const
		{
			gl::Uniform1i( Location, v );
		}

		/// <summary>
		/// glUniformMatrix4fv
		/// </summary>
		/// <param name="mat">The matrix.</param>
		void Set( const glm::mat4& mat ) const
		{
			gl::UniformMatrix4fv( Location, 1, 0, glm::value_ptr( mat ) );
		}
	};

	class ShaderProgram
	{
		GLuint nameOfProgram;

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
			nameOfProgram( gl::CreateProgram() ),
			shaders( std::move( shaders ) )
		{
			Link();
			LoadUniforms( unis );
		}

		~ShaderProgram()
		{
			if (nameOfProgram != 0)
			{
				gl::DeleteProgram( nameOfProgram );
				nameOfProgram = 0;
			}
		}

		//
		// Alias for glUseProgram.
		//
		void Use() const
		{
			gl::UseProgram( nameOfProgram );
		}

		void LoadUniforms( const std::vector<Uniform>& unis );
		void Link();
	};
}