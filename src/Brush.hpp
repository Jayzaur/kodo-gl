#pragma once

namespace kodogl
{
	enum class BrushType
	{
		Linear,
		Texture,
		TextureMask
	};

	struct Brush
	{
		BrushType Type;
		glm::uint8 Opacity;

		void SetOpacity( glm::uint8 opacity )
		{
			Opacity = opacity;
		}

	protected:

		Brush( BrushType type, glm::uint8 opacity = 255 ) : Type( type ), Opacity( opacity ) {}
	};

	struct ColorBrush : Brush
	{
		glm::vec4 Weights;
		glm::uint32 ColorA;
		glm::uint32 ColorB;

		ColorBrush( glm::uint32 color, glm::uint8 opacity = 255 ) :
			Brush( BrushType::Linear, opacity ),
			Weights( glm::vec4( 1.0f ) ),
			ColorA( color ),
			ColorB( color )
		{
		}

		ColorBrush( glm::uint32 colorA, glm::uint32 colorB, glm::uint8 opacity = 255 ) :
			Brush( BrushType::Linear, opacity ),
			Weights( glm::vec4( 0.0f, 1.0f, 1.0f, 0.0f ) ),
			ColorA( colorA ),
			ColorB( colorB )
		{
		}

		ColorBrush( const glm::vec4& color, glm::uint8 opacity = 255 ) : ColorBrush( glm::vec4( 1.0f ), color, color, opacity ) { }
		ColorBrush( const glm::vec4& colorA, const glm::vec4& colorB, glm::uint8 opacity = 255 ) : ColorBrush( glm::vec4( 0.0f, 1.0f, 1.0f, 0.0f ), colorA, colorB, opacity ) { }
		ColorBrush( const glm::vec4& weights, const glm::vec4& colorA, const glm::vec4& colorB, glm::uint8 opacity = 255 ) :
			Brush( BrushType::Linear, opacity ),
			Weights( weights ),
			ColorA( glm::packUnorm4x8( colorA ) ),
			ColorB( glm::packUnorm4x8( colorB ) )
		{
		}

		void Color( const glm::vec4& color )
		{
			ColorA = glm::packUnorm4x8( color );
			ColorB = ColorA;
		}
	};

	struct TextureBrush : Brush
	{

	};

	struct TextureMaskBrush : ColorBrush
	{
		
	};
}
