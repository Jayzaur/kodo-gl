#pragma once

#include "Windows.hpp"

namespace kodogl
{
	class WindowContext
	{
		friend class Window;

		glm::vec4 area;

		GLubyte currentLayer = 0;

		VertexBuffer<Vertex2f1f>& dynamicColoredGeometry;
		VertexBuffer<Vertex2f2f1f>& texturedGeometry;
		std::vector<DrawingReference>& cmdVector;

	public:

		bool Modified;

		WindowContext( Window* window );

		const glm::vec4& Area();
		void Area( const glm::vec4& areaToSet );

		glm::vec4 Transform( const glm::vec4& quad ) const;

		void Reset();

		void PushLayer();
		void PopLayer();

		void DrawQuads( glm::vec4* quads, int quadsLength, const Brush* brush );
		void DrawQuad( const glm::vec4& quad, const Brush* brush );
	};
}
