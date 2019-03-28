#pragma once

#include "kodo-gl.hpp"

#include <utf8/utf8.h>
#include <glm/glm.hpp>

#include "AtlasFonts.hpp"

namespace kodogl
{
	class Atlas;
	class AtlasVertexBuffer;
	class GlyphEnumerator;

	template<typename TIntegral>
	class RangeIterator
	{
		TIntegral value;
		const TIntegral step;

	public:

		explicit RangeIterator( TIntegral value, TIntegral step ) :
			value( value ), step( step )
		{
			static_assert(std::is_integral<TIntegral>::value, "range_iterator only accepts integral types.");
		}

		bool operator != ( const RangeIterator& other ) const
		{
			return value != other.value;
		}

		const TIntegral& operator * () const
		{
			return value;
		}

		RangeIterator& operator++()
		{
			value += step;
			return *this;
		}
	};

	template<typename TIntegral>
	class IteratorRange
	{
		const TIntegral from;
		const TIntegral to;
		const TIntegral step;

	public:

		explicit IteratorRange( TIntegral to ) : IteratorRange( 0, to, 1 ) {}
		explicit IteratorRange( TIntegral from, TIntegral to, TIntegral step )
			: from( from ), to( to ), step( step )
		{
			static_assert(std::is_integral<TIntegral>::value, "range only accepts integral types.");
		}

		RangeIterator<TIntegral> begin() const { return RangeIterator<TIntegral>( from, step ); }
		RangeIterator<TIntegral> end() const { return RangeIterator<TIntegral>( to, step ); }
	};

	template<typename TIntegral>
	auto Range( TIntegral to )
	{
		return IteratorRange<TIntegral>( 0, to, 1 );
	}

	template<typename TIntegral>
	auto Range( TIntegral from, TIntegral to, TIntegral step )
	{
		return IteratorRange<TIntegral>( from, to, step );
	}

	struct AtlasLoaderFont;

	//
	// A class that represents a font within a texture atlas.
	//
	class AtlasFont
	{
	public:

		static constexpr auto NullCodepoint = size_t( -1 );
		static constexpr auto FallbackCodepoint = utf8::uint32_t( 63 );

		const float_t Size;

		Atlas& atlas;

	private:

		std::unordered_map<utf8::uint32_t, AtlasGlyph> glyphs;

		//
		// This field is simply used to compute a default line spacing (i.e., the baseline-to-baseline distance) when writing text with this font. 
		// Note that it usually is larger than the sum of the ascender and descender taken as absolute values. 
		// There is also no guarantee that no glyphs extend above or below subsequent baselines when using this distance.
		//
		float_t height;
		//
		// This field is the distance that must be placed between two lines of text. 
		// The baseline-to-baseline distance should be computed as: ascender - descender + linegap
		//
		float_t linegap;
		//
		// The ascender is the vertical distance from the horizontal baseline to the highest 'character' coordinate in a font face. 
		// Unfortunately, font formats define the ascender differently. 
		// For some, it represents the ascent of all capital latin characters (without accents), 
		// for others it is the ascent of the highest accented character, and finally,
		// other formats define it as being equal to bbox.yMax.
		//
		float_t ascender;
		//
		// The descender is the vertical distance from the horizontal baseline to the lowest 'character' coordinate in a font face. 
		// Unfortunately, font formats define the descender differently. 
		// For some, it represents the descent of all capital latin characters (without accents), 
		// for others it is the ascent of the lowest accented character, and finally, 
		// other formats define it as being equal to bbox.yMin. This field is negative for values below the baseline.
		//
		float_t descender;
		//
		// The position of the underline line for this face. It is the center of the underlining stem. 
		// Only relevant for scalable formats.
		//
		float_t underlinePosition;
		//
		// The thickness of the underline for this face. 
		// Only relevant for scalable formats.
		//
		float_t underlineThickness;

	public:

		float_t BaselineToBaseline;

		const Atlas& GetAtlas() const
		{
			return atlas;
		}

		AtlasFont( const AtlasFont& that ) = delete;
		AtlasFont( AtlasFont&& that );

		explicit AtlasFont( Atlas& atlas, AtlasLoaderFont&& font );

		GlyphEnumerator GlyphEnumerator( const std::string& str, bool fallBack = false ) const;

		bool HasGlyph( utf8::uint32_t codepoint ) const { return glyphs.count( codepoint ) > 0; }
		const AtlasGlyph& GetGlyph( utf8::uint32_t codepoint, bool fallback = false ) const;

		glm::vec2 Measure( const std::string& text ) const;
	};

	class GlyphEnumerator
	{
		CodepointEnumerator codepoints;
		const AtlasFont& font;
		bool fallback;

	public:

		GlyphEnumerator( const AtlasFont& font, const std::string& str, bool fallback ) :
			codepoints( str ),
			font( font ),
			fallback( fallback )
		{
		}

		const AtlasGlyph& Next()
		{
			auto codepoint = codepoints.Next();

			if (font.HasGlyph( codepoint ))
				return font.GetGlyph( AtlasFont::NullCodepoint );

			return font.GetGlyph( codepoint, fallback );
		}

		operator bool() const
		{
			return codepoints;
		}
	};
}
