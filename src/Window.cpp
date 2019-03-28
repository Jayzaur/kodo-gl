#include "Window.hpp"

#include "kodo-gl.hpp"
#include "Shaders.hpp"
#include "Shader.hpp"
#include "VertexBuffer.hpp"

#include "WindowContext.hpp"

enum class ColoringUniforms
{
	Projection,
	ColorA,
	ColorB,
	Opacity
};

enum class TextureMaskUniforms
{
	Projection,
	Texture,
	ColorA,
	ColorB,
	Opacity
};

namespace kodogl
{
	Window::Window(GLFWwindow* glfwWindow) :
		glfwPointer(glfwWindow)
	{
		basicGeometryBuffer = std::make_unique<VertexBuffer<Vertex2f1f>>();

		//
		// Create the off-screen frame buffer.
		//

		{
			gl::GenFramebuffers(1, &idOfFrameBuffer);
			gl::GenTextures(1, &idOfFrameBufferTexture);

			std::array<Vertex2f2f, 4> vertices{
				Vertex2f2f{ -1, 1,  0,1 },
				Vertex2f2f{ -1,-1,  0,0 },
				Vertex2f2f{ +1,-1,  1,0 },
				Vertex2f2f{ +1,+1,  1,1 }
			};

			frameBufferGeometry = std::make_unique<VertexBuffer<Vertex2f2f>>();
			frameBufferGeometry->PushQuad(vertices);

			std::vector<Shader> shaders;
			shaders.emplace_back(ShaderType::Vertex, frameBufferVertexShaderSource);
			shaders.emplace_back(ShaderType::Fragment, frameBufferFragmentShaderSource);
			std::vector<Uniform> uniforms;
			uniforms.emplace_back(0, "FrameBufferTexture");

			frameBufferProgram = std::make_unique<ShaderProgram>("FrameBufferProgram", shaders, uniforms);
			frameBufferProgram->Use();
			frameBufferProgram->Get(0) = 0;
		}

		{
			std::vector<Shader> shaders;
			shaders.emplace_back(ShaderType::Vertex, textureMaskGeometryVertexShaderSource);
			shaders.emplace_back(ShaderType::Fragment, textureMaskGeometryFragmentShaderSource);
			std::vector<Uniform> uniforms;
			uniforms.emplace_back(TextureMaskUniforms::Texture, "Texture");
			uniforms.emplace_back(TextureMaskUniforms::ColorA, "ColorA");
			uniforms.emplace_back(TextureMaskUniforms::ColorB, "ColorB");
			uniforms.emplace_back(TextureMaskUniforms::Opacity, "Opacity");
			uniforms.emplace_back(TextureMaskUniforms::Projection, "Projection");

			textureMaskGeometryProgram = std::make_unique<ShaderProgram>("textureMaskGeometryProgram", shaders, uniforms);
			textureMaskGeometryProgram->Use();
			textureMaskGeometryProgram->Get(TextureMaskUniforms::Texture) = 0;
			textureMaskGeometryProgram->Get(TextureMaskUniforms::ColorA) = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			textureMaskGeometryProgram->Get(TextureMaskUniforms::ColorB) = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			textureMaskGeometryProgram->Get(TextureMaskUniforms::Opacity) = 1.0f;

			textureGeometryBuffer = std::make_unique<VertexBuffer<Vertex2f2f1f>>();
		}

		{
			std::vector<Shader> shaders;
			shaders.emplace_back(ShaderType::Vertex, basicGeometryVertexShaderSource);
			shaders.emplace_back(ShaderType::Fragment, basicGeometryFragmentShaderSource);
			std::vector<Uniform> uniforms;
			uniforms.emplace_back(ColoringUniforms::ColorA, "ColorA");
			uniforms.emplace_back(ColoringUniforms::ColorB, "ColorB");
			uniforms.emplace_back(ColoringUniforms::Opacity, "Opacity");
			uniforms.emplace_back(ColoringUniforms::Projection, "Projection");

			basicGeometryProgram = std::make_unique<ShaderProgram>("basicGeometryProgram", shaders, uniforms);
			basicGeometryProgram->Use();
			basicGeometryProgram->Get(ColoringUniforms::ColorA) = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			basicGeometryProgram->Get(ColoringUniforms::ColorB) = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			basicGeometryProgram->Get(ColoringUniforms::Opacity) = 1.0f;
		}
	}

	void Window::OnPositionChanged(glm::int32 x, glm::int32 y)
	{
		// Adjust window position.
		area.x = static_cast<glm::float32>(x);
		area.y = static_cast<glm::float32>(y);
	}

	void Window::OnSizeChanged(glm::int32 width, glm::int32 height)
	{
		// Adjust window area.
		area.z = static_cast<glm::float32>(width);
		area.w = static_cast<glm::float32>(height);

		//
		// Adjust the frame buffer.
		//
		gl::Viewport(0, 0, width, height);
		gl::BindTexture(gl::TEXTURE_2D, idOfFrameBufferTexture);
		gl::TexImage2D(gl::TEXTURE_2D, 0, gl::RGBA, width, height, 0, gl::RGBA, gl::UNSIGNED_BYTE, nullptr);
		gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::LINEAR);
		gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, gl::LINEAR);
		gl::BindFramebuffer(gl::FRAMEBUFFER, idOfFrameBuffer);
		gl::FramebufferTexture2D(gl::FRAMEBUFFER, gl::COLOR_ATTACHMENT0, gl::TEXTURE_2D, idOfFrameBufferTexture, 0);

		//
		// Adjust projection matrices.
		//
		projection = glm::ortho(0.0f, static_cast<float_t>(width), static_cast<float_t>(height), 0.0f);

		basicGeometryProgram->Use();
		basicGeometryProgram->Get(ColoringUniforms::Projection).Set(projection);
		textureMaskGeometryProgram->Use();
		textureMaskGeometryProgram->Get(TextureMaskUniforms::Projection).Set(projection);
	}

	void Window::BeginFrame()
	{
		bool fullFrame = false;

		basicGeometryBuffer->Clear();

		commandVector.clear();

		for (const auto& context : drawingContexts)
		{
			context->Reset();
		}

		if (fullFrame)
		{
			for (const auto& context : drawingContexts)
			{

			}
		}
	}

	void Window::EndFrame()
	{
		auto fullFrame = true;

		//
		// Sort the accumulated commands.
		//
		std::sort(commandVector.begin(), commandVector.end());

		//
		// Switch to the off-screen frame buffer.
		//
		gl::BindFramebuffer(gl::FRAMEBUFFER, idOfFrameBuffer);

		//
		// Setup blending mode.
		//
		gl::Enable(gl::BLEND);
		gl::BlendEquation(gl::FUNC_ADD);
		gl::BlendFunc(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);
		gl::Disable(gl::CULL_FACE);
		gl::Disable(gl::DEPTH_TEST);

		if (fullFrame)
		{
			//
			// Clear the whole frame buffer.
			//
			gl::ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			gl::Clear(gl::COLOR_BUFFER_BIT);
		}
		else
		{
			//
			// Clear only the modified parts of the frame buffer.
			//
			gl::ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			gl::Enable(gl::SCISSOR_TEST);

			for (const auto& context : drawingContexts)
			{
				if (context->Modified)
				{
					// Scissor the context area.
					const auto& contextArea = context->Area();
					gl::Scissor(static_cast<GLint>(contextArea.x),
								static_cast<GLint>(area.w - contextArea.w),
								static_cast<GLsizei>(contextArea.z - contextArea.x),
								static_cast<GLsizei>(contextArea.w - contextArea.y));

					// Clear the context area.
					gl::Clear(gl::COLOR_BUFFER_BIT);
				}
			}
		}

		CommandType currentType = CommandType::None;
		WindowContext* currentContext = nullptr;
		GenericVertexBuffer* currentBuffer = nullptr;

		for (const auto& ref : commandVector)
		{
			if (!fullFrame && currentContext != ref.Context)
			{
				currentContext = ref.Context;

				// Scissor the context area.
				const auto& contextArea = currentContext->Area();
				//printf_s( "%f", area.x );
				gl::Scissor(static_cast<GLint>(contextArea.x),
							static_cast<GLint>(area.w - contextArea.w),
							static_cast<GLsizei>(contextArea.z - contextArea.x),
							static_cast<GLsizei>(contextArea.w - contextArea.y));
			}

			if (currentBuffer != ref.Buffer)
			{
				currentBuffer = ref.Buffer;
				currentBuffer->Bind();
			}

			switch (ref.Type)
			{
				case CommandType::Color:
				{
					if (currentType != CommandType::Color)
					{
						currentType = CommandType::Color;
						basicGeometryProgram->Use();
					}

					basicGeometryProgram->Get(ColoringUniforms::ColorA) = glm::unpackUnorm4x8(ref.ColorA);
					basicGeometryProgram->Get(ColoringUniforms::ColorB) = glm::unpackUnorm4x8(ref.ColorB);

					currentBuffer->Render(ref.GeometryRef);
					break;
				}
				case CommandType::Texture:
					break;
				case CommandType::TextureMask:
				{
					if (currentType != CommandType::TextureMask)
					{
						currentType = CommandType::TextureMask;
						textureMaskGeometryProgram->Use();
					}

					gl::ActiveTexture(gl::TEXTURE0);
					gl::BindTexture(gl::TEXTURE_2D, ref.TextureRef);

					textureMaskGeometryProgram->Get(TextureMaskUniforms::ColorA) = glm::unpackUnorm4x8(ref.ColorA);
					textureMaskGeometryProgram->Get(TextureMaskUniforms::ColorB) = glm::unpackUnorm4x8(ref.ColorB);

					currentBuffer->Render(ref.GeometryRef);
					break;
				}
#ifdef _DEBUG
				default:
					//kodoglError( "kodoglFrameEnd: Invalid CommandType!" );
					break;
#endif
			}
		}

		gl::Disable(gl::SCISSOR_TEST);

		//
		// Switch to default frame buffer.
		//
		gl::BindFramebuffer(gl::FRAMEBUFFER, 0);
		
		//
		// Render off-screen buffer to screen.
		//
		gl::ActiveTexture(gl::TEXTURE0);
		gl::BindTexture(gl::TEXTURE_2D, idOfFrameBufferTexture);

		frameBufferProgram->Use();
		frameBufferGeometry->Bind();
		frameBufferGeometry->Render();

		//
		// Swap front and back buffers.
		//
		glfwSwapBuffers(glfwPointer);
	}
}