#include "TextureFont.hpp"
#include "TextureAtlas.hpp"

namespace kodogl
{
	AtlasFont::AtlasFont( AtlasFont&& that ) :
		size( that.size ),
		atlas( that.atlas ),
		filename( std::move( that.filename ) ),
		glyphs( std::move( that.glyphs ) ),
		height( 0 ), linegap( 0 ), ascender( 0 ), descender( 0 ), underline_position( 0 ), underline_thickness( 0 )
	{
	}

	GlyphEnumerator AtlasFont::GlyphEnumerator( const std::string& str ) const
	{
		return kodogl::GlyphEnumerator( *this, str );
	}

	AtlasFont::AtlasFont( Atlas& atlas, const std::string& filename, float_t size, const std::string& charset ) :
		size( size ),
		atlas( atlas ),
		filename( filename )
	{
		auto library = LoadFT_Library();
		auto face = LoadFT_Face( library.get(), filename );

		underline_position = face.get()->underline_position / static_cast<float_t>(HRES*HRES) * size;
		underline_position = round( underline_position );
		if (underline_position > -2)
		{
			underline_position = -2.0;
		}

		underline_thickness = face.get()->underline_thickness / static_cast<float_t>(HRES*HRES) * size;
		underline_thickness = round( underline_thickness );
		if (underline_thickness < 1)
		{
			underline_thickness = 1.0;
		}

		auto metrics = face.get()->size->metrics;
		ascender = (metrics.ascender >> 6) / 100.0f;
		descender = (metrics.descender >> 6) / 100.0f;
		height = (metrics.height >> 6) / 100.0f;
		linegap = height - ascender + descender;

#pragma warning ( disable: 4838 )
		static const uint8_t data[4 * 4 * 3] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
#pragma warning ( default: 4838 )

		Region region;

		if (!atlas.GetRegion( 5, 5, region ))
			throw AtlasFontException( "Atlas is full." );

		atlas.SetRegion( region.X, region.Y, 4, 4, data, 0 );

		AtlasGlyph glyph{ size_t( -1 ), atlas.Normalize( Region{ region.X + 2, region.Y + 2, 1, 1 } ) };
		glyphs.emplace( glyph.Codepoint, glyph );

		LoadGlyphs( face.get(), charset );
	}

	void AtlasFont::LoadGlyphs( FT_Face face, const std::string& charset )
	{
		// Load each glyph.

		CodepointEnumerator codepoints{ charset };

		while (codepoints)
		{
			auto codepoint = codepoints.Next();

			// Maybe the glyph is already loaded?
			if (IsLoaded( codepoint ))
				continue;

			auto glyphIndex = FT_Get_Char_Index( face, static_cast<FT_ULong>(codepoint) );

			if (FT_Load_Glyph( face, glyphIndex, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT ))
				throw AtlasFontException( "Failed to load glyph." );

			auto glyphSlot = face->glyph;
			auto ft_bitmap = glyphSlot->bitmap;
			auto ft_glyph_top = glyphSlot->bitmap_top;
			auto ft_glyph_left = glyphSlot->bitmap_left;
			auto glyphWidth = ft_bitmap.width / 1;
			auto glyphHeight = ft_bitmap.rows;

			Region atlasRegion;

			// We want each glyph to be separated by at least one black pixel.
			if (!atlas.GetRegion( glyphWidth + 1, glyphHeight + 1, atlasRegion ))
				throw AtlasFontException( "Atlas is full." );

			Region glyphRegion{ atlasRegion.X, atlasRegion.Y, glyphWidth, glyphHeight };

			atlas.SetRegion( glyphRegion, ft_bitmap.buffer, ft_bitmap.pitch );

			// Discard hinting to get advance.
			FT_Load_Glyph( face, glyphIndex, FT_LOAD_RENDER | FT_LOAD_NO_HINTING );

			AtlasGlyph glyph;
			glyph.Codepoint = codepoint;
			glyph.Width = glyphWidth;
			glyph.Height = glyphHeight;
			glyph.OffsetX = ft_glyph_left;
			glyph.OffsetY = ft_glyph_top;
			glyph.AdvanceX = static_cast<float_t>(face->glyph->advance.x) / static_cast<float_t>(HRES);
			glyph.AdvanceY = static_cast<float_t>(face->glyph->advance.y) / static_cast<float_t>(HRES);
			glyph.Region = atlas.Normalize( glyphRegion );

			glyphs.emplace( codepoint, glyph );
		}

		GenerateKerning( face );
	}
}
