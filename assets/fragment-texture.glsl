#version 400

in vec2 globalTextureCoordinate;
in vec4 globalColor;

uniform sampler2D Texture;

out vec4 outColor;

void main()
{
	vec4 texColor = texture( Texture, globalTextureCoordinate );
	//outColor = vec4( texColor.r, texColor.r, texColor.r, texColor.r );
	//outColor = texture( Texture, globalTextureCoordinate );
	outColor = vec4( globalColor.rgb, globalColor.a * texColor.r );
    //float textureAlpha = texture( Texture, globalTextureCoordinate ).a;
    //outColor = vec4( globalColor.rgb, globalColor.a * textureAlpha );
	//outColor = vec4( 0.0f, 0.0f, globalColor.b, textureAlpha );
}
