#pragma once

#include "kodo-gl.hpp"

#include <glm/glm.hpp>
#include <utf8/utf8.h>

#include "AtlasFonts.hpp"

namespace kodogl
{
	class AtlasFont;

	class AtlasException : public exception
	{
	public:
		explicit AtlasException(std::string message) : exception(message) {}
	};

	struct AtlasNode
	{
		int32_t X;
		int32_t Y;
		int32_t Z;

		explicit AtlasNode() : X(0), Y(0), Z(0) {}
		explicit AtlasNode(int32_t x, int32_t y, int32_t z) : X(x), Y(y), Z(z) {}
	};

	class Atlas;

	class AtlasLoader
	{
		static constexpr char* DefaultCharset = u8" -_.:,;<>|*'^?+´`!@\"#%&/\\~={}[]()€$abcdefghijklmnopqrstuvwxyzåäöABCDEFGHIJKLMOPQRSTUVWXYZÅÄÖ0123456789";
		static constexpr float_t HRES = 64;
		static constexpr float_t DPI = 96;

		typedef decltype(&FT_Done_FreeType) FT_Library_Deleter;
		typedef decltype(&FT_Done_Face) FT_Face_Deleter;

		std::unique_ptr<FT_LibraryRec_, FT_Library_Deleter> library;

		size_t widthOfTexture;
		size_t heightOfTexture;

		size_t usedOfTexture;

		std::vector<uint8_t> data;
		std::vector<AtlasNode> nodes;
		std::vector<AtlasLoaderFont> fonts;

		bool automaticResize;

	public:

		//
		// The amount of space used within the GPU texture. (0.0..1.0)
		//
		float_t UsedOfTexture() const
		{
			return static_cast<float_t>(usedOfTexture) / static_cast<float_t>(data.size());
		}

		static std::unique_ptr<FT_LibraryRec_, FT_Library_Deleter> LoadFT_Library()
		{
			FT_Library library;

			if (FT_Init_FreeType(&library))
				throw AtlasException("FT_Init_FreeType failed.");

			return std::unique_ptr<FT_LibraryRec_, FT_Library_Deleter>(library, FT_Done_FreeType);
		}

		static std::unique_ptr<FT_FaceRec_, FT_Face_Deleter> LoadFT_Face(FT_Library library, const uint8_t* data, size_t dataSize, float_t size)
		{
			FT_Face face;

			if (FT_New_Memory_Face(library, data, dataSize, 0, &face))
				throw AtlasException("FT_New_Face failed.");

			auto facePtr = std::unique_ptr<FT_FaceRec_, FT_Face_Deleter>(face, FT_Done_Face);

			if (FT_Select_Charmap(face, FT_ENCODING_UNICODE))
				throw AtlasException("FT_Select_Charmap failed.");
			if (FT_Set_Char_Size(face, static_cast<FT_F26Dot6>(size * HRES), 0, static_cast<FT_UInt>(DPI * HRES), static_cast<FT_UInt>(DPI)))
				throw AtlasException("FT_Set_Char_Size failed.");

			FT_Matrix matrix = { static_cast<FT_Fixed>(1.0 / HRES * 0x10000L), 0, 0, 0x10000L };
			FT_Set_Transform(face, &matrix, nullptr);

			return facePtr;
		}

		static std::unique_ptr<FT_FaceRec_, FT_Face_Deleter> LoadFT_Face(FT_Library library, const std::string& filename, float_t size)
		{
			FT_Face face;

			if (FT_New_Face(library, filename.c_str(), 0, &face))
				throw AtlasException("FT_New_Face failed.");

			auto facePtr = std::unique_ptr<FT_FaceRec_, FT_Face_Deleter>(face, FT_Done_Face);

			if (FT_Select_Charmap(face, FT_ENCODING_UNICODE))
				throw AtlasException("FT_Select_Charmap failed.");
			if (FT_Set_Char_Size(face, static_cast<FT_F26Dot6>(size * HRES), 0, static_cast<FT_UInt>(DPI * HRES), static_cast<FT_UInt>(DPI)))
				throw AtlasException("FT_Set_Char_Size failed.");

			FT_Matrix matrix = { static_cast<FT_Fixed>(1.0 / HRES * 0x10000L), 0, 0, 0x10000L };
			FT_Set_Transform(face, &matrix, nullptr);

			return facePtr;
		}

		AtlasLoader(size_t width, size_t height, bool automaticResize = false) :
			widthOfTexture(width), heightOfTexture(height), usedOfTexture(0),
			automaticResize(automaticResize),
			library(LoadFT_Library())
		{
			data.resize(width * height, 0);
			nodes.push_back(AtlasNode{ 1, 1, static_cast<int32_t>(widthOfTexture - 2) });
		}

		std::unique_ptr<Atlas> Finish();

		GLuint Load(const uint8_t* buffer, size_t bufferSize, float_t size, const std::string& charset = DefaultCharset)
		{
			auto face = LoadFT_Face(library.get(), buffer, bufferSize, size);

			AtlasLoaderFont font(size);

			font.underlinePosition = face->underline_position / (HRES * HRES) * size;
			font.underlinePosition = glm::min(-2.0f, glm::round(font.underlinePosition));
			font.underlineThickness = face->underline_thickness / (HRES * HRES) * size;
			font.underlineThickness = glm::max(1.0f, glm::round(font.underlineThickness));

			auto metrics = face->size->metrics;
			font.ascender = (metrics.ascender >> 6) / 100.0f;
			font.descender = (metrics.descender >> 6) / 100.0f;
			font.height = (metrics.height >> 6) / 100.0f;
			font.linegap = font.height - font.ascender + font.descender;

			font.BaselineToBaseline = (font.ascender - font.descender + font.linegap) * 100.0f;

			GenerateGlyphs(font, face.get(), u8"?" + charset);

			auto nameOfFont = static_cast<GLuint>(fonts.size());
			fonts.emplace_back(font);
			return nameOfFont;
		}

		GLuint Load(const std::string& filename, float_t size, const std::string& charset = DefaultCharset)
		{
			auto face = LoadFT_Face(library.get(), filename, size);

			AtlasLoaderFont font(size);

			font.underlinePosition = face->underline_position / (HRES * HRES) * size;
			font.underlinePosition = glm::min(-2.0f, glm::round(font.underlinePosition));
			font.underlineThickness = face->underline_thickness / (HRES * HRES) * size;
			font.underlineThickness = glm::max(1.0f, glm::round(font.underlineThickness));

			auto metrics = face->size->metrics;
			font.ascender = (metrics.ascender >> 6) / 100.0f;
			font.descender = (metrics.descender >> 6) / 100.0f;
			font.height = (metrics.height >> 6) / 100.0f;
			font.linegap = font.height - font.ascender + font.descender;

			font.BaselineToBaseline = (font.ascender - font.descender + font.linegap) * 100.0f;

			GenerateGlyphs(font, face.get(), u8"?" + charset);

			auto nameOfFont = static_cast<GLuint>(fonts.size());
			fonts.emplace_back(font);
			return nameOfFont;
		}

		void GenerateGlyphs(AtlasLoaderFont& font, FT_Face face, const std::string& charset)
		{
			CodepointEnumerator codepoints{ charset };

			while (codepoints)
			{
				auto codepoint = codepoints.Next();

				// Maybe the glyph is already loaded?
				if (font.glyphs.count(codepoint))
					continue;

				auto glyphIndex = FT_Get_Char_Index(face, static_cast<FT_ULong>(codepoint));

				if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT))
					throw AtlasException("FT_Load_Glyph failed.");

				auto glyphSlot = face->glyph;
				auto ft_bitmap = glyphSlot->bitmap;
				auto ft_glyph_top = glyphSlot->bitmap_top;
				auto ft_glyph_left = glyphSlot->bitmap_left;
				auto glyphWidth = ft_bitmap.width;
				auto glyphHeight = ft_bitmap.rows;

				Region atlasRegion;

				// We want each glyph to be separated by at least one black pixel.
				if (!GetRegion(glyphWidth + 1, glyphHeight + 1, atlasRegion))
					throw AtlasException("Atlas is full.");

				Region glyphRegion{ atlasRegion.X, atlasRegion.Y, glyphWidth, glyphHeight };

				SetRegion(glyphRegion, ft_bitmap.buffer, ft_bitmap.pitch);

				// Discard hinting to get advance.
				FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER | FT_LOAD_NO_HINTING);

				AtlasGlyph glyph{
					codepoint,
					static_cast<float_t>(glyphWidth),
					static_cast<float_t>(glyphHeight),
					static_cast<float_t>(ft_glyph_left),
					static_cast<float_t>(ft_glyph_top),
					static_cast<float_t>(face->glyph->advance.x) / HRES,
					static_cast<float_t>(face->glyph->advance.y) / HRES,
					Normalize(glyphRegion)
				};

				font.glyphs.emplace(codepoint, glyph);
			}

			GenerateKerning(font, face);
		}

		void GenerateKerning(AtlasLoaderFont& font, FT_Face face)
		{
			//
			// For each glyph couple combination, check if kerning is necessary.
			//
			for (auto& kp : font.glyphs)
			{
				auto& glyph = kp.second;
				auto glyphIndex = FT_Get_Char_Index(face, glyph.Codepoint);

				// Skip the null glyph.
				if (glyph.Codepoint == size_t(-1))
					continue;

				//glyph.Kerning.clear();

				for (const auto& kpj : font.glyphs)
				{
					auto prevGlyph = kpj.second;
					auto prevIndex = FT_Get_Char_Index(face, prevGlyph.Codepoint);

					FT_Vector kerning;
					FT_Get_Kerning(face, prevIndex, glyphIndex, FT_KERNING_UNFITTED, &kerning);

					if (kerning.x)
					{
						//glyph.Kerning.push_back( TextureFontKerning{ prevGlyph.Codepoint, kerning.x / static_cast<float_t>(HRES * HRES) } );
					}
				}
			}
		}

		//
		// Gets the requested region and indicates whether or not it fits in the atlas.
		//
		bool GetRegion(int32_t width, int32_t height, Region& region)
		{
			auto bestHeight = size_t(UINT_MAX);
			auto bestWidth = size_t(UINT_MAX);
			auto bestIndex = -1;
			auto fittedX = 0;
			auto fittedY = 0;

			for (size_t i = 0; i < nodes.size(); ++i)
			{
				auto y = Fit(i, width, height);

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

			nodes.insert(nodes.cbegin() + bestIndex, AtlasNode{ fittedX, fittedY + height, width });

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

				nodes.erase(nodes.cbegin() + i);
				--i;
			}

			Merge();
			usedOfTexture += width * height;
			region = Region{ fittedX, fittedY, width, height };
			return true;
		}

		void SetRegion(const Region& region, const uint8_t* const dat, size_t stride)
		{
			auto charsize = sizeof(char);

			for (auto i = 0; i < region.H; ++i)
			{
				memcpy(data.data() + ((region.Y + i) * widthOfTexture + region.X) * charsize,
					   dat + (i * stride) * charsize,
					   region.W * charsize);
			}
		}

		NormalizedRegion Normalize(const Region& region) const
		{
			return NormalizedRegion{
				NormalizeX(region.X),
				NormalizeY(region.Y),
				NormalizeX(region.X + region.W),
				NormalizeY(region.Y + region.H) };
		}

		float_t NormalizeX(int32_t x) const
		{
			return static_cast<float_t>(x) / static_cast<float_t>(widthOfTexture);
		}

		float_t NormalizeY(int32_t y) const
		{
			return static_cast<float_t>(y) / static_cast<float_t>(heightOfTexture);
		}

	private:

		int32_t Fit(size_t index, size_t width, size_t height)
		{
			auto node = nodes[index];
			auto i = index;
			auto x = node.X;
			auto y = node.Y;
			auto width_left = static_cast<int32_t>(width);

			if (x + width > widthOfTexture - 1)
				return -1;

			while (width_left > 0)
			{
				node = nodes[i];

				if (node.Y > y)
					y = node.Y;

				if (y + height > heightOfTexture - 1)
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
					nodes.erase(nodes.cbegin() + i + 1);
					--i;
				}
			}
		}
	};

	class Atlas
	{
		friend AtlasLoader;

		GLuint nameOfTexture;

		std::vector<AtlasFont> fonts;

		void emplace_back(AtlasLoaderFont&& font);

	public:

		//
		// Gets the AtlasFont with the specified name.
		const AtlasFont& Get(GLuint name) const
		{
			return fonts.at(name);
		}

		//
		// Gets the name of the Atlas texture.
		GLuint Name() const
		{
			return nameOfTexture;
		}

		//
		// Copying is not allowed.
		Atlas(const Atlas&) = delete;

		//
		// Create a new Atlas from another Atlas.
		Atlas(Atlas&& other) :
			nameOfTexture(other.nameOfTexture)
		{
			other.nameOfTexture = 0;
		}

		//
		// Create a new Atlas.
		explicit Atlas(GLuint nameOfTexture) :
			nameOfTexture(nameOfTexture)
		{
		}

		//
		// Destroy the Atlas.
		~Atlas()
		{
			if (nameOfTexture != 0)
			{
				gl::DeleteTextures(1, &nameOfTexture);
				nameOfTexture = 0;
			}
		}

		//
		// glBindTexture
		void Bind() const
		{
			gl::BindTexture(gl::TEXTURE_2D, nameOfTexture);
		}
	};
}
