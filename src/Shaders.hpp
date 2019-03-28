#pragma once

#ifndef GLSL
#define GLSL(src) "#version 400 core\n" #src
#endif

//
// Frame buffer vertex shader.
//
static const char* frameBufferVertexShaderSource = GLSL(
	// Input vertices and texture coordinates from vertex attributes.
	layout( location = 0 ) in vec2 vertexXY;
	layout( location = 1 ) in vec2 vertexST;
	// Output texture coordinate to fragment shader.
	out vec2 fragmentST;

	void main()
	{
		// Output the vertex position and texture coordinate.
		gl_Position = vec4( vertexXY, 0.0, 1.0 );
		fragmentST = vertexST;
	}
);
//
// Frame buffer fragment shader.
//
static const char* frameBufferFragmentShaderSource = GLSL(
	// Input texture coordinate.	
	in vec2 fragmentST;
	// Uniform texture to be sampled.
	uniform sampler2D FrameBufferTexture;
	// Output fragment color.
	out vec4 fragmentColor;

	void main()
	{
		// Sample the frame buffer texture to get the color.
		fragmentColor = texture( FrameBufferTexture, fragmentST );
	}
);

//
// Basic geometry vertex shader.
//
static const char* basicGeometryVertexShaderSource = GLSL(
	// Vertex attributes.
	layout( location = 0 ) in vec2 inputXY;
	layout( location = 1 ) in float inputWeight;
	// Uniform transformation matrices.
	//uniform mat4 Model;
	//uniform mat4 View;
	uniform mat4 Projection;

	// Output color for the basic fragment shader.
	out float fragmentWeight;

	void main()
	{
		fragmentWeight = inputWeight;
		//globalColor = inputRGBA;

		gl_Position = Projection * vec4( inputXY, 0.0, 1.0 );
		//gl_Position = Projection * (View * (Model * vec4( inputXY, 0.0, 1.0 )));
	}
);
//
// Basic geometry fragment shader.
//
static const char* basicGeometryFragmentShaderSource = GLSL(
	// Input color from the basic vertex shader.
	in float fragmentWeight;

	uniform vec4 ColorA;
	uniform vec4 ColorB;
	uniform float Opacity;

	// Output color.
	out vec4 outColor;

	void main()
	{
		vec4 mixedColor = mix( ColorA, ColorB, fragmentWeight );
		outColor = vec4( mixedColor.rgb, mixedColor.a * Opacity );
	}
);

//
// Basic geometry vertex shader.
//
static const char* textureMaskGeometryVertexShaderSource = GLSL(
	// Vertex attributes.
	layout( location = 0 ) in vec2 inputXY;
	layout( location = 1 ) in vec2 inputST;
	layout( location = 2 ) in float inputWeight;

	// Uniform transformation matrices.
	uniform mat4 Projection;

	// Output color for the fragment shader.
	out vec2 fragmentST;
	out float fragmentWeight;

	void main()
	{
		fragmentST = inputST;
		fragmentWeight = inputWeight;

		gl_Position = Projection * vec4( inputXY, 0.0, 1.0 );
	}
);
//
// Basic geometry fragment shader.
//
static const char* textureMaskGeometryFragmentShaderSource = GLSL(
	in vec2 fragmentST;
	in float fragmentWeight;

	uniform sampler2D Texture;
	uniform vec4 ColorA;
	uniform vec4 ColorB;
	uniform float Opacity;

	out vec4 outColor;

	void main()
	{
		float textureOpacity = texture( Texture, fragmentST ).r;
		vec4 mixedColor = mix( ColorA, ColorB, fragmentWeight );
		outColor = vec4( mixedColor.rgb, mixedColor.a * Opacity * textureOpacity );
	}
);