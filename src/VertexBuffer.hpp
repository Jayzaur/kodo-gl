#pragma once

#include "kodo-gl.hpp"

namespace kodogl
{
	class VertexBufferException : public exception
	{
	public:
		explicit VertexBufferException( std::string message ) : exception( message ) {}
	};

	struct VertexAttribute
	{
		const GLuint Index;
		const GLint Components;
		const GLenum Type;
		const GLboolean Normalized;
		const GLvoid* Pointer;

		VertexAttribute( GLuint index, GLint components, GLenum type, GLboolean normalized, const GLvoid* pointer ) :
			Index( index ), Components( components ), Type( type ), Normalized( normalized ), Pointer( pointer )
		{
		}
	};

	template<typename TVertex>
	class VertexBuffer
	{
	public:

		static constexpr auto SizeOfVertex = sizeof( TVertex );
		static constexpr auto SizeOfIndex = sizeof( GLuint );
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

			VertexBufferItem() : StartOfIndices( 0 ), CountOfIndices( 0 ), StartOfVertices( 0 ), CountOfVertices( 0 ) {}
			VertexBufferItem( uint32_t si, uint32_t ci, uint32_t sv, uint32_t cv ) :
				StartOfIndices( si ), CountOfIndices( ci ),
				StartOfVertices( sv ), CountOfVertices( cv )
			{
			}
		};

		// Vector of vertices.
		std::vector<TVertex> vertices;
		// Vector of indices.
		std::vector<GLuint> indices;
		// Vector of items.
		std::map<GLint, VertexBufferItem> items;

		// GL identity of the Vertex Array Object.
		GLuint idOfVAO;
		// GL identity of the vertices buffer.
		GLuint idOfVertices;
		// GL identity of the indices buffer.
		GLuint idOfIndices;

		// Current size of the vertices buffer in the GPU.
		size_t sizeofGPUVertices;
		// Current size of the indices buffer in the GPU.
		size_t sizeofGPUIndices;

		// State of the vertex buffer.
		VertexBufferState state;

		size_t idCounter;

	public:

		GLuint Name() const
		{
			return idOfVAO;
		}

		const VertexBufferItem& At( GLuint i ) const
		{
			return items.at( i );
		}

		TVertex& VertexAt( size_t i )
		{
			state = VertexBufferState::Dirty;
			return vertices[i];
		}

		size_t CountOfVertices() const
		{
			return vertices.size();
		}

		auto begin()
		{
			return vertices.begin();
		}

		auto end()
		{
			return vertices.end();
		}

		/*typename std::vector<TVertex>::iterator BeginOf( GLint id )
		{
			auto& item = items.at( id );
			return vertices.begin() + item.StartOfVertices;
		}

		decltype(std::vector<TVertex>::iterator) EndOf( GLint id )
		{
			auto& item = items.at( id );
			return vertices.begin() + item.StartOfVertices + item.CountOfVertices;
		}*/

		size_t size() const
		{
			return items.size();
		}

		VertexBuffer( const VertexBuffer& ) = delete;
		VertexBuffer( VertexBuffer&& other ) :
			vertices( std::move( other.vertices ) ),
			indices( std::move( other.indices ) ),
			items( std::move( other.items ) ),
			idOfVAO( other.idOfVAO ), idOfVertices( other.idOfVertices ), idOfIndices( other.idOfIndices ),
			sizeofGPUVertices( other.sizeofGPUVertices ), sizeofGPUIndices( other.sizeofGPUIndices ),
			state( other.state ),
			idCounter( other.idCounter )
		{
			other.idOfVAO = 0;
			other.idOfIndices = 0;
			other.idOfVertices = 0;
		}

		explicit VertexBuffer() :
			idOfVAO( 0 ), idOfVertices( 0 ), idOfIndices( 0 ),
			sizeofGPUVertices( 0 ), sizeofGPUIndices( 0 ),
			state( VertexBufferState::Dirty ),
			idCounter( 0 )
		{
			gl::GenBuffers( 1, &idOfVertices );
			gl::GenBuffers( 1, &idOfIndices );
			gl::GenVertexArrays( 1, &idOfVAO );

			gl::BindVertexArray( idOfVAO );
			gl::BindBuffer( gl::ARRAY_BUFFER, idOfVertices );

			for (const auto& a : TVertex::Attributes())
			{
				gl::EnableVertexAttribArray( a.Index );
				gl::VertexAttribPointer( a.Index, a.Components, a.Type, a.Normalized, SizeOfVertex, a.Pointer );
			}

			gl::BindBuffer( gl::ARRAY_BUFFER, 0 );
			gl::BindBuffer( gl::ELEMENT_ARRAY_BUFFER, idOfIndices );
		}

		~VertexBuffer()
		{
			state = VertexBufferState::Frozen;

			items.clear();
			indices.clear();
			vertices.clear();

			if (idOfVAO != 0) {
				gl::DeleteVertexArrays( 1, &idOfVAO );
				idOfVAO = 0;
			}
			if (idOfVertices != 0) {
				gl::DeleteBuffers( 1, &idOfVertices );
				idOfVertices = 0;
			}
			if (idOfIndices != 0) {
				gl::DeleteBuffers( 1, &idOfIndices );
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

			gl::BindBuffer( gl::ARRAY_BUFFER, idOfVertices );

			if (sizeofVertices == sizeofGPUVertices)
			{
				gl::BufferSubData( gl::ARRAY_BUFFER, 0, sizeofVertices, vertices.data() );
			}
			else
			{
				gl::BufferData( gl::ARRAY_BUFFER, sizeofVertices, vertices.data(), gl::DYNAMIC_DRAW );
				sizeofGPUVertices = sizeofVertices;
			}

			gl::BindBuffer( gl::ARRAY_BUFFER, 0 );

			//
			// Upload indices
			//

			auto sizeofIndices = indices.size() * SizeOfIndex;

			gl::BindBuffer( gl::ELEMENT_ARRAY_BUFFER, idOfIndices );

			if (sizeofIndices == sizeofGPUIndices)
			{
				gl::BufferSubData( gl::ELEMENT_ARRAY_BUFFER, 0, sizeofIndices, indices.data() );
			}
			else
			{
				gl::BufferData( gl::ELEMENT_ARRAY_BUFFER, sizeofIndices, indices.data(), gl::DYNAMIC_DRAW );
				sizeofGPUIndices = sizeofIndices;
			}

			gl::BindBuffer( gl::ELEMENT_ARRAY_BUFFER, 0 );
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
		}

		void Render( GLenum mode = gl::TRIANGLES )
		{
			Bind();
			gl::DrawElements( mode, indices.size(), gl::UNSIGNED_INT, nullptr );
			Unbind();
		}

	private:

		void Bind()
		{
			// Unbind so no existing VAO-state is overwritten, (e.g. the GL_ELEMENT_ARRAY_BUFFER-binding).
			Unbind();

			if (state != VertexBufferState::Clean)
			{
				Upload();
				state = VertexBufferState::Clean;
			}

			// Bind VAO for drawing.
			gl::BindVertexArray( idOfVAO );
		}

		static void Unbind()
		{
			gl::BindVertexArray( 0 );
		}

		void RenderItem( GLint id, GLenum mode )
		{
			const auto& item = items[id];
			gl::DrawElements( mode, item.CountOfIndices, gl::UNSIGNED_INT, reinterpret_cast<void*>(item.StartOfIndices * sizeof( GLuint )) );
		}

		template<typename TCollection>
		void InsertIndices( size_t index, const TCollection& iRange )
		{
			std::copy( iRange.begin(), iRange.end(), std::inserter( indices, indices.begin() + index ) );
		}

		template<typename TCollection>
		void InsertVertices( size_t index, const TCollection& vRange )
		{
			for (auto& i : indices) {
				if (i > index) {
					i += index;
				}
			}

			std::copy( vRange.begin(), vRange.end(), std::inserter( vertices, vertices.begin() + index ) );
		}

		void EraseIndices( size_t first, size_t last )
		{
			indices.erase( indices.cbegin() + first, indices.cbegin() + last );
		}

		void EraseVertices( size_t first, size_t last )
		{
			for (auto& i : indices) {
				if (i > first) {
					i -= last - first;
				}
			}

			vertices.erase( vertices.cbegin() + first, vertices.cbegin() + last );
		}

	public:

		template<typename TVertices>
		GLint PushQuads( const TVertices& vRange )
		{
			state = VertexBufferState::Dirty;

			auto startOfVertices = vertices.size();
			auto countOfVertices = vRange.size();
			auto startOfIndices = indices.size();
			auto countOfIndices = (countOfVertices / 4) * 6;

			for (auto i : Range( 0, static_cast<int>(countOfVertices), 4 ))
			{
				auto vBeganAt = vertices.size();

				for (auto iofV : Range( 4 ))
					vertices.push_back( vRange[i + iofV] );

				for (auto iofI : Range( 6 ))
					indices.push_back( IndicesOfQuad[iofI] + vBeganAt );
			}

			idCounter++;
			items.emplace( idCounter, VertexBufferItem{ startOfIndices, countOfIndices, startOfVertices, countOfVertices } );
			return idCounter;
		}

		template<typename TVertices>
		GLint PushQuad( const TVertices& vRange )
		{
			state = VertexBufferState::Dirty;

			auto startOfVertices = vertices.size();
			auto startOfIndices = indices.size();

			for (const auto& v : vRange)
				vertices.push_back( v );

			for (const auto& i : IndicesOfQuad)
				indices.push_back( i + startOfVertices );

			idCounter++;
			items.emplace( idCounter, VertexBufferItem{ startOfIndices, 6, startOfVertices, 4 } );
			return idCounter;
		}

		//
		// Push vertices and indices to the back of the buffer.
		//
		template<typename TVertices, typename TIndices>
		GLint Push( const TVertices& vRange, const TIndices& iRange )
		{
			state = VertexBufferState::Dirty;

			auto startOfVertices = vertices.size();
			auto startOfIndices = indices.size();

			for (const auto& v : vRange)
				vertices.push_back( v );

			for (const auto& i : iRange)
				indices.push_back( i + startOfVertices );

			idCounter++;
			items.emplace( idCounter, VertexBufferItem{ startOfIndices, iRange.size(), startOfVertices, vRange.size() } );
			return idCounter;
		}

		void Erase( GLint id )
		{
			if (!items.count( id ))
				return;

			state = VertexBufferState::Dirty;

			auto& item = items[id];
			auto vstart = item.StartOfVertices;
			auto vcount = item.CountOfVertices;
			auto istart = item.StartOfIndices;
			auto icount = item.CountOfIndices;

			// Update items.
			for (size_t i = 0; i < items.size(); ++i)
			{
				auto& it = items[i];

				if (it.StartOfVertices > vstart)
				{
					it.StartOfVertices -= vcount;
					it.StartOfIndices -= icount;
				}
			}

			EraseIndices( istart, istart + icount );
			EraseVertices( vstart, vstart + vcount );

			items.erase( id );
		}
	};
}