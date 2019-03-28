#include "Shader.hpp"

#include <fstream>
#include <sstream>

namespace kodogl
{
	std::string Shader::SourceFromFile( const std::string& filename )
	{
		std::ifstream stream{ filename, std::ifstream::in | std::ifstream::binary };

		if (!stream)
		{
			std::ostringstream errorStream;
			errorStream << "Unable to load shader source from '" << filename.c_str() << "'.";
			throw ShaderException( errorStream.str() );
		}

		stream.seekg( 0, std::ifstream::end );
		auto length = static_cast<size_t>(stream.tellg());
		stream.seekg( 0, std::ifstream::beg );

		std::string buffer( length, '\0' );
		stream.read( &buffer[0], length );
		return buffer;
	}

	void ShaderProgram::LoadUniforms( const std::vector<Uniform>& unis )
	{
		for (const auto& uniform : unis)
		{
			if (uniforms.count( uniform.Id ) != 0)
				throw ShaderException( "Can't have more than one uniform with the same identifier." );

			auto location = gl::GetUniformLocation( nameOfProgram, uniform.Name.c_str() );

			if (location == -1)
			{
				std::ostringstream errorStream;
				errorStream << "Couldn't find the uniform '" << uniform.Name.c_str() << "'.";
				throw ShaderException( errorStream.str() );
			}

			uniforms.emplace( uniform.Id, Uniform{ uniform.Id, uniform.Name, location } );
		}
	}

	void ShaderProgram::Link( const std::vector<Shader>& shaders )
	{
		auto linkStatus = 0;

		// Attach our shaders to our program.
		for (const auto& shader : shaders)
			gl::AttachShader( nameOfProgram, shader.Name() );

		// Link our program.
		gl::LinkProgram( nameOfProgram );
		gl::GetProgramiv( nameOfProgram, gl::LINK_STATUS, &linkStatus );

		if (linkStatus == gl::FALSE_)
		{
			auto maxLength = 0;
			gl::GetProgramiv( nameOfProgram, gl::INFO_LOG_LENGTH, &maxLength );

			std::string infoLog( maxLength, '\0' );
			gl::GetProgramInfoLog( nameOfProgram, maxLength, &maxLength, &infoLog[0] );

			std::ostringstream errorStream;
			errorStream << "Error while linking " << programStr.c_str() << ": " << infoLog.c_str();

			throw ShaderException( errorStream.str() );
		}
	}
}
