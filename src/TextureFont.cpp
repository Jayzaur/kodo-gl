#include "TextureFont.hpp"
#include "TextureAtlas.hpp"

#include <sstream>

namespace kodogl
{
	AtlasFont::AtlasFont( AtlasFont&& that ) :
		Size( that.Size ),
		atlas( that.atlas ),
		filename( std::move( that.filename ) ),
		glyphs( std::move( that.glyphs ) ),
		height( that.height ),
		linegap( that.linegap ), ascender( that.ascender ),
		descender( that.descender ), underlinePosition( that.underlinePosition ),
		underlineThickness( that.underlineThickness ), BaselineToBaseline( that.BaselineToBaseline )
	{
	}

	GlyphEnumerator AtlasFont::GlyphEnumerator( const std::string& str ) const
	{
		return kodogl::GlyphEnumerator( *this, str );
	}

	const AtlasGlyph& AtlasFont::GetGlyph( utf8::uint32_t codepoint ) const
	{
		if (!IsLoaded( codepoint ))
		{
			std::ostringstream errorStream;
			errorStream << "Glyph not loaded for codepoint '" << codepoint << "'.";
			throw AtlasFontException( errorStream.str() );
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

			maxHeight = glm::max( height, static_cast<float_t>(glyph.Height) );
			minOffY = glm::min( minOffY, static_cast<float_t>(glyph.OffsetY) - static_cast<float_t>(glyph.Height) );
		}

		return glm::vec2( width, glm::abs( maxHeight ) + glm::abs( minOffY ) );
	}

	glm::vec4 AtlasFont::Measure( const glm::vec2& pos, const std::string& text ) const
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

			maxHeight = glm::max( height, static_cast<float_t>(glyph.Height) );
			minOffY = glm::min( minOffY, static_cast<float_t>(glyph.OffsetY) - static_cast<float_t>(glyph.Height) );
		}

		return glm::vec4( pos.x, pos.y + minOffY, pos.x + width, pos.y + glm::abs( maxHeight ) );
	}

	bool AtlasFont::IsLoaded( utf8::uint32_t codepoint ) const
	{
		return glyphs.count( codepoint ) > 0;
	}

	AtlasFont::AtlasFont( Atlas& atlas, const std::string& filename, float_t size, const std::string& charset ) :
		Size( size ),
		atlas( atlas ),
		filename( filename )
	{
		auto library = LoadFT_Library();
		auto face = LoadFT_Face( library.get(), filename, Size );

		underlinePosition = face->underline_position / (HRES * HRES) * Size;
		underlinePosition = glm::min( -2.0f, glm::round( underlinePosition ) );
		underlineThickness = face->underline_thickness / (HRES * HRES) * Size;
		underlineThickness = glm::max( 1.0f, glm::round( underlineThickness ) );

		auto metrics = face->size->metrics;
		ascender = (metrics.ascender >> 6) / 100.0f;
		descender = (metrics.descender >> 6) / 100.0f;
		height = (metrics.height >> 6) / 100.0f;
		linegap = height - ascender + descender;

		BaselineToBaseline = (ascender - descender + linegap)*100.0f;

#pragma warning ( disable: 4838 )
		static constexpr uint8_t data[4 * 4 * 3] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
													 -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
#pragma warning ( default: 4838 )

		Region region;

		if (!atlas.GetRegion( 5, 5, region ))
			throw AtlasFontException( "Atlas is full." );

		atlas.SetRegion( region.X, region.Y, 4, 4, data, 0 );

		AtlasGlyph glyph{ size_t( -1 ), atlas.Normalize( Region{ region.X + 2, region.Y + 2, 1, 1 } ) };
		glyphs.emplace( glyph.Codepoint, glyph );

		GenerateGlyphs( face.get(), charset );
	}

	void AtlasFont::GenerateGlyphs( FT_Face face, const std::string& charset )
	{
		CodepointEnumerator codepoints{ charset };

		while (codepoints)
		{
			auto codepoint = codepoints.Next();

			// Maybe the glyph is already loaded?
			if (IsLoaded( codepoint ))
				continue;

			auto glyphIndex = FT_Get_Char_Index( face, static_cast<FT_ULong>(codepoint) );

			if (FT_Load_Glyph( face, glyphIndex, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT ))
				throw AtlasFontException( "FT_Load_Glyph failed." );

			auto glyphSlot = face->glyph;
			auto ft_bitmap = glyphSlot->bitmap;
			auto ft_glyph_top = glyphSlot->bitmap_top;
			auto ft_glyph_left = glyphSlot->bitmap_left;
			auto glyphWidth = ft_bitmap.width;
			auto glyphHeight = ft_bitmap.rows;

			Region atlasRegion;

			// We want each glyph to be separated by at least one black pixel.
			if (!atlas.GetRegion( glyphWidth + 1, glyphHeight + 1, atlasRegion ))
				throw AtlasFontException( "Atlas is full." );

			Region glyphRegion{ atlasRegion.X, atlasRegion.Y, glyphWidth, glyphHeight };

			atlas.SetRegion( glyphRegion, ft_bitmap.buffer, ft_bitmap.pitch );

			// Discard hinting to get advance.
			FT_Load_Glyph( face, glyphIndex, FT_LOAD_RENDER | FT_LOAD_NO_HINTING );

			AtlasGlyph glyph{
				codepoint,
				static_cast<float_t>(glyphWidth),
				static_cast<float_t>(glyphHeight),
				static_cast<float_t>(ft_glyph_left),
				static_cast<float_t>(ft_glyph_top),
				static_cast<float_t>(face->glyph->advance.x) / HRES,
				static_cast<float_t>(face->glyph->advance.y) / HRES,
				atlas.Normalize( glyphRegion )
			};

			glyphs.emplace( codepoint, glyph );
		}

		GenerateKerning( face );
	}

	void AtlasFont::GenerateKerning( FT_Face face )
	{
		//
		// For each glyph couple combination, check if kerning is necessary.
		//
		for (auto& kp : glyphs)
		{
			auto& glyph = kp.second;
			auto glyphIndex = FT_Get_Char_Index( face, glyph.Codepoint );

			// Skip the null glyph.
			if (glyph.Codepoint == size_t( -1 ))
				continue;

			glyph.Kerning.clear();

			for (const auto& kpj : glyphs)
			{
				auto prevGlyph = kpj.second;
				auto prevIndex = FT_Get_Char_Index( face, prevGlyph.Codepoint );

				FT_Vector kerning;
				FT_Get_Kerning( face, prevIndex, glyphIndex, FT_KERNING_UNFITTED, &kerning );

				if (kerning.x)
				{
					glyph.Kerning.push_back( TextureFontKerning{ prevGlyph.Codepoint, kerning.x / static_cast<float_t>(HRES * HRES) } );
				}
			}
		}
	}

	void AtlasVertexBuffer::Remove( GLint id )
	{
		stored.erase( id );
	}

	void AtlasVertexBuffer::Modify( GLint id, const AtlasFont& font, const std::string& text, const glm::vec2& position, const glm::vec4& color )
	{
		const auto& item = stored.at( id );

		auto x = position.x;
		auto y = position.y;
		auto glyphs = font.GlyphEnumerator( text );

		while (glyphs)
		{
			auto glyph = glyphs.Next();
			auto x0 = x + glyph.OffsetX;
			auto y0 = y + glyph.OffsetY;
			auto x1 = x0 + glyph.Width;
			auto y1 = y0 - glyph.Height;

			auto& vert = vertices[item.StartOfVertices + 0];
			vert.x = x0;
			vert.y = y0;
			vert.r = color.r;
			vert.g = color.g;
			vert.b = color.b;
			vert.a = color.a;

			vert = vertices[item.StartOfVertices + 1];
			vert.x = x0;
			vert.y = y1;
			vert.r = color.r;
			vert.g = color.g;
			vert.b = color.b;
			vert.a = color.a;

			vert = vertices[item.StartOfVertices + 2];
			vert.x = x1;
			vert.y = y1;
			vert.r = color.r;
			vert.g = color.g;
			vert.b = color.b;
			vert.a = color.a;

			vert = vertices[item.StartOfVertices + 3];
			vert.x = x1;
			vert.y = y0;
			vert.r = color.r;
			vert.g = color.g;
			vert.b = color.b;
			vert.a = color.a;

			x += glyph.AdvanceX;
			y += glyph.AdvanceY;
		}

		state = VertexBufferState::Dirty;
	}

	void AtlasVertexBuffer::Add( const AtlasFont& font, const std::string& text, const glm::vec2& position, const glm::vec4& color, GLint& id )
	{
		idCounter++;

		auto x = position.x;
		auto y = position.y;
		auto glyphs = font.GlyphEnumerator( text );

		while (glyphs)
		{
			auto glyph = glyphs.Next();
			auto x0 = x + glyph.OffsetX;
			auto y0 = y + glyph.OffsetY;
			auto x1 = x0 + glyph.Width;
			auto y1 = y0 - glyph.Height;
			auto s0 = glyph.Region.L;
			auto t0 = glyph.Region.T;
			auto s1 = glyph.Region.R;
			auto t1 = glyph.Region.B;

			auto startOfVertices = vertices.size();
			auto startOfIndices = indices.size();

			vertices.emplace_back( x0, y0, s0, t0, color );
			vertices.emplace_back( x0, y1, s0, t1, color );
			vertices.emplace_back( x1, y1, s1, t1, color );
			vertices.emplace_back( x1, y0, s1, t0, color );

			for (const auto& i : IndicesOfVertex)
				indices.push_back( i );

			// Update indices.
			for (size_t i = 0; i < 6; ++i)
				indices[startOfIndices + i] += startOfVertices;

			// Emplace item.
			stored.emplace( idCounter, VertexBufferItem{ startOfIndices, startOfVertices } );

			x += glyph.AdvanceX;
			y += glyph.AdvanceY;
		}

		state = VertexBufferState::Dirty;

		id = idCounter;
	}
}
