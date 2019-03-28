#pragma once

#include "kodo-gl.hpp"

namespace kodogl
{
	struct VertexAttribute
	{
		const GLuint Index;
		const GLint Components;
		const GLenum Type;
		const GLboolean Normalized;
		const GLvoid* Pointer;

		VertexAttribute(GLuint index, GLint components, GLenum type, GLboolean normalized, const GLvoid* pointer) :
			Index(index), Components(components), Type(type), Normalized(normalized), Pointer(pointer)
		{
		}
	};

	struct Vertex2f
	{
		glm::vec2 Vertex;

		Vertex2f() {}
		Vertex2f(float_t x, float_t y) : Vertex2f(glm::vec2(x, y)) {}
		Vertex2f(const glm::vec2& vp) : Vertex(vp) { }

		static const std::array<VertexAttribute, 1>& Attributes()
		{
			static const std::array<VertexAttribute, 1> attributes{
				VertexAttribute{ 0, 2, gl::FLOAT, false, reinterpret_cast<GLvoid*>(offsetof(Vertex2f, Vertex)) }
			};
			return attributes;
		}
	};

	struct Vertex2f1f
	{
		glm::vec2 Vertex;
		float_t Weight;

		Vertex2f1f() {}
		Vertex2f1f(float_t x, float_t y, float_t w) : Vertex2f1f(glm::vec2(x, y), w) {}
		Vertex2f1f(const glm::vec2& vp, float_t w) : Vertex(vp), Weight(w) { }

		static const std::array<VertexAttribute, 2>& Attributes()
		{
			static const std::array<VertexAttribute, 2> attributes{
				VertexAttribute{ 0, 2, gl::FLOAT, false, reinterpret_cast<GLvoid*>(offsetof(Vertex2f1f, Vertex)) },
				VertexAttribute{ 1, 1, gl::FLOAT, false, reinterpret_cast<GLvoid*>(offsetof(Vertex2f1f, Weight)) }
			};
			return attributes;
		}
	};

	struct Vertex2f4f
	{
		glm::vec2 Vertex;
		glm::vec4 Color;

		Vertex2f4f() {}
		Vertex2f4f(float_t x, float_t y, const glm::vec4& c) : Vertex2f4f(glm::vec2(x, y), c) {}
		Vertex2f4f(const glm::vec2& vp, const glm::vec4& c) : Vertex(vp), Color(c) { }

		static const std::array<VertexAttribute, 2>& Attributes()
		{
			static const std::array<VertexAttribute, 2> attributes{
				VertexAttribute{ 0, 2, gl::FLOAT, false, reinterpret_cast<GLvoid*>(offsetof(Vertex2f4f, Vertex)) },
				VertexAttribute{ 1, 4, gl::FLOAT, false, reinterpret_cast<GLvoid*>(offsetof(Vertex2f4f, Color)) }
			};
			return attributes;
		}
	};

	struct Vertex2f2f1f
	{
		glm::vec2 Vertex;
		glm::vec2 Texture;
		glm::float_t Weight;

		Vertex2f2f1f() {}
		Vertex2f2f1f(float_t x, float_t y, float_t s, float_t t, glm::float_t w) : Vertex2f2f1f(glm::vec2(x, y), glm::vec2(s, t), w) {}
		Vertex2f2f1f(const glm::vec2& vp, const glm::vec2& tp, glm::float_t w) : Vertex(vp), Texture(tp), Weight(w) {}

		static const std::array<VertexAttribute, 3>& Attributes()
		{
			static const std::array<VertexAttribute, 3> attributes{
				VertexAttribute{ 0, 2, gl::FLOAT, false, reinterpret_cast<GLvoid*>(offsetof(Vertex2f2f1f, Vertex)) },
				VertexAttribute{ 1, 2, gl::FLOAT, false, reinterpret_cast<GLvoid*>(offsetof(Vertex2f2f1f, Texture)) },
				VertexAttribute{ 2, 1, gl::FLOAT, false, reinterpret_cast<GLvoid*>(offsetof(Vertex2f2f1f, Weight)) }
			};
			return attributes;
		}
	};

	struct Vertex2f2f
	{
		glm::vec2 Vertex;
		glm::vec2 Texture;

		Vertex2f2f() {}
		Vertex2f2f(float_t x, float_t y, float_t s, float_t t) : Vertex2f2f(glm::vec2(x, y), glm::vec2(s, t)) {}
		Vertex2f2f(const glm::vec2& vp, const glm::vec2& tp) : Vertex(vp), Texture(tp) {}

		static const std::array<VertexAttribute, 2>& Attributes()
		{
			static const std::array<VertexAttribute, 2> attributes{
				VertexAttribute{ 0, 2, gl::FLOAT, false, reinterpret_cast<GLvoid*>(offsetof(Vertex2f2f, Vertex)) },
				VertexAttribute{ 1, 2, gl::FLOAT, false, reinterpret_cast<GLvoid*>(offsetof(Vertex2f2f, Texture)) },
			};
			return attributes;
		}
	};

	struct Vertex2f2f4f
	{
		glm::vec2 Vertex;
		glm::vec2 Texture;
		glm::vec4 Color;

		Vertex2f2f4f() {}
		Vertex2f2f4f(float_t x, float_t y, float_t s, float_t t, const glm::vec4& c) : Vertex2f2f4f(glm::vec2(x, y), glm::vec2(s, t), c) {}
		Vertex2f2f4f(const glm::vec2& vp, const glm::vec2& tp, const glm::vec4& c) : Vertex(vp), Texture(tp), Color(c) {}

		static const std::array<VertexAttribute, 3>& Attributes()
		{
			static const std::array<VertexAttribute, 3> attributes{
				VertexAttribute{ 0, 2, gl::FLOAT, false, reinterpret_cast<GLvoid*>(offsetof(Vertex2f2f4f, Vertex)) },
				VertexAttribute{ 1, 2, gl::FLOAT, false, reinterpret_cast<GLvoid*>(offsetof(Vertex2f2f4f, Texture)) },
				VertexAttribute{ 2, 4, gl::FLOAT, false, reinterpret_cast<GLvoid*>(offsetof(Vertex2f2f4f, Color)) }
			};
			return attributes;
		}
	};

	class VertexBufferException : public exception
	{
	public:
		explicit VertexBufferException(std::string message) : exception(message) {}
	};

	template<typename TVertex>
	class GeometryBuffer : public nocopy
	{
	public:

		static constexpr auto SizeOfVertex = sizeof(TVertex);
		static constexpr auto SizeOfIndex = sizeof(GLuint);
		static constexpr std::array<uint8_t, 6> IndicesOfQuad = { 0, 1, 2, 0, 2, 3 };

	private:

		typedef GLuint TKey;

		struct Item
		{
			uint32_t StartOfIndices;
			const uint32_t CountOfIndices;

			uint32_t StartOfVertices;
			const uint32_t CountOfVertices;

			Item() : Item(0, 0, 0, 0) {}
			Item(uint32_t si, uint32_t ci, uint32_t sv, uint32_t cv) :
				StartOfIndices(si), CountOfIndices(ci),
				StartOfVertices(sv), CountOfVertices(cv)
			{
			}
		};

		// Item key 'generator'.
		uint32_t itemsKeyCounter;

		// Map of items.
		std::unordered_map<TKey, Item> items;
	};

	class GenericVertexBuffer
	{
	public:

		virtual ~GenericVertexBuffer() {}

		// glBindVertexArray the vertex buffer. Automatically updates GPU with any modifications.
		virtual void Bind() = 0;
		// glBindVertexArray(0).
		virtual void Unbind() = 0;

		// glDrawElements the whole vertex buffer.
		virtual void Render() = 0;
		// glDrawElements a specific range of vertices.
		virtual void Render(glm::uint32) = 0;
	};

	template<typename TVertex>
	class VertexBuffer : public nocopy, public GenericVertexBuffer
	{
	public:

		static constexpr auto SizeOfVertex = sizeof(TVertex);
		static constexpr auto SizeOfIndex = sizeof(GLuint);
		static constexpr std::array<uint8_t, 6> IndicesOfQuad = { 0,1,2, 0,2,3 };

	private:

		enum class VertexBufferState
		{
			Clean,
			Dirty,
			Frozen
		};

		struct VertexBufferItem
		{
			uint32_t StartOfIndices;
			const uint32_t CountOfIndices;

			uint32_t StartOfVertices;
			const uint32_t CountOfVertices;

			VertexBufferItem() : StartOfIndices(0), CountOfIndices(0), StartOfVertices(0), CountOfVertices(0) {}
			VertexBufferItem(uint32_t si, uint32_t ci, uint32_t sv, uint32_t cv) :
				StartOfIndices(si), CountOfIndices(ci),
				StartOfVertices(sv), CountOfVertices(cv)
			{
			}
		};

		// Vector of vertices.
		std::vector<TVertex> vertices;
		// Vector of indices.
		std::vector<GLuint> indices;
		// Map of items.
		std::unordered_map<GLuint, VertexBufferItem> items;

		// GL identity of the Vertex Array Object.
		GLuint idOfVAO;
		// GL identity of the vertex buffer.
		GLuint idOfVertices;
		// GL identity of the index buffer.
		GLuint idOfIndices;

		// Current size of the vertex buffer in the GPU.
		glm::uint32 sizeofGPUVertices;
		// Current size of the index buffer in the GPU.
		glm::uint32 sizeofGPUIndices;

		// State of the buffer.
		VertexBufferState state;

		// Item key 'generator'.
		glm::uint32 keyCounter;

	public:

		// GL identity of the Vertex Array Object.
		glm::uint32 Name() const
		{
			return idOfVAO;
		}

		glm::uint32 SizeInBytes() const
		{
			return (SizeOfVertex * vertices.size()) + (SizeOfIndex * indices.size());
		}

		const VertexBufferItem& At(glm::uint32 i) const
		{
			return items.at(i);
		}

		TVertex& VertexAt(glm::uint32 i)
		{
			state = VertexBufferState::Dirty;
			return vertices[i];
		}

		const TVertex& VertexAt(glm::uint32 i) const
		{
			return vertices[i];
		}

		glm::uint32 CountOfVertices() const
		{
			return vertices.size();
		}

		VertexBuffer(VertexBuffer&& other) :
			vertices(std::move(other.vertices)),
			indices(std::move(other.indices)),
			items(std::move(other.items)),
			idOfVAO(other.idOfVAO), idOfVertices(other.idOfVertices), idOfIndices(other.idOfIndices),
			sizeofGPUVertices(other.sizeofGPUVertices), sizeofGPUIndices(other.sizeofGPUIndices),
			state(other.state),
			keyCounter(other.keyCounter)
		{
			other.idOfVAO = 0;
			other.idOfIndices = 0;
			other.idOfVertices = 0;
		}

		explicit VertexBuffer() :
			idOfVAO(0), idOfVertices(0), idOfIndices(0),
			sizeofGPUVertices(0), sizeofGPUIndices(0),
			state(VertexBufferState::Dirty),
			keyCounter(0)
		{
			gl::GenBuffers(1, &idOfVertices);
			gl::GenBuffers(1, &idOfIndices);
			gl::GenVertexArrays(1, &idOfVAO);

			gl::BindVertexArray(idOfVAO);
			gl::BindBuffer(gl::ARRAY_BUFFER, idOfVertices);

			for (const auto& a : TVertex::Attributes())
			{
				gl::EnableVertexAttribArray(a.Index);
				gl::VertexAttribPointer(a.Index, a.Components, a.Type, a.Normalized, SizeOfVertex, a.Pointer);
			}

			gl::BindBuffer(gl::ARRAY_BUFFER, 0);
			gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, idOfIndices);
		}

		~VertexBuffer()
		{
			state = VertexBufferState::Frozen;

			items.clear();
			indices.clear();
			vertices.clear();

			if (idOfVAO != 0) {
				gl::DeleteVertexArrays(1, &idOfVAO);
				idOfVAO = 0;
			}
			if (idOfVertices != 0) {
				gl::DeleteBuffers(1, &idOfVertices);
				idOfVertices = 0;
			}
			if (idOfIndices != 0) {
				gl::DeleteBuffers(1, &idOfIndices);
				idOfIndices = 0;
			}
		}

		//
		// Upload the vertex buffer to the GPU.
		//
		void Upload()
		{
			// Do nothing if frozen.
			if (state == VertexBufferState::Frozen)
				return;

			//
			// Upload vertices
			//

			auto sizeofVertices = vertices.size() * SizeOfVertex;

			gl::BindBuffer(gl::ARRAY_BUFFER, idOfVertices);

			if (sizeofVertices == sizeofGPUVertices)
			{
				gl::BufferSubData(gl::ARRAY_BUFFER, 0, sizeofVertices, vertices.data());
			}
			else
			{
				gl::BufferData(gl::ARRAY_BUFFER, sizeofVertices, vertices.data(), gl::DYNAMIC_DRAW);
				sizeofGPUVertices = sizeofVertices;
			}

			gl::BindBuffer(gl::ARRAY_BUFFER, 0);

			//
			// Upload indices
			//

			auto sizeofIndices = indices.size() * SizeOfIndex;

			gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, idOfIndices);

			if (sizeofIndices == sizeofGPUIndices)
			{
				gl::BufferSubData(gl::ELEMENT_ARRAY_BUFFER, 0, sizeofIndices, indices.data());
			}
			else
			{
				gl::BufferData(gl::ELEMENT_ARRAY_BUFFER, sizeofIndices, indices.data(), gl::DYNAMIC_DRAW);
				sizeofGPUIndices = sizeofIndices;
			}

			gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, 0);
		}

		//
		// Clear the vertex buffer.
		//
		void Clear()
		{
			state = VertexBufferState::Frozen;
			items.clear();
			indices.clear();
			vertices.clear();
			keyCounter = 0;
		}

		void Bind() override
		{
			if (state != VertexBufferState::Clean)
			{
				// Unbind so no existing VAO-state is overwritten, (e.g. the GL_ELEMENT_ARRAY_BUFFER-binding).
				Unbind();
				Upload();
				state = VertexBufferState::Clean;
			}

			// Bind VAO for drawing.
			gl::BindVertexArray(idOfVAO);
		}

		void Unbind() override
		{
			gl::BindVertexArray(0);
		}

		void Render() override
		{
			gl::DrawElements(gl::TRIANGLES, indices.size(), gl::UNSIGNED_INT, nullptr);
		}

		void Render(glm::uint32 id) override
		{
			const auto& item = items[id];
			const auto* indexPtr = reinterpret_cast<void*>(item.StartOfIndices * SizeOfIndex);

			gl::DrawElements(gl::TRIANGLES, item.CountOfIndices, gl::UNSIGNED_INT, indexPtr);
		}

	public:

		template<typename TVertices>
		glm::uint32 PushQuads(const TVertices& vRange)
		{
			state = VertexBufferState::Dirty;

			auto startOfVertices = vertices.size();
			auto countOfVertices = vRange.size();
			auto startOfIndices = indices.size();
			auto countOfIndices = (countOfVertices / 4) * 6;

			for (auto i : Range(0, static_cast<int>(countOfVertices), 4))
			{
				auto vBeganAt = vertices.size();

				for (auto iofV : Range(4))
					vertices.push_back(vRange[i + iofV]);

				for (auto iofI : Range(6))
					indices.push_back(IndicesOfQuad[iofI] + vBeganAt);
			}

			keyCounter++;
			items.emplace(keyCounter, VertexBufferItem{ startOfIndices, countOfIndices, startOfVertices, countOfVertices });
			return keyCounter;
		}

		glm::uint32 AllocateQuads(glm::uint32 quadsLength, glm::uint32* vI, glm::uint32* iI)
		{
			state = VertexBufferState::Dirty;

			auto startOfVertices = vertices.size();
			auto countOfVertices = quadsLength * 4;
			auto startOfIndices = indices.size();
			auto countOfIndices = quadsLength * 6;

			vertices.resize(vertices.size() + countOfVertices);
			indices.resize(indices.size() + countOfIndices);

			keyCounter++;
			items.emplace(keyCounter, VertexBufferItem{ startOfIndices, countOfIndices, startOfVertices, countOfVertices });

			*vI = startOfVertices;
			*iI = startOfIndices;

			return keyCounter;
		}

		// Push a quad to the specified pre-allocated position.
		template<typename TVertices>
		void PushQuadTo(glm::uint32 vI, glm::uint32 iI, glm::uint32 num, const TVertices& quad)
		{
			auto nV = num * 4;
			auto nI = num * 6;

			vertices[vI + 0 + nV] = quad[0];
			vertices[vI + 1 + nV] = quad[1];
			vertices[vI + 2 + nV] = quad[2];
			vertices[vI + 3 + nV] = quad[3];

			indices[iI + 0 + nI] = vI + nV + IndicesOfQuad[0];
			indices[iI + 1 + nI] = vI + nV + IndicesOfQuad[1];
			indices[iI + 2 + nI] = vI + nV + IndicesOfQuad[2];
			indices[iI + 3 + nI] = vI + nV + IndicesOfQuad[3];
			indices[iI + 4 + nI] = vI + nV + IndicesOfQuad[4];
			indices[iI + 5 + nI] = vI + nV + IndicesOfQuad[5];
		}

		template<typename TVertices>
		void PushQuad(glm::uint32 key, glm::uint32 num, const TVertices& quad)
		{
			const auto& item = items[key];

			for (auto i = 0; i < 4; i++)
				vertices[item.StartOfVertices + i + (num * 4)] = quad[i];

			for (auto i = 0; i < 6; i++)
				indices[item.StartOfIndices + i + (num * 6)] = item.StartOfVertices + (num * 4) + IndicesOfQuad[i];
		}

		template<typename TVertices>
		glm::uint32 PushQuad(const TVertices& vRange)
		{
			state = VertexBufferState::Dirty;

			auto startOfVertices = vertices.size();
			auto startOfIndices = indices.size();

			for (const auto& v : vRange)
				vertices.push_back(v);

			for (const auto& i : IndicesOfQuad)
				indices.push_back(i + startOfVertices);

			keyCounter++;
			items.emplace(keyCounter, VertexBufferItem{ startOfIndices, 6, startOfVertices, 4 });
			return keyCounter;
		}

		//
		// Push vertices and indices to the back of the buffer.
		//
		template<typename TVertices, typename TIndices>
		glm::uint32 Push(const TVertices& vRange, const TIndices& iRange)
		{
			state = VertexBufferState::Dirty;

			auto startOfVertices = vertices.size();
			auto startOfIndices = indices.size();

			for (const auto& v : vRange)
				vertices.push_back(v);

			for (const auto& i : iRange)
				indices.push_back(i + startOfVertices);

			keyCounter++;
			items.emplace(keyCounter, VertexBufferItem{ startOfIndices, iRange.size(), startOfVertices, vRange.size() });
			return keyCounter;
		}
	};
}