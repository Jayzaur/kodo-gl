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

	enum class ShaderType : GLenum
	{
		Vertex = gl::VERTEX_SHADER,
		Fragment = gl::FRAGMENT_SHADER
	};

	class Shader : public nocopy
	{
		GLuint nameOfShader;
		ShaderType typeOfShader;

	public:

		GLuint Name() const
		{
			return nameOfShader;
		}

		Shader( Shader&& other ) :
			nameOfShader( other.nameOfShader ),
			typeOfShader( other.typeOfShader )
		{
			other.nameOfShader = 0;
		}

		Shader( ShaderType type, std::string source ) :
			nameOfShader( gl::CreateShader( (GLenum)type ) ),
			typeOfShader( type )
		{
			Compile( source );
		}

		~Shader()
		{
			if (nameOfShader != 0)
			{
				gl::DeleteShader( nameOfShader );
				nameOfShader = 0;
			}
		}

		void Compile( std::string source ) const
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
			return Shader{ ShaderType::Vertex, source };
		}

		static Shader CreateFragment( std::string source )
		{
			return Shader{ ShaderType::Fragment, source };
		}

		static std::string SourceFromFile( const std::string& filename );
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
			Id( static_cast<GLint>(id) ),
			Name( name ),
			Location( location )
		{
		}

		const Uniform& operator = ( GLint v ) const { return Set( v ); }
		const Uniform& Set( GLint v ) const
		{
			gl::Uniform1i( Location, v );
			return *this;
		}

		const Uniform& operator = ( GLfloat v ) const { return Set( v ); }
		const Uniform& Set( GLfloat v ) const
		{
			gl::Uniform1f( Location, v );
			return *this;
		}

		const Uniform& operator = ( const glm::vec4& v ) const { return Set( v ); }
		const Uniform& Set( const glm::vec4& v ) const
		{
			gl::Uniform4f( Location, v.x, v.y, v.z, v.w );
			return *this;
		}

		const Uniform& operator = ( const glm::mat4& v ) const { return Set( v ); }
		const Uniform& Set( const glm::mat4& v ) const
		{
			gl::UniformMatrix4fv( Location, 1, 0, glm::value_ptr( v ) );
			return *this;
		}
	};

	class ShaderProgram : public nocopy
	{
		GLuint nameOfProgram;

		std::string programStr;
		std::unordered_map<GLint, Uniform> uniforms;

	public:

		//
		// Get the Uniform with the specified identifier.
		//
		template<typename T>
		const Uniform& operator []( T id ) const
		{
			static_assert(std::is_integral<T>::value || std::is_enum<T>::value, "T must be convertible to an integral.");
			return uniforms.at( static_cast<GLint>(id) );
		}

		//
		// Get the Uniform with the specified identifier.
		//
		template<typename T>
		const Uniform& Get( T id ) const
		{
			static_assert(std::is_integral<T>::value || std::is_enum<T>::value, "T must be convertible to an integral.");
			return uniforms.at( static_cast<GLint>(id) );
		}

		ShaderProgram( std::string programStr, const std::vector<Shader>& shaders, const std::vector<Uniform>& unis ) :
			nameOfProgram( gl::CreateProgram() ),
			programStr( programStr )
		{
			Link( shaders );
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

	private:

		void LoadUniforms( const std::vector<Uniform>& unis );
		void Link( const std::vector<Shader>& shaders );
	};
}