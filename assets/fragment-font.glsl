#version 400

in vec2 globalTextureCoordinate;
in vec4 globalColor;

uniform sampler2D Texture;

out vec4 outColor;

void main()
{
    float a = texture( Texture, globalTextureCoordinate ).r;

    outColor = vec4( globalColor.rgb, globalColor.a * a );
}
