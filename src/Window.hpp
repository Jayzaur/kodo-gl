#pragma once

#include "Windows.hpp"

namespace kodogl
{
	class WindowContext;

	typedef void( *RefreshCallback )();
	typedef void( *MouseContainedCallback )(bool);
	typedef void( *MouseMoveCallback )(glm::float32, glm::float32);
	typedef void( *PositionChanged )(glm::int32, glm::int32);
	typedef void( *SizeChangedCallback )(glm::int32, glm::int32);

	class Window
	{
		friend class WindowContext;

		GLFWwindow* glfwPointer;

		RefreshCallback refreshCallback;
		MouseContainedCallback mouseContainedCallback;
		MouseMoveCallback mouseMoveCallback;
		PositionChanged positionChangedCallback;
		SizeChangedCallback sizeChangedCallback;

		std::vector<DrawingReference> commandVector;
		std::vector<std::unique_ptr<WindowContext>> drawingContexts;

		std::unique_ptr<ShaderProgram> frameBufferProgram;
		std::unique_ptr<ShaderProgram> basicGeometryProgram;
		std::unique_ptr<ShaderProgram> textureGeometryProgram;
		std::unique_ptr<ShaderProgram> textureMaskGeometryProgram;
		std::unique_ptr<VertexBuffer<Vertex2f2f>> frameBufferGeometry;
		std::unique_ptr<VertexBuffer<Vertex2f1f>> basicGeometryBuffer;
		std::unique_ptr<VertexBuffer<Vertex2f2f1f>> textureGeometryBuffer;

		glm::vec4 area;
		glm::mat4x4 projection;
		glm::uint32 idOfFrameBuffer;
		glm::uint32 idOfFrameBufferTexture;

	public:

		GLFWwindow* GLFWPointer() { return glfwPointer; }

		Window( GLFWwindow* glfwWindow );

		WindowContext* AddContext( std::unique_ptr<WindowContext> context )
		{
			drawingContexts.emplace_back( std::move( context ) );
			return drawingContexts.back().get();
		}

		void OnRefresh() { if (refreshCallback) { refreshCallback(); } }
		void OnMouseContained( bool contained ) { if (mouseContainedCallback) { mouseContainedCallback( contained ); } }
		void OnMouseMove( glm::float32 x, glm::float32 y ) { if (mouseMoveCallback) { mouseMoveCallback( x, y ); } }
		void OnPositionChanged( glm::int32 x, glm::int32 y );
		void OnSizeChanged( glm::int32 width, glm::int32 height );

		void SetRefreshCallback( RefreshCallback callback ) { refreshCallback = callback; }
		void SetMouseContainedCallback( MouseContainedCallback callback ) { mouseContainedCallback = callback; }
		void SetMouseMoveCallback( MouseMoveCallback callback ) { mouseMoveCallback = callback; }
		void SetPositionChangedCallback( PositionChanged callback ) { positionChangedCallback = callback; }
		void SetSizeChangedCallback( SizeChangedCallback callback ) { sizeChangedCallback = callback; }

		void BeginFrame();
		void EndFrame();
	};
}
