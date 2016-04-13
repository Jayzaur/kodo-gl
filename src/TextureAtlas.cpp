#include "TextureAtlas.hpp"
#include "TextureFont.hpp"

namespace kodogl
{
	const AtlasFont& Atlas::AddFont( const std::string name, const std::string& filename, float_t size, const std::string& charset )
	{
		auto emplaced = fonts.emplace( name, AtlasFont{ *this, filename, size, charset } );
		return (*emplaced.first).second;
	}
}
