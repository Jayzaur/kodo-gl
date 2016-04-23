#pragma once

#include "kodo-gl.hpp"

#include <glm/glm.hpp>

namespace kodogl
{
	class TextureException : public exception
	{
	public:
		explicit TextureException( std::string message ) : exception( message ) {}
	};

	class TextureLoader
	{
		FILE* fp;
		png_structp libpng;
		png_infop libpngInfo;
		png_infop linpngEndInfo;

	public:

		GLuint id;
		GLenum format;

		glm::vec2 dimensions;

		~TextureLoader()
		{
			fclose( fp );
			png_destroy_read_struct( &libpng, &libpngInfo, &linpngEndInfo );
		}

		void libpngUpdate( png_uint_32& pngWidth, png_uint_32& pngHeight, png_int_32& pngDepth, png_int_32& pngColor ) const
		{
			png_read_update_info( libpng, libpngInfo );
			png_get_IHDR( libpng, libpngInfo, &pngWidth, &pngHeight, &pngDepth, &pngColor, nullptr, nullptr, nullptr );
		}

		static void libpngError( png_structp, png_const_charp msg )
		{
			throw TextureException( "libpng: Unknown/unsupported color type -> " + std::string( msg ) );
		}

		explicit TextureLoader( const std::string& filename, bool onlyOpacity )
		{
			png_byte header[8];

			// Open the file.
			fopen_s( &fp, filename.c_str(), "rb" );
			if (fp == nullptr)
				throw TextureException( "Couldn't open -> " + filename );

			// Read the header.
			fread( header, 1, 8, fp );
			if (png_sig_cmp( header, 0, 8 ))
				throw TextureException( "Not a PNG file -> " + filename );

			// Create libpng structs.
			libpng = png_create_read_struct( PNG_LIBPNG_VER_STRING, nullptr, libpngError, nullptr );
			if (libpng == nullptr)
				throw TextureException( "png_create_read_struct failed -> " + filename );
			libpngInfo = png_create_info_struct( libpng );
			if (libpngInfo == nullptr)
				throw TextureException( "png_create_info_struct failed -> " + filename );
			linpngEndInfo = png_create_info_struct( libpng );
			if (linpngEndInfo == nullptr)
				throw TextureException( "png_create_info_struct failed -> " + filename );

			png_init_io( libpng, fp );
			png_set_sig_bytes( libpng, 8 );

			png_int_32 pngDepth, pngColor;
			png_uint_32 pngWidth, pngHeight;

			png_read_info( libpng, libpngInfo );
			png_get_IHDR( libpng, libpngInfo, &pngWidth, &pngHeight, &pngDepth, &pngColor, nullptr, nullptr, nullptr );

			if (pngDepth < 8) {
				png_set_expand_gray_1_2_4_to_8( libpng );
				libpngUpdate( pngWidth, pngHeight, pngDepth, pngColor );
			}
			if (pngColor == PNG_COLOR_TYPE_PALETTE) {
				png_set_palette_to_rgb( libpng );
				libpngUpdate( pngWidth, pngHeight, pngDepth, pngColor );
			}
			if (pngColor == PNG_COLOR_TYPE_RGB || pngColor == PNG_COLOR_TYPE_GRAY) {
				png_set_add_alpha( libpng, 1, PNG_FILLER_AFTER );
				libpngUpdate( pngWidth, pngHeight, pngDepth, pngColor );
			}

			if (onlyOpacity)
			{
				if (pngColor != PNG_COLOR_TYPE_GRAY_ALPHA)
				{
					png_set_rgb_to_gray( libpng, 1, 0, 0 );
					libpngUpdate( pngWidth, pngHeight, pngDepth, pngColor );
				}
			}
			else if (pngColor != PNG_COLOR_TYPE_RGB_ALPHA)
			{
				if (pngColor == PNG_COLOR_TYPE_GRAY_ALPHA)
				{
					png_set_gray_to_rgb( libpng );
					libpngUpdate( pngWidth, pngHeight, pngDepth, pngColor );
				}
				if (pngColor != PNG_COLOR_TYPE_RGB_ALPHA)
					throw TextureException( "Unknown/unsupported color type -> " + filename );
			}

			// Get the row byte count and make sure alignment is correct.
			auto bytesPerRow = png_get_rowbytes( libpng, libpngInfo );
			bytesPerRow += 3 - ((bytesPerRow - 1) % 4);

			// Allocate the image data.
			std::vector<png_byte> imageData( static_cast<size_t>(bytesPerRow * pngHeight), 0 );
			std::vector<png_byte*> imageRows( pngHeight );

			// Set the row pointers to the correct offsets in image data.
			for (png_uint_32 i = 0; i < pngHeight; i++)
				imageRows[pngHeight - 1 - i] = imageData.data() + i * bytesPerRow;

			// Read the PNG.
			png_read_image( libpng, imageRows.data() );

			dimensions = glm::vec2( pngWidth, pngHeight );

			// Generate the OpenGL texture.
			gl::GenTextures( 1, &id );
			gl::BindTexture( gl::TEXTURE_2D, id );

			if (onlyOpacity)
			{
				//
				// Read only the alpha channel.
				//
				std::vector<png_byte> imageData2( imageData.size() / 2, 0 );

				for (size_t i = 0, ii = 1; i < imageData2.size(); i++, ii += 2)
					imageData2[i] = imageData[ii];

				format = gl::R8;
				gl::TexStorage2D( gl::TEXTURE_2D, 4, format, pngWidth, pngHeight );
				gl::TexSubImage2D( gl::TEXTURE_2D, 0, 0, 0, pngWidth, pngHeight, gl::RED, gl::UNSIGNED_BYTE, imageData2.data() );
			}
			else
			{
				format = gl::RGBA8;
				gl::TexStorage2D( gl::TEXTURE_2D, 4, format, pngWidth, pngHeight );
				gl::TexSubImage2D( gl::TEXTURE_2D, 0, 0, 0, pngWidth, pngHeight, gl::RGBA, gl::UNSIGNED_BYTE, imageData.data() );
			}

			gl::GenerateMipmap( gl::TEXTURE_2D );
			gl::TexParameterf( gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::LINEAR_MIPMAP_LINEAR );
			gl::TexParameterf( gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, gl::LINEAR );
		}
	};

	class Texture
	{
		GLuint nameOfTeture;
		GLenum format;

		glm::vec2 dimensions;

	public:

		GLuint Name() const
		{
			return nameOfTeture;
		}

		explicit Texture( const std::string& filename, bool onlyOpacity )
		{
			TextureLoader loader( filename, onlyOpacity );
			nameOfTeture = loader.id;
			format = loader.format;
			dimensions = loader.dimensions;
		}

		void Bind() const
		{
			gl::BindTexture( gl::TEXTURE_2D, nameOfTeture );
		}
	};
}
