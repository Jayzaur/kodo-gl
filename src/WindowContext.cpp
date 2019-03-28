#include "WindowContext.hpp"

#include "kodo-gl.hpp"
#include "VertexBuffer.hpp"

#include "Window.hpp"

namespace kodogl
{
	WindowContext::WindowContext(Window* window) :
		Modified(false),
		area(glm::vec4(1, 1, 11, 11)),
		dynamicColoredGeometry(*window->basicGeometryBuffer),
		texturedGeometry(*window->textureGeometryBuffer),
		cmdVector(window->commandVector)
	{

	}

	const glm::vec4& WindowContext::Area()
	{
		return area;
	}

	void WindowContext::Area(const glm::vec4& areaToSet)
	{
		Modified = true;
		area = areaToSet;
	}

	glm::vec4 WindowContext::Transform(const glm::vec4& quad) const
	{
		return glm::vec4(area.x + quad.x, area.y + quad.y, area.x + quad.z, area.y + quad.w);
	}

	void WindowContext::Reset()
	{
		Modified = false;
		currentLayer = 0;
	}

	void WindowContext::PushLayer()
	{
		currentLayer += 1;
	}

	void WindowContext::PopLayer()
	{
		if (currentLayer > 0)
		{
			currentLayer -= 1;
		}
	}

	void WindowContext::DrawQuads(glm::vec4* quads, int quadsLength, const Brush* brush)
	{
		Modified = true;

		switch (brush->Type)
		{
			case BrushType::Linear:
			{
				static std::array<Vertex2f1f, 4> vertices;

				const auto* colorBrush = reinterpret_cast<const ColorBrush*>(brush);

				glm::uint32 vI;
				glm::uint32 iI;
				glm::uint32 quadsId = dynamicColoredGeometry.AllocateQuads(quadsLength, &vI, &iI);

				for (auto i = 0; i < quadsLength; i++)
				{
					auto transformedQuad = Transform(quads[i]);

					vertices[0] = Vertex2f1f{ transformedQuad.x, transformedQuad.y, colorBrush->Weights.x };
					vertices[1] = Vertex2f1f{ transformedQuad.x, transformedQuad.w, colorBrush->Weights.y };
					vertices[2] = Vertex2f1f{ transformedQuad.z, transformedQuad.w, colorBrush->Weights.z };
					vertices[3] = Vertex2f1f{ transformedQuad.z, transformedQuad.y, colorBrush->Weights.w };

					dynamicColoredGeometry.PushQuadTo(vI, iI, i, vertices);
				}

				DrawingReference ref;
				ref.Layer = currentLayer;
				ref.GeometryRef = quadsId;
				ref.TextureRef = 0;
				ref.Type = CommandType::Color;
				ref.ColorA = colorBrush->ColorA;
				ref.ColorB = colorBrush->ColorB;
				ref.Context = this;
				ref.Buffer = &dynamicColoredGeometry;
				cmdVector.emplace_back(ref);
				break;
			}
			case BrushType::Texture:
				break;
			case BrushType::TextureMask:
				break;
		}
	}

	void WindowContext::DrawQuad(const glm::vec4& quad, const Brush* brush)
	{
		Modified = true;

		switch (brush->Type)
		{
			case BrushType::Linear:
			{
				const auto* colorBrush = reinterpret_cast<const ColorBrush*>(brush);

				auto transformedQuad = Transform(quad);

				static std::array<Vertex2f1f, 4> vertices;
				vertices[0] = Vertex2f1f{ transformedQuad.x, transformedQuad.y, colorBrush->Weights.x };
				vertices[1] = Vertex2f1f{ transformedQuad.x, transformedQuad.w, colorBrush->Weights.y };
				vertices[2] = Vertex2f1f{ transformedQuad.z, transformedQuad.w, colorBrush->Weights.z };
				vertices[3] = Vertex2f1f{ transformedQuad.z, transformedQuad.y, colorBrush->Weights.w };

				auto quadId = dynamicColoredGeometry.PushQuad(vertices);

				DrawingReference ref;
				ref.Layer = currentLayer;
				ref.GeometryRef = quadId;
				ref.TextureRef = 0;
				ref.Type = CommandType::Color;
				ref.ColorA = colorBrush->ColorA;
				ref.ColorB = colorBrush->ColorB;
				ref.Context = this;
				ref.Buffer = &dynamicColoredGeometry;
				cmdVector.emplace_back(ref);
				break;
			}
			case BrushType::Texture:
				break;
			case BrushType::TextureMask:
				break;
		}
	}
}