#pragma once

#include "VertexBuffer.hpp"
#include "Shader.hpp"
#include "Brush.hpp"

namespace kodogl
{
	enum class CommandType : glm::uint8
	{
		None = 0,
		Color = 1,
		Texture = 2,
		TextureMask = 4,
	};

	class WindowContext;

	struct DrawingReference
	{
		glm::uint32 GeometryRef;
		glm::uint8 Layer;

		CommandType Type;

		WindowContext* Context;
		GenericVertexBuffer* Buffer;

		glm::uint32 ColorA;
		glm::uint32 ColorB;

		glm::uint32 TextureRef;

		DrawingReference() :
			GeometryRef( 0 ),
			TextureRef( 0 ),
			Layer( 0 ),
			Type( CommandType::None ),
			Context( nullptr ),
			Buffer( nullptr ),
			ColorA( 0 ),
			ColorB( 0 )
		{
		}

		//
		// Sorting operator implementation.
		//
		// Ordering:
		//     1. Layer
		//     2. Type
		//     3. Texture
		//	   4. Vertex
		//
		bool operator < ( const DrawingReference& other ) const
		{
			if (Layer == other.Layer)
			{
				if (Type == other.Type)
				{
					if (Type == CommandType::Texture || Type == CommandType::TextureMask)
					{
						return TextureRef < other.TextureRef;
					}

					return GeometryRef < other.GeometryRef;
				}

				return static_cast<glm::uint8>(Type) < static_cast<glm::uint8>(other.Type);
			}
			else
			{
				return Layer < other.Layer;
			}
		}
	};
}