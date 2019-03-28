#version 400

layout ( location = 0 ) in vec2 inputXY;
layout ( location = 1 ) in vec2 inputST;
layout ( location = 2 ) in vec4 inputRGBA;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

out vec2 globalTextureCoordinate;
out vec4 globalColor;

void main()
{
    globalTextureCoordinate = inputST;
	globalColor = inputRGBA;

    gl_Position = Projection * (View * (Model * vec4( inputXY, 0.0, 1.0 )));
}
