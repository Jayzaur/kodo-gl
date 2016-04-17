#include "TextureAtlas.hpp"
#include "TextureFont.hpp"

namespace kodogl
{
	const AtlasFont& Atlas::Add( const std::string name, const std::string& filename, float_t size, const std::string& charset )
	{
		auto font = AtlasFont{ *this, filename, size, charset };
		auto emplaced = fonts.emplace( std::move( name ), std::move( font ) );
		return (*emplaced.first).second;
	}
}
