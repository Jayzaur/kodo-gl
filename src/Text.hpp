#pragma once

#include "VertexBuffer.hpp"
#include "Atlas.hpp"
#include "AtlasFont.hpp"
#include "Brush.hpp"

namespace kodogl
{
	enum class TextAligment
	{
		Near,
		Mid,
		Far
	};

	class TextLayout
	{
		VertexBuffer<Vertex2f2f1f>& vertexBuffer;

		GLuint idOfVertices;

		glm::vec2 position;
		glm::vec2 dimensions;

	public:

		const glm::vec2& Dimensions() const
		{
			return dimensions;
		}

		TextLayout& operator = ( const TextLayout& other ) = delete;
		TextLayout& operator = ( TextLayout&& other )
		{
			if (vertexBuffer.Name() != other.vertexBuffer.Name())
				throw kodogl::exception( "Can't move from a different vertex buffer." );

			//Remove();

			position = other.position;
			dimensions = other.dimensions;
			idOfVertices = other.idOfVertices;

			other.idOfVertices = 0;

			return *this;
		}

		TextLayout( const TextLayout& ) = delete;
		TextLayout( TextLayout&& other ) :
			vertexBuffer( other.vertexBuffer )
		{
			position = other.position;
			dimensions = other.dimensions;
			idOfVertices = other.idOfVertices;

			other.idOfVertices = 0;
		}

		explicit TextLayout( const std::string& text, const AtlasFont& font, VertexBuffer<Vertex2f2f1f>& vertexBuffer, const glm::vec4& bounds, TextAligment xAlign = TextAligment::Near, TextAligment yAlign = TextAligment::Near ) :
			vertexBuffer( vertexBuffer )
		{
			auto calculatedWidth = 0.0f;
			auto calculatedHeight = 0.0f;
			auto calculatedYOffset = 10000000.0f;

			{
				CodepointEnumerator testcodepoints{ text };

				while (testcodepoints)
				{
					auto codepoint = testcodepoints.Next();

					if (codepoint == 10) // Line Feed U+000A
					{
						//textLocation.y -= font.BaselineToBaseline;
						//textLocation.x = 0;
						continue;
					}

					auto glyph = font.GetGlyph( codepoint, true );
					calculatedWidth += glyph.AdvanceX;
					calculatedHeight = glm::max( calculatedHeight, glyph.Height );
					calculatedYOffset = glm::min( calculatedYOffset, glyph.OffsetY - glyph.Height );
				}
			}

			dimensions = glm::vec2( calculatedWidth, glm::abs( calculatedHeight ) + glm::abs( calculatedYOffset ) );

			glm::vec2 textLocation;

			if (xAlign == TextAligment::Near)
			{
				textLocation.x = bounds.x;
			}
			else if (xAlign == TextAligment::Mid)
			{
				textLocation.x = bounds.x + (bounds.z - bounds.x) / dimensions.x;
			}
			else
			{
				textLocation.x = bounds.z - dimensions.x;
			}

			if (yAlign == TextAligment::Near)
			{
				textLocation.y = bounds.y;
			}
			else if (yAlign == TextAligment::Mid)
			{
				textLocation.y = bounds.y + (bounds.w - bounds.y) / dimensions.y;
			}
			else
			{
				textLocation.y = bounds.w - dimensions.y;
			}

			std::vector<Vertex2f2f1f> vertices;

			CodepointEnumerator codepoints{ text };

			while (codepoints)
			{
				auto codepoint = codepoints.Next();

				if (codepoint == 10) // Line Feed U+000A
				{
					textLocation.y -= font.BaselineToBaseline;
					textLocation.x = 0;
					continue;
				}

				auto glyph = font.GetGlyph( codepoint, true );
				auto x0 = textLocation.x + glyph.OffsetX;
				auto y0 = textLocation.y - glyph.OffsetY;
				auto x1 = x0 + glyph.Width;
				auto y1 = y0 + glyph.Height;
				auto s0 = glyph.Region.L;
				auto t0 = glyph.Region.T;
				auto s1 = glyph.Region.R;
				auto t1 = glyph.Region.B;

				vertices.emplace_back( x0, y0, s0, t0, 1.0f );
				vertices.emplace_back( x0, y1, s0, t1, 1.0f );
				vertices.emplace_back( x1, y1, s1, t1, 1.0f );
				vertices.emplace_back( x1, y0, s1, t0, 1.0f );

				textLocation.x += glyph.AdvanceX;
				textLocation.y += glyph.AdvanceY;

				calculatedWidth += glyph.AdvanceX;
				calculatedHeight = glm::max( glyph.Height, glyph.Height );
				calculatedYOffset = glm::min( calculatedYOffset, glyph.OffsetY - glyph.Height );
			}

			idOfVertices = vertexBuffer.PushQuads( vertices );
		}

		void Update( const glm::vec2& pos, const glm::vec4& col )
		{
			if (position != pos)
			{
				auto vertices = vertexBuffer.At( idOfVertices );
				auto delta = pos - position;

				for (auto i = vertices.StartOfVertices; i < vertices.StartOfVertices + vertices.CountOfVertices; i++)
				{
					auto& v = vertexBuffer.VertexAt( i );
					v.Vertex += delta;
				}

				position = pos;
			}
		}
	};
}
