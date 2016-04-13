#pragma once

#include "kodo-gl.hpp"

#include <sstream>
#include <utf8\utf8.h>
#include <glm\glm.hpp>

#include "Texture.h"

namespace kodogl
{
	class Atlas;
	class GlyphEnumerator;

	struct AtlasVertex
	{
		float_t x, y, z;    // position
		float_t s, t;       // texture
		float_t r, g, b, a; // color

		AtlasVertex( const glm::vec3& position, const glm::vec2& texture, const glm::vec4& color ) :
			x( position.x ), y( position.y ), z( position.z ),
			s( texture.s ), t( texture.t ),
			r( color.r ), g( color.g ), b( color.b ), a( color.a )
		{
		}
	};

	class AtlasFontException : public std::exception
	{
		std::string message;
	public:
		explicit AtlasFontException( std::string message ) : std::exception( message.c_str() ), message( message ) {}
	};

	// 
	// A structure that describes a kerning value relatively to a Unicode codepoint.
	// 
	struct TextureFontKerning
	{
		// Unicode codepoint in UTF-32 LE encoding.
		uint32_t Codepoint;

		// Kerning value (in fractional pixels).
		float_t Value;
	};

	//
	// A structure that describes a glyph.
	//
	struct AtlasGlyph
	{
		// Unicode codepoint in UTF-32 LE encoding.
		uint32_t Codepoint;

		// Glyph's width in pixels.
		size_t Width;
		// Glyph's height in pixels.
		size_t Height;

		// Glyph's left bearing expressed in integer pixels.
		int32_t OffsetX;
		// Glyphs's top bearing expressed in integer pixels.
		int32_t OffsetY;

		// For horizontal text layouts, this is the horizontal distance (in fractional pixels) 
		// used to increment the pen position when the glyph is drawn as part of a string of text.
		float_t AdvanceX;
		// For vertical text layouts, this is the vertical distance (in fractional pixels) 
		// used to increment the pen position when the glyph is drawn as part of a string of text.
		float_t AdvanceY;

		// Normalized region occupied by this glyph within a texture atlas.
		NormalizedRegion Region;

		// A vector of kerning pairs relative to this glyph.
		std::vector<TextureFontKerning> Kerning;

		AtlasGlyph() : Codepoint( 0 ), Width( 0 ), Height( 0 ), OffsetX( 0 ), OffsetY( 0 ), AdvanceX( 0 ), AdvanceY( 0 ) {}
		AtlasGlyph( uint32_t codepoint, NormalizedRegion region ) :
			Codepoint( codepoint ),
			Width( 0 ), Height( 0 ),
			OffsetX( 0 ), OffsetY( 0 ),
			AdvanceX( 0 ), AdvanceY( 0 ),
			Region( region )
		{
		}
	};

	class CodepointEnumerator
	{
		std::string::const_iterator cbegin;
		std::string::const_iterator cend;
		std::string::const_iterator citer;
	public:
		explicit CodepointEnumerator( const std::string& str ) : cbegin( str.cbegin() ), cend( str.cend() ), citer( str.cbegin() ) {}
		utf8::uint32_t Next() { return utf8::next( citer, cend ); }
		operator bool() const { return citer != cend; }
	};

	//
	// A class that represents a font within a texture atlas.
	//
	class AtlasFont
	{
		static const int32_t HRES = 64;
		static const int32_t DPI = 96;

		const float_t size;

		Atlas& atlas;

		const std::string filename;
		std::map<uint32_t, AtlasGlyph> glyphs;

		typedef decltype(&FT_Done_Face) FT_Face_Deleter;
		typedef decltype(&FT_Done_FreeType) FT_Library_Deleter;

		/**
		* This field is simply used to compute a default line spacing (i.e., the
		* baseline-to-baseline distance) when writing text with this font. Note
		* that it usually is larger than the sum of the ascender and descender
		* taken as absolute values. There is also no guarantee that no glyphs
		* extend above or below subsequent baselines when using this distance.
		*/
		float_t height;
		/**
		* This field is the distance that must be placed between two lines of
		* text. The baseline-to-baseline distance should be computed as:
		* ascender - descender + linegap
		*/
		float_t linegap;
		/**
		* The ascender is the vertical distance from the horizontal baseline to
		* the highest 'character' coordinate in a font face. Unfortunately, font
		* formats define the ascender differently. For some, it represents the
		* ascent of all capital latin characters (without accents), for others it
		* is the ascent of the highest accented character, and finally, other
		* formats define it as being equal to bbox.yMax.
		*/
		float_t ascender;
		/**
		* The descender is the vertical distance from the horizontal baseline to
		* the lowest 'character' coordinate in a font face. Unfortunately, font
		* formats define the descender differently. For some, it represents the
		* descent of all capital latin characters (without accents), for others it
		* is the ascent of the lowest accented character, and finally, other
		* formats define it as being equal to bbox.yMin. This field is negative
		* for values below the baseline.
		*/
		float_t descender;
		/* The position of the underline line for this face. It is the center of the underlining stem. Only relevant for scalable formats. */
		float_t underline_position;
		/* The thickness of the underline for this face. Only relevant for scalable formats. */
		float_t underline_thickness;

	public:

		AtlasFont( const AtlasFont& that ) = delete;

		AtlasFont( AtlasFont&& that );

		GlyphEnumerator GlyphEnumerator( const std::string& str ) const;

		const AtlasGlyph& GetGlyph( utf8::uint32_t codepoint ) const
		{
			if (!IsLoaded( codepoint ))
			{
				std::ostringstream errorStream;
				errorStream << "Glyph not loaded for codepoint '" << codepoint << "'.";
				throw AtlasFontException( errorStream.str() );
			}

			return glyphs.at( codepoint );
		}

		AtlasFont( Atlas& atlas, const std::string& filename, float_t size, const std::string& charset );

	private:

		bool IsLoaded( uint32_t codepoint ) const
		{
			return glyphs.count( codepoint ) > 0;
		}

		std::unique_ptr<FT_LibraryRec_, FT_Library_Deleter> LoadFT_Library() const
		{
			FT_Library library;

			if (FT_Init_FreeType( &library )) {
				throw AtlasFontException( "Failed to initialize FreeType library." );
			}

			return std::unique_ptr<FT_LibraryRec_, FT_Library_Deleter>( library, FT_Done_FreeType );
		}

		std::unique_ptr<FT_FaceRec_, FT_Face_Deleter> LoadFT_Face( FT_Library library, const std::string& filename ) const
		{
			FT_Face face;

			if (FT_New_Face( library, filename.c_str(), 0, &face )) {
				throw AtlasFontException( "Failed to load font face." );
			}

			auto facePtr = std::unique_ptr<FT_FaceRec_, FT_Face_Deleter>( face, FT_Done_Face );

			if (FT_Select_Charmap( face, FT_ENCODING_UNICODE )) {
				throw AtlasFontException( "Failed to select char map." );
			}
			if (FT_Set_Char_Size( face, static_cast<int32_t>(size * HRES), 0, DPI * HRES, DPI )) {
				throw AtlasFontException( "Failed to set char size." );
			}

			FT_Matrix matrix = { static_cast<FT_Fixed>(1.0 / static_cast<float_t>(HRES) * 0x10000L), 0, 0, 0x10000L };
			FT_Set_Transform( face, &matrix, nullptr );

			return facePtr;
		}

		void LoadGlyphs( FT_Face face, const std::string& charset );

		void GenerateKerning( FT_Face face )
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
						glyph.Kerning.push_back( TextureFontKerning{ prevGlyph.Codepoint, kerning.x / static_cast<float_t>(HRES*HRES) } );
					}
				}
			}
		}
	};

	class GlyphEnumerator
	{
		CodepointEnumerator codepoints;
		const AtlasFont& font;
	public:
		GlyphEnumerator( const AtlasFont& font, const std::string& str ) : codepoints( str ), font( font ) {}
		const AtlasGlyph& Next() { return font.GetGlyph( codepoints.Next() ); }
		operator bool() const { return codepoints; }
	};
}
