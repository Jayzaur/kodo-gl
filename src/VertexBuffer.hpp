#pragma once

#include "kodo-gl.hpp"

namespace kodogl
{
	class VertexBufferException : public std::exception
	{
		std::string message;
	public:
		explicit VertexBufferException( std::string message ) : std::exception( message.c_str() ), message( message ) {}
	};

	//
	// Generic vertex attribute.
	//
	struct VertexAttribute
	{
		//
		// Attribute name.
		//
		const GLchar* Name;
		//
		// Index of the generic vertex attribute.
		//
		GLuint Index;
		//
		// Number of components per generic vertex attribute. (Must be 1, 2, 3, or 4)
		//
		GLint Components;
		//
		// Data type of each component in the attribute.
		//
		GLenum Type;
		//
		// Whether fixed-point data values should be normalized (GL_TRUE) or converted directly as fixed-point values (GL_FALSE) when they are accessed.
		//
		GLboolean normalized;
		//
		// Pointer to the first component of the first attribute element in the array.
		//
		GLvoid* Pointer;

		explicit VertexAttribute( const char* name, GLint size = 4, GLenum type = gl::FLOAT, GLboolean normalized = false ) :
			Name( name ), Index( -1 ), Components( size ), Type( type ), normalized( normalized ), Pointer( nullptr )
		{
		}

		static VertexAttribute Create4f( const char* name, bool normalized = false )
		{
			return VertexAttribute( name, 4, gl::FLOAT, normalized );
		}

		static VertexAttribute Create3f( const char* name, bool normalized = false )
		{
			return VertexAttribute( name, 3, gl::FLOAT, normalized );
		}

		static VertexAttribute Create2f( const char* name, bool normalized = false )
		{
			return VertexAttribute( name, 2, gl::FLOAT, normalized );
		}
	};

	template<typename TVertex>
	class VertexBuffer
	{
		enum class VertexBufferState
		{
			Clean,
			Dirty,
			Frozen
		};

		struct VertexBufferItem
		{
			uint32_t StartOfIndices;
			uint32_t CountOfIndices;

			uint32_t StartOfVertices;
			uint32_t CountOfVertices;
		};

		// Vector of attributes.
		std::vector<VertexAttribute> attributes;
		// Vector of vertices.
		std::vector<TVertex> vertices;
		// Vector of indices.
		std::vector<GLuint> indices;
		// Vector of items.
		std::vector<VertexBufferItem> items;

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

		// GL primitives to render.
		GLenum mode;

		// Attribute stride.
		size_t attributeStride;

		// State of the vertex buffer.
		VertexBufferState state;

	public:

		size_t size() const
		{
			return items.size();
		}

		explicit VertexBuffer( const std::vector<VertexAttribute>& attrs ) :
			idOfVAO( 0 ), idOfVertices( 0 ), idOfIndices( 0 ),
			sizeofGPUVertices( 0 ), sizeofGPUIndices( 0 ),
			mode( 0 ),
			state( VertexBufferState::Dirty )
		{
			size_t stride = 0;
			GLchar *pointer = nullptr;

			for (const auto& attribute : attrs)
			{
				attributes.push_back( attribute );
				attributes.at( attributes.size() - 1 ).Pointer = pointer;

				size_t sizeofAttribute;

				switch (attribute.Type)
				{
					case gl::BOOL:           sizeofAttribute = sizeof( GLboolean ); break;
					case gl::BYTE:           sizeofAttribute = sizeof( GLbyte ); break;
					case gl::UNSIGNED_BYTE:  sizeofAttribute = sizeof( GLubyte ); break;
					case gl::SHORT:          sizeofAttribute = sizeof( GLshort ); break;
					case gl::UNSIGNED_SHORT: sizeofAttribute = sizeof( GLushort ); break;
					case gl::INT:            sizeofAttribute = sizeof( GLint ); break;
					case gl::UNSIGNED_INT:   sizeofAttribute = sizeof( GLuint ); break;
					case gl::FLOAT:          sizeofAttribute = sizeof( GLfloat ); break;
					default:                 throw VertexBufferException( "Unknown attribute type." );
				}

				stride += attribute.Components * sizeofAttribute;
				pointer += attribute.Components * sizeofAttribute;
			}

			attributeStride = stride;
		}

		~VertexBuffer()
		{
			state = VertexBufferState::Frozen;

			items.clear();
			indices.clear();
			vertices.clear();
			attributes.clear();

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

			if (idOfVertices == 0)
				gl::GenBuffers( 1, &idOfVertices );
			if (idOfIndices == 0)
				gl::GenBuffers( 1, &idOfIndices );

			//
			// Upload vertices
			// Always upload vertices first such that indices do not point to non existing data.
			//
			{
				auto sizeofVertices = vertices.size() * sizeof( TVertex );

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
			}

			//
			// Upload indices
			//
			{
				auto sizeofIndices = indices.size() * sizeof( GLuint );

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

	private:

		void EnableAttribute( VertexAttribute& attribute ) const
		{
			if (attribute.Index == -1)
			{
				GLint program;
				gl::GetIntegerv( gl::CURRENT_PROGRAM, &program );

				if (program == 0)
					throw VertexBufferException( "No program currently active." );

				attribute.Index = gl::GetAttribLocation( program, attribute.Name );

				if (attribute.Index == -1)
					throw VertexBufferException( "Couldn't find the attribute." );
			}

			gl::EnableVertexAttribArray( attribute.Index );
			gl::VertexAttribPointer( attribute.Index, attribute.Components, attribute.Type, attribute.normalized, attributeStride, attribute.Pointer );
		}

		void RenderSetup( GLenum mode )
		{
			// Unbind so no existing VAO-state is overwritten, (e.g. the GL_ELEMENT_ARRAY_BUFFER-binding).
			RenderFinish();

			if (state != VertexBufferState::Clean)
			{
				Upload();
				state = VertexBufferState::Clean;
			}

			if (idOfVAO == 0)
			{
				// Generate and set up VAO.
				gl::GenVertexArrays( 1, &idOfVAO );
				gl::BindVertexArray( idOfVAO );

				{
					gl::BindBuffer( gl::ARRAY_BUFFER, idOfVertices );

					for (auto& attribute : attributes)
					{
						EnableAttribute( attribute );
					}

					gl::BindBuffer( gl::ARRAY_BUFFER, 0 );
				}

				if (indices.size() != 0)
				{
					gl::BindBuffer( gl::ELEMENT_ARRAY_BUFFER, idOfIndices );
				}
			}

			// Bind VAO for drawing.
			gl::BindVertexArray( idOfVAO );

			this->mode = mode;
		}

		static void RenderFinish()
		{
			gl::BindVertexArray( 0 );
		}

		void RenderItem( size_t index )
		{
			const auto& item = items[index];

			if (indices.size() != 0)
			{
				auto start = item.z;
				auto count = item.w;
				gl::DrawElements( mode, count, gl::UNSIGNED_INT, reinterpret_cast<void*>(start * sizeof( GLuint )) );
			}
			else if (vertices.size() != 0)
			{
				auto start = item.x;
				auto count = item.y;
				gl::DrawArrays( mode, start * sizeof( TVertex ), count );
			}
		}

	public:

		void Render( GLenum mode = gl::TRIANGLES )
		{
			auto icount = indices.size();
			auto vcount = vertices.size();

			RenderSetup( mode );

			if (icount)
			{
				gl::DrawElements( mode, icount, gl::UNSIGNED_INT, nullptr );
			}
			else
			{
				gl::DrawArrays( mode, 0, vcount );
			}

			RenderFinish();
		}

		void Push( const std::vector<GLuint>& range )
		{
			state = VertexBufferState::Dirty;
			std::copy( range.begin(), range.end(), std::back_inserter( indices ) );
		}

		void Push( const std::vector<TVertex>& range )
		{
			state = VertexBufferState::Dirty;
			std::copy( range.begin(), range.end(), std::back_inserter( vertices ) );
		}

		void Insert( size_t index, const std::vector<GLuint>& range )
		{
			state = VertexBufferState::Dirty;
			std::copy( range.begin(), range.end(), std::inserter( indices, indices.begin() + index ) );
		}

		void Insert( size_t index, const std::vector<TVertex>& range )
		{
			state = VertexBufferState::Dirty;

			for (auto& i : indices) {
				if (i > index) {
					i += index;
				}
			}

			std::copy( range.begin(), range.end(), std::inserter( vertices, vertices.begin() + index ) );
		}

		void EraseIndices( size_t first, size_t last )
		{
			state = VertexBufferState::Dirty;
			indices.erase( indices.cbegin() + first, indices.cbegin() + last );
		}

		void EraseVertices( size_t first, size_t last )
		{
			state = VertexBufferState::Dirty;

			for (auto& i : indices) {
				if (i > first) {
					i -= last - first;
				}
			}

			vertices.erase( vertices.cbegin() + first, vertices.cbegin() + last );
		}

		//
		// Push vertices and indices to the back of the buffer.
		//
		size_t Push( const std::vector<TVertex>& vertices, const std::vector<GLuint>& indices )
		{
			return Insert( items.size(), vertices, indices );
		}

		size_t Insert( size_t index, const std::vector<TVertex>& vertices, const std::vector<GLuint>& indices )
		{
			auto startOfVertices = this->vertices.size();
			auto startOfIndices = this->indices.size();

			Push( vertices );
			Push( indices );

			// Update indices within the vertex buffer.
			for (size_t i = 0; i < indices.size(); ++i)
			{
				this->indices[startOfIndices + i] += startOfVertices;
			}

			// Insert item.
			VertexBufferItem item{ 0 };
			item.StartOfVertices = startOfVertices;
			item.CountOfVertices = vertices.size();
			item.StartOfIndices = startOfIndices;
			item.CountOfIndices = indices.size();
			items.insert( items.cbegin() + index, item );
			state = VertexBufferState::Dirty;
			return index;
		}

		void Erase( size_t index )
		{
			auto& item = items[index];
			auto vstart = item.vstart;
			auto vcount = item.vcount;
			auto istart = item.istart;
			auto icount = item.icount;

			// Update items.
			for (size_t i = 0; i < items.size(); ++i)
			{
				item = items[i];

				if (item.vstart > vstart)
				{
					item.vstart -= vcount;
					item.istart -= icount;
				}
			}

			EraseIndices( istart, istart + icount );
			EraseVertices( vstart, vstart + vcount );
			items.erase( items.begin() + index );
			state = VertexBufferState::Dirty;
		}
	};
}