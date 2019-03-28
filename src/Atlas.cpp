#include "Atlas.hpp"
#include "AtlasFont.hpp"

namespace kodogl
{
	void Atlas::emplace_back( AtlasLoaderFont&& font )
	{
		fonts.emplace_back( *this, std::move( font ) );
	}

	std::unique_ptr<Atlas> AtlasLoader::Finish()
	{
		GLuint idOfTexture;

		gl::GenTextures( 1, &idOfTexture );

		gl::BindTexture( gl::TEXTURE_2D, idOfTexture );
		gl::TexParameteri( gl::TEXTURE_2D, gl::TEXTURE_WRAP_S, gl::CLAMP_TO_EDGE );
		gl::TexParameteri( gl::TEXTURE_2D, gl::TEXTURE_WRAP_T, gl::CLAMP_TO_EDGE );
		gl::TexParameteri( gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, gl::LINEAR );
		gl::TexParameteri( gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::LINEAR );

		gl::TexStorage2D( gl::TEXTURE_2D, 1, gl::R8, widthOfTexture, heightOfTexture );
		gl::TexSubImage2D( gl::TEXTURE_2D, 0, 0, 0, widthOfTexture, heightOfTexture, gl::RED, gl::UNSIGNED_BYTE, data.data() );

		auto atlas = std::make_unique<Atlas>( idOfTexture );

		for (auto&& loaderFont : fonts)
		{
			atlas->emplace_back( std::move( loaderFont ) );
		}

		return atlas;
	}
}
