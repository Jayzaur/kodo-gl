#pragma once

#include "kodo-gl.hpp"

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
		GLuint shaderHandle;
		GLenum shaderType;

	public:

		GLuint Handle() const
		{
			return shaderHandle;
		}

		std::string source;

		explicit Shader( std::string source, GLenum type ) :
			shaderHandle( 0 ),
			shaderType( type ),
			source( source )
		{
			Compile();
		}

		void Compile()
		{
			auto sourcePtr = source.c_str();
			auto compileStatus = 0;

			shaderHandle = gl::CreateShader( shaderType );
			gl::ShaderSource( shaderHandle, 1, &sourcePtr, nullptr );
			gl::CompileShader( shaderHandle );
			gl::GetShaderiv( shaderHandle, gl::COMPILE_STATUS, &compileStatus );

			if (compileStatus == gl::FALSE_)
			{
				auto maxLength = 0;
				gl::GetShaderiv( shaderHandle, gl::INFO_LOG_LENGTH, &maxLength );

				// The maxLength includes the NULL character.
				std::string infoLog( maxLength, '\0' );
				gl::GetShaderInfoLog( shaderHandle, maxLength, &maxLength, &infoLog[0] );

				// We don't need the shader anymore.
				gl::DeleteShader( shaderHandle );

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

		static std::string Source( const std::string& filename )
		{
			FILE* file;
			fopen_s( &file, filename.c_str(), "rb" );

			if (!file)
				throw ShaderException( "Unable to load shader source from file." );

			fseek( file, 0, SEEK_END );
			auto size = ftell( file );
			fseek( file, 0, SEEK_SET );

			std::string buffer( size, '\0' );
			fread( &buffer[0], sizeof( char ), size, file );
			fclose( file );
			return buffer;
		}
	};

	struct ShaderUniformDesc
	{
		GLint Id;
		std::string Name;

		template<typename TId>
		ShaderUniformDesc( TId id, std::string name ) :
			Id( static_cast<GLint>(id) ), Name( name )
		{
		}
	};

	struct ShaderUniform
	{
		GLint Id;
		GLint Location;
		std::string Name;

		ShaderUniform( GLint id, GLint location, std::string name ) :
			Id( id ), Location( location ), Name( name )
		{

		}
	};

	class ShaderProgram
	{
	public:

		const GLuint Id;

	private:

		std::vector<Shader> shaders;
		std::map<GLint, ShaderUniform> uniforms;

	public:

		template<typename T>
		GLint GetUniform( T id ) const
		{
			return uniforms.at( static_cast<GLint>(id) ).Location;
		}

		explicit ShaderProgram( std::vector<Shader> shaders, std::vector<ShaderUniformDesc> unis ) :
			Id( gl::CreateProgram() ),
			shaders( shaders )
		{
			Link();
			LoadUniforms( unis );
		}

		~ShaderProgram()
		{
			gl::DeleteProgram( Id );
		}

		//
		// Alias for glUseProgram.
		//
		void Use() const
		{
			gl::UseProgram( Id );
		}

		void LoadUniforms( std::vector<ShaderUniformDesc> unis )
		{
			for (auto& uniform : unis)
			{
				if (uniforms.count( uniform.Id ) != 0)
					throw ShaderException( "Can't have more than one uniform with the same Id." );

				auto location = gl::GetUniformLocation( Id, uniform.Name.c_str() );

				if (location == -1)
					throw ShaderException( "Couldn't find the uniform." );

				uniforms.emplace( uniform.Id, ShaderUniform{ uniform.Id, location, uniform.Name } );
			}
		}

		void Link()
		{
			auto linkStatus = 0;

			// Attach our shaders to our program.
			for (const auto& shader : shaders)
				gl::AttachShader( Id, shader.Handle() );

			// Link our program.
			gl::LinkProgram( Id );
			gl::GetProgramiv( Id, gl::LINK_STATUS, &linkStatus );

			if (linkStatus == gl::FALSE_)
			{
				auto maxLength = 0;
				gl::GetProgramiv( Id, gl::INFO_LOG_LENGTH, &maxLength );

				// The maxLength includes the NULL character.
				std::string infoLog( maxLength, '\0' );
				gl::GetProgramInfoLog( Id, maxLength, &maxLength, &infoLog[0] );

				// We don't need the program anymore.
				gl::DeleteProgram( Id );

				// Don't leak shaders either.
				for (const auto& shader : shaders)
					gl::DeleteShader( shader.Handle() );

				throw ShaderException( infoLog );
			}
		}
	};
}