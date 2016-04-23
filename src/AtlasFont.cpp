#include "AtlasFont.hpp"
#include "Atlas.hpp"

#include <sstream>

namespace kodogl
{
	AtlasFont::AtlasFont( AtlasFont&& that ) :
		Size( that.Size ),
		atlas( that.atlas ),
		glyphs( std::move( that.glyphs ) ),
		height( that.height ),
		linegap( that.linegap ), ascender( that.ascender ),
		descender( that.descender ), underlinePosition( that.underlinePosition ),
		underlineThickness( that.underlineThickness ), BaselineToBaseline( that.BaselineToBaseline )
	{
	}

	AtlasFont::AtlasFont( Atlas& atlas, AtlasLoaderFont&& font ) :
		Size( font.Size ),
		atlas( atlas ),
		glyphs( std::move( font.glyphs ) ),
		height( font.height ),
		linegap( font.linegap ), ascender( font.ascender ),
		descender( font.descender ), underlinePosition( font.underlinePosition ),
		underlineThickness( font.underlineThickness ),
		BaselineToBaseline( font.BaselineToBaseline )
	{
	}

	GlyphEnumerator AtlasFont::GlyphEnumerator( const std::string& str, bool fallBack ) const
	{
		return kodogl::GlyphEnumerator( *this, str, fallBack );
	}

	const AtlasGlyph& AtlasFont::GetGlyph( utf8::uint32_t codepoint, bool fallback ) const
	{
		if (!glyphs.count( codepoint ))
		{
			if (fallback)
			{
				return glyphs.at( FallbackCodepoint );
			}

			throw AtlasException( "Glyph not loaded for codepoint -> " );
		}

		return glyphs.at( codepoint );
	}

	glm::vec2 AtlasFont::Measure( const std::string& text ) const
	{
		auto glyphs = GlyphEnumerator( text );
		auto width = 0.0f;
		auto maxHeight = 0.0f;
		auto minOffY = 0.0f;

		while (glyphs)
		{
			auto glyph = glyphs.Next();

			if (minOffY == 0)
				minOffY = static_cast<float_t>(glyph.OffsetY) - static_cast<float_t>(glyph.Height);

			width += glyph.AdvanceX;

			maxHeight = glm::max( glyph.Height, static_cast<float_t>(glyph.Height) );
			minOffY = glm::min( minOffY, static_cast<float_t>(glyph.OffsetY) - static_cast<float_t>(glyph.Height) );
		}

		return glm::vec2( width, glm::abs( maxHeight ) + glm::abs( minOffY ) );
	}
}
