#pragma once

#include <glm/glm.hpp>

#include "VertexBuffer.hpp"

namespace kodogl
{
	struct Region
	{
		int32_t X;
		int32_t Y;
		int32_t W;
		int32_t H;

		Region() : X( 0 ), Y( 0 ), W( 0 ), H( 0 ) {}
		Region( int32_t x, int32_t y, int32_t width, int32_t height ) : X( x ), Y( y ), W( width ), H( height ) { }
	};

	struct NormalizedRegion
	{
		float_t L, T, R, B;

		glm::vec2 LeftBottom() const { return glm::vec2( L, T ); }
		glm::vec2 LeftTop() const { return glm::vec2( L, B ); }
		glm::vec2 RightBottom() const { return glm::vec2( R, T ); }
		glm::vec2 RightTop() const { return glm::vec2( R, B ); }

		NormalizedRegion() : L( 0 ), T( 0 ), R( 0 ), B( 0 ) {}
		NormalizedRegion( float_t left, float_t top, float_t right, float_t bottom ) : L( left ), T( top ), R( right ), B( bottom ) { }
	};

	struct AtlasVertex
	{
		glm::vec2 Vertex;
		glm::vec2 Texture;
		glm::vec4 Color;

		AtlasVertex() {}
		AtlasVertex( float_t x, float_t y, float_t s, float_t t, const glm::vec4& c ) : AtlasVertex( glm::vec2( x, y ), glm::vec2( s, t ), c ) {}
		AtlasVertex( const glm::vec2& vp, const glm::vec2& tp, const glm::vec4& c ) : Vertex( vp ), Texture( tp ), Color( c ) {}

		static std::array<VertexAttribute, 3> Attributes()
		{
			return{
				VertexAttribute{ 0, 2, gl::FLOAT, false, reinterpret_cast<GLvoid*>(offsetof( AtlasVertex, Vertex )) },
				VertexAttribute{ 1, 2, gl::FLOAT, false, reinterpret_cast<GLvoid*>(offsetof( AtlasVertex, Texture )) },
				VertexAttribute{ 2, 4, gl::FLOAT, false, reinterpret_cast<GLvoid*>(offsetof( AtlasVertex, Color )) }
			};
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
	// A structure that describes a kerning value relatively to a Unicode codepoint.
	// 
	struct TextureFontKerning
	{
		// Unicode codepoint in UTF-32 LE encoding.
		utf8::uint32_t Codepoint;
		// Kerning value (in fractional pixels).
		float_t Value;
	};

	//
	// A structure that describes a glyph.
	//
	struct AtlasGlyph
	{
		// Unicode codepoint in UTF-32 LE encoding.
		const uint32_t Codepoint;

		// Glyph's width in pixels.
		const float_t Width;
		// Glyph's height in pixels.
		const float_t Height;

		// Glyph's left bearing in pixels.
		const float_t OffsetX;
		// Glyphs's top bearing in pixels.
		const float_t OffsetY;

		// For horizontal text layouts, this is the horizontal distance (in fractional pixels) 
		// used to increment the pen position when the glyph is drawn as part of a string of text.
		const float_t AdvanceX;
		// For vertical text layouts, this is the vertical distance (in fractional pixels) 
		// used to increment the pen position when the glyph is drawn as part of a string of text.
		const float_t AdvanceY;

		// Normalized region occupied by this glyph within a texture atlas.
		const NormalizedRegion Region;

		AtlasGlyph() : Codepoint( 0 ), Width( 0 ), Height( 0 ), OffsetX( 0 ), OffsetY( 0 ), AdvanceX( 0 ), AdvanceY( 0 ) {}
		AtlasGlyph( uint32_t codepoint, const NormalizedRegion& region ) :
			Codepoint( codepoint ),
			Width( 0 ), Height( 0 ),
			OffsetX( 0 ), OffsetY( 0 ),
			AdvanceX( 0 ), AdvanceY( 0 ),
			Region( region )
		{
		}
		AtlasGlyph( uint32_t cp, float_t w, float_t h, float_t xoff, float_t yoff, float_t xadv, float_t yadv, const NormalizedRegion& region ) :
			Codepoint( cp ),
			Width( w ), Height( h ),
			OffsetX( xoff ), OffsetY( yoff ),
			AdvanceX( xadv ), AdvanceY( yadv ),
			Region( region )
		{
		}
	};

	struct AtlasLoaderFont
	{
		const float_t Size;
		float_t height;
		float_t linegap;
		float_t ascender;
		float_t descender;
		float_t underlinePosition;
		float_t underlineThickness;
		float_t BaselineToBaseline;

		std::unordered_map<utf8::uint32_t, AtlasGlyph> glyphs;

		explicit AtlasLoaderFont( float_t size ) : Size( size ), height( 0 ), linegap( 0 ), ascender( 0 ), descender( 0 ), underlinePosition( 0 ), underlineThickness( 0 ), BaselineToBaseline( 0 )
		{

		}
	};
}
