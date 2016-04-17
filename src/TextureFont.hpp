#pragma once

#include "kodo-gl.hpp"

#include <utf8/utf8.h>
#include <glm/glm.hpp>

#include "Texture.h"

#include "VertexBuffer.hpp"

namespace kodogl
{
	class Atlas;
	class AtlasVertexBuffer;
	class GlyphEnumerator;

	struct AtlasVertex
	{
		float_t x, y;
		float_t s, t;
		float_t r, g, b, a;

		AtlasVertex( float_t x, float_t y, float_t s, float_t t, const glm::vec4& color ) :
			x( x ), y( y ),
			s( s ), t( t ),
			r( color.r ), g( color.g ), b( color.b ), a( color.a )
		{
		}

		AtlasVertex( const glm::vec2& position, const glm::vec2& texture, const glm::vec4& color ) :
			x( position.x ), y( position.y ),
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

		// A vector of kerning pairs relative to this glyph.
		std::vector<TextureFontKerning> Kerning;

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
		typedef decltype(&FT_Done_FreeType) FT_Library_Deleter;
		typedef decltype(&FT_Done_Face) FT_Face_Deleter;

		static constexpr float_t HRES = 64;
		static constexpr float_t DPI = 96;

		const float_t Size;

		Atlas& atlas;

		const std::string filename;
		std::map<utf8::uint32_t, AtlasGlyph> glyphs;

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

		AtlasFont( const AtlasFont& that ) = delete;
		AtlasFont( AtlasFont&& that );
		AtlasFont( Atlas& atlas, const std::string& filename, float_t size, const std::string& charset );

		GlyphEnumerator GlyphEnumerator( const std::string& str ) const;

		const AtlasGlyph& GetGlyph( utf8::uint32_t codepoint ) const;

		glm::vec2 Measure( const std::string& text ) const;

		//
		// Calculates the area (in pixels) for a string of text placed at the specified position.
		//
		glm::vec4 Measure( const glm::vec2& pos, const std::string& text ) const;

	private:

		bool IsLoaded( utf8::uint32_t codepoint ) const;
		void GenerateGlyphs( FT_Face face, const std::string& charset );
		void GenerateKerning( FT_Face face );

		static std::unique_ptr<FT_LibraryRec_, FT_Library_Deleter> LoadFT_Library()
		{
			FT_Library library;

			if (FT_Init_FreeType( &library ))
				throw AtlasFontException( "FT_Init_FreeType failed." );

			return std::unique_ptr<FT_LibraryRec_, FT_Library_Deleter>( library, FT_Done_FreeType );
		}

		static std::unique_ptr<FT_FaceRec_, FT_Face_Deleter> LoadFT_Face( FT_Library library, const std::string& filename, float_t size )
		{
			FT_Face face;

			if (FT_New_Face( library, filename.c_str(), 0, &face ))
				throw AtlasFontException( "FT_New_Face failed." );

			auto facePtr = std::unique_ptr<FT_FaceRec_, FT_Face_Deleter>( face, FT_Done_Face );

			if (FT_Select_Charmap( face, FT_ENCODING_UNICODE ))
				throw AtlasFontException( "FT_Select_Charmap failed." );
			if (FT_Set_Char_Size( face, static_cast<FT_F26Dot6>(size * HRES), 0, static_cast<FT_UInt>(DPI * HRES), static_cast<FT_UInt>(DPI) ))
				throw AtlasFontException( "FT_Set_Char_Size failed." );

			FT_Matrix matrix = { static_cast<FT_Fixed>(1.0 / HRES * 0x10000L), 0, 0, 0x10000L };
			FT_Set_Transform( face, &matrix, nullptr );

			return facePtr;
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

	class AtlasVertexBuffer
	{
		static constexpr auto SizeOfVertex = sizeof( AtlasVertex );
		static constexpr uint8_t IndicesOfVertex[6] = { 0,1,2, 0,2,3 };

		enum class VertexBufferState
		{
			Clean,
			Dirty,
			Frozen
		};

		struct VertexBufferItem
		{
			static constexpr uint32_t CountOfIndices = 6;
			static constexpr uint32_t CountOfVertices = 4;

			uint32_t StartOfIndices;
			uint32_t StartOfVertices;

			VertexBufferItem( uint32_t si, uint32_t sv ) : StartOfIndices( si ), StartOfVertices( sv ) { }
		};

		// Vector of attributes.
		std::array<VertexAttribute, 3> attributes;
		// Vector of vertices.
		std::vector<AtlasVertex> vertices;
		// Vector of indices.
		std::vector<GLuint> indices;
		// Map of items.
		std::map<GLint, VertexBufferItem> stored;

		// GL identity of the Vertex Array Object.
		GLuint idOfVAO;
		// GL identity of the vertices buffer.
		GLuint idOfVertices;
		// GL identity of the indices buffer.
		GLuint idOfIndices;

		// Current size of the vertices buffer in the GPU.
		size_t sizeofGPUVertices;
		// Current size of the indices buffer in the GPU.
		size_t sizeofGPUIndices;

		// GL primitives to render.
		GLenum mode;

		// Attribute stride.
		size_t attributeStride;

		// State of the vertex buffer.
		VertexBufferState state;

		GLint idCounter;

	public:

		size_t size() const
		{
			return stored.size();
		}

		explicit AtlasVertexBuffer() :
			idOfVAO( 0 ), idOfVertices( 0 ), idOfIndices( 0 ),
			sizeofGPUVertices( 0 ), sizeofGPUIndices( 0 ),
			mode( 0 ),
			state( VertexBufferState::Dirty ),
			idCounter( 0 )
		{
			size_t stride = 0;
			GLchar *pointer = nullptr;

			attributes[0] = VertexAttribute{ "vertex", 2, gl::FLOAT };
			attributes[0].Pointer = pointer;
			stride += 2 * sizeof( GLfloat );
			pointer += 2 * sizeof( GLfloat );
			attributes[1] = VertexAttribute{ "tex_coord", 2, gl::FLOAT };
			attributes[1].Pointer = pointer;
			stride += 2 * sizeof( GLfloat );
			pointer += 2 * sizeof( GLfloat );
			attributes[2] = VertexAttribute{ "color", 4, gl::FLOAT };
			attributes[2].Pointer = pointer;
			stride += 4 * sizeof( GLfloat );
			pointer += 4 * sizeof( GLfloat );

			attributeStride = stride;
		}

		~AtlasVertexBuffer()
		{
			state = VertexBufferState::Frozen;

			stored.clear();
			indices.clear();
			vertices.clear();

			if (idOfVAO != 0) {
				gl::DeleteVertexArrays( 1, &idOfVAO );
				idOfVAO = 0;
			}
			if (idOfVertices != 0) {
				gl::DeleteBuffers( 1, &idOfVertices );
				idOfVertices = 0;
			}
			if (idOfIndices != 0) {
				gl::DeleteBuffers( 1, &idOfIndices );
				idOfIndices = 0;
			}
		}

		//
		// Upload the vertex buffer to the GPU.
		//
		void Upload()
		{
			// Do nothing if frozen.
			if (state == VertexBufferState::Frozen)
				return;

			if (idOfVertices == 0)
				gl::GenBuffers( 1, &idOfVertices );
			if (idOfIndices == 0)
				gl::GenBuffers( 1, &idOfIndices );

			//
			// Upload vertices
			// Always upload vertices first such that indices do not point to non existing data.
			//
			{
				auto sizeofVertices = vertices.size() * SizeOfVertex;

				gl::BindBuffer( gl::ARRAY_BUFFER, idOfVertices );

				if (sizeofVertices == sizeofGPUVertices)
				{
					gl::BufferSubData( gl::ARRAY_BUFFER, 0, sizeofVertices, vertices.data() );
				}
				else
				{
					gl::BufferData( gl::ARRAY_BUFFER, sizeofVertices, vertices.data(), gl::DYNAMIC_DRAW );
					sizeofGPUVertices = sizeofVertices;
				}

				gl::BindBuffer( gl::ARRAY_BUFFER, 0 );
			}

			//
			// Upload indices
			//
			{
				auto sizeofIndices = indices.size() * sizeof( GLuint );

				gl::BindBuffer( gl::ELEMENT_ARRAY_BUFFER, idOfIndices );

				if (sizeofIndices == sizeofGPUIndices)
				{
					gl::BufferSubData( gl::ELEMENT_ARRAY_BUFFER, 0, sizeofIndices, indices.data() );
				}
				else
				{
					gl::BufferData( gl::ELEMENT_ARRAY_BUFFER, sizeofIndices, indices.data(), gl::DYNAMIC_DRAW );
					sizeofGPUIndices = sizeofIndices;
				}

				gl::BindBuffer( gl::ELEMENT_ARRAY_BUFFER, 0 );
			}
		}

		//
		// Clear the vertex buffer.
		//
		void Clear()
		{
			state = VertexBufferState::Frozen;
			stored.clear();
			indices.clear();
			vertices.clear();
		}

	private:

		void EnableAttribute( VertexAttribute& attribute ) const
		{
			if (attribute.Index == -1)
			{
				GLint program;
				gl::GetIntegerv( gl::CURRENT_PROGRAM, &program );

				if (program == 0)
					throw VertexBufferException( "No program currently active." );

				attribute.Index = gl::GetAttribLocation( program, attribute.Name );

				if (attribute.Index == -1)
					throw VertexBufferException( "Couldn't find the attribute." );
			}

			gl::EnableVertexAttribArray( attribute.Index );
			gl::VertexAttribPointer( attribute.Index, attribute.Components, attribute.Type, attribute.normalized, attributeStride, attribute.Pointer );
		}

		void Bind()
		{
			// Unbind so no existing VAO-state is overwritten, (e.g. the GL_ELEMENT_ARRAY_BUFFER-binding).
			Unbind();

			if (state != VertexBufferState::Clean)
			{
				Upload();
				state = VertexBufferState::Clean;
			}

			if (idOfVAO == 0)
			{
				// Generate and set up VAO.
				gl::GenVertexArrays( 1, &idOfVAO );
				gl::BindVertexArray( idOfVAO );

				{
					gl::BindBuffer( gl::ARRAY_BUFFER, idOfVertices );

					for (auto& attribute : attributes)
					{
						EnableAttribute( attribute );
					}

					gl::BindBuffer( gl::ARRAY_BUFFER, 0 );
				}

				gl::BindBuffer( gl::ELEMENT_ARRAY_BUFFER, idOfIndices );
			}

			// Bind VAO for drawing.
			gl::BindVertexArray( idOfVAO );
		}

		static void Unbind()
		{
			gl::BindVertexArray( 0 );
		}

	public:

		void Render()
		{
			Bind();

			gl::DrawElements( gl::TRIANGLES, indices.size(), gl::UNSIGNED_INT, nullptr );

			Unbind();
		}

		void Remove( GLint id );
		void Modify( GLint id, const AtlasFont& font, const std::string& text, const glm::vec2& position, const glm::vec4& color );
		void Add( const AtlasFont& font, const std::string& text, const glm::vec2& position, const glm::vec4& color, GLint& id );

		/*void Erase( size_t index )
		{
			auto& item = items[index];
			auto vstart = item.vstart;
			auto vcount = item.vcount;
			auto istart = item.istart;
			auto icount = item.icount;

			// Update items.
			for (size_t i = 0; i < items.size(); ++i)
			{
				item = items[i];

				if (item.vstart > vstart)
				{
					item.vstart -= vcount;
					item.istart -= icount;
				}
			}

			EraseIndices( istart, istart + icount );
			EraseVertices( vstart, vstart + vcount );
			items.erase( items.begin() + index );
			state = VertexBufferState::Dirty;
		}*/

	private:

		void Insert( size_t index, const std::vector<GLuint>& range )
		{
			state = VertexBufferState::Dirty;
			std::copy( range.begin(), range.end(), std::inserter( indices, indices.begin() + index ) );
		}

		void Insert( size_t index, const std::vector<AtlasVertex>& range )
		{
			state = VertexBufferState::Dirty;

			for (auto& i : indices) {
				if (i > index) {
					i += index;
				}
			}

			std::copy( range.begin(), range.end(), std::inserter( vertices, vertices.begin() + index ) );
		}

		void EraseIndices( size_t first, size_t last )
		{
			state = VertexBufferState::Dirty;
			indices.erase( indices.cbegin() + first, indices.cbegin() + last );
		}

		void EraseVertices( size_t first, size_t last )
		{
			state = VertexBufferState::Dirty;

			for (auto& i : indices) {
				if (i > first) {
					i -= last - first;
				}
			}

			vertices.erase( vertices.cbegin() + first, vertices.cbegin() + last );
		}
	};
}
