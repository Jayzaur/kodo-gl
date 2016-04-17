#pragma once

#include "kodo-gl.hpp"

#include "Texture.h"

namespace kodogl
{
	class AtlasFont;

	class Atlas
	{
	public:

		static constexpr char* DefaultCharset = u8" -_.:,;<>|*'^?+´`!@abcdefghijklmnopqrstuvwxyzåäöABCDEFGHIJKLMOPQRSTUVWXYZÅÄÖ0123456789";
		static constexpr size_t Width = 512;
		static constexpr size_t Height = 512;

	private:

		struct AtlasNode
		{
			int32_t X;
			int32_t Y;
			int32_t Z;

			explicit AtlasNode() : X( 0 ), Y( 0 ), Z( 0 ) {}
			explicit AtlasNode( int32_t x, int32_t y, int32_t z ) : X( x ), Y( y ), Z( z ) {}
		};

		std::array<uint8_t, Width * Height> data;
		std::vector<AtlasNode> nodes;

		std::map<std::string, AtlasFont> fonts;

		GLuint amountUsed;
		GLuint idOfTexture;

	public:

		const AtlasFont& Get( const std::string& name )
		{
			return fonts.at( name );
		}

		//
		// The amount of space used. (0.0..1.0)
		//
		float_t Used() const
		{
			return static_cast<float_t>(amountUsed) / static_cast<float_t>(data.size());
		}

		GLuint Id() const
		{
			return idOfTexture;
		}

		const AtlasFont& Add( const std::string name, const std::string& filename, float_t size, const std::string& charset = DefaultCharset );
		//
		// Create a new TextureAtlas.
		//
		Atlas() :
			amountUsed( 0 ), idOfTexture( 0 )
		{
			data.fill( '\0' );
			nodes.push_back( AtlasNode{ 1, 1, Width - 2 } );
		}

		void Bind() const
		{
			gl::BindTexture( gl::TEXTURE_2D, idOfTexture );
		}

		//
		// Uploads the atlas texture to the GPU.
		//
		void Upload()
		{
			if (!idOfTexture)
				gl::GenTextures( 1, &idOfTexture );

			gl::BindTexture( gl::TEXTURE_2D, idOfTexture );
			gl::TexParameteri( gl::TEXTURE_2D, gl::TEXTURE_WRAP_S, gl::CLAMP_TO_EDGE );
			gl::TexParameteri( gl::TEXTURE_2D, gl::TEXTURE_WRAP_T, gl::CLAMP_TO_EDGE );
			gl::TexParameteri( gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, gl::LINEAR );
			gl::TexParameteri( gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::LINEAR );
			gl::TexImage2D( gl::TEXTURE_2D, 0, gl::RED, Width, Height, 0, gl::RED, gl::UNSIGNED_BYTE, data.data() );
		}

		//
		// Gets the requested region and indicates whether or not it fits in the atlas.
		//
		bool GetRegion( int32_t width, int32_t height, Region& region )
		{
			auto bestHeight = size_t( UINT_MAX );
			auto bestWidth = size_t( UINT_MAX );
			auto bestIndex = -1;
			auto fittedX = 0;
			auto fittedY = 0;

			for (size_t i = 0; i < nodes.size(); ++i)
			{
				auto y = Fit( i, width, height );

				if (y >= 0)
				{
					auto& node = nodes[i];

					if (static_cast<size_t>(y + height) < bestHeight ||
						 (y + height == bestHeight && (node.Z > 0 && static_cast<size_t>(node.Z) < bestWidth)))
					{
						bestHeight = y + height;
						bestIndex = i;
						bestWidth = node.Z;
						fittedX = node.X;
						fittedY = y;
					}
				}
			}

			if (bestIndex == -1)
			{
				return false;
			}

			nodes.insert( nodes.cbegin() + bestIndex, AtlasNode{ fittedX, fittedY + height, width } );

			for (size_t i = bestIndex + 1; i < nodes.size(); ++i)
			{
				auto& node = nodes[i];
				auto& prev = nodes[i - 1];

				if (node.X >= prev.X + prev.Z)
					break;

				auto shrink = prev.X + prev.Z - node.X;
				node.X += shrink;
				node.Z -= shrink;

				if (node.Z > 0)
					break;

				nodes.erase( nodes.cbegin() + i );
				--i;
			}

			Merge();
			amountUsed += width * height;
			region = Region{ fittedX, fittedY, width, height };
			return true;
		}

		void SetRegion( const Region& region, const uint8_t* data, size_t stride )
		{
			SetRegion( region.X, region.Y, region.W, region.H, data, stride );
		}

		void SetRegion( size_t x, size_t y, size_t width, size_t height, const uint8_t* dat, size_t stride )
		{
			assert( x > 0 );
			assert( y > 0 );
			assert( x < this->Width - 1 );
			assert( x + width <= this->Width - 1 );
			assert( y < this->Height - 1 );
			assert( y + height <= this->Height - 1 );

			auto charsize = sizeof( char );

			for (size_t i = 0; i < height; ++i)
			{
				memcpy( data.data() + ((y + i) * Width + x) * charsize,
						dat + (i * stride) * charsize,
						width * charsize );
			}
		}

		static NormalizedRegion Normalize( Region region )
		{
			return NormalizedRegion{
				NormalizeX( region.X ),
				NormalizeY( region.Y ),
				NormalizeX( region.X + region.W ),
				NormalizeY( region.Y + region.H ) };
		}

		static float_t NormalizeX( int32_t x )
		{
			return static_cast<float_t>(x) / static_cast<float_t>(Width);
		}

		static float_t NormalizeY( int32_t y )
		{
			return static_cast<float_t>(y) / static_cast<float_t>(Height);
		}

	private:

		int32_t Fit( size_t index, size_t width, size_t height )
		{
			auto node = nodes[index];
			auto x = node.X;
			auto y = node.Y;
			int32_t width_left = width;
			auto i = index;

			if (x + width > Width - 1)
				return -1;

			while (width_left > 0)
			{
				node = nodes[i];

				if (node.Y > y)
					y = node.Y;

				if (y + height > Height - 1)
					return -1;

				width_left -= node.Z;
				++i;
			}

			return y;
		}

		void Merge()
		{
			for (size_t i = 0; i < nodes.size() - 1; ++i)
			{
				auto& node = nodes[i];
				auto& next = nodes[i + 1];

				if (node.Y == next.Y)
				{
					node.Z += next.Z;
					nodes.erase( nodes.cbegin() + i + 1 );
					--i;
				}
			}
		}
	};
}
