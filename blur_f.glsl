#version 330

in vec2 UV;

uniform sampler2D texture;

uniform vec2 direction, coords;

layout(location=0) out vec4 Output;

void main()
{
	vec2 TexCoord=UV*coords;

	vec4
	s =texture2D(texture, TexCoord+(direction*-7.0))*0.0044299121055113265;
	s+=texture2D(texture, TexCoord+(direction*-6.0))*0.00895781211794;
	s+=texture2D(texture, TexCoord+(direction*-5.0))*0.0215963866053;
	s+=texture2D(texture, TexCoord+(direction*-4.0))*0.0443683338718;
	s+=texture2D(texture, TexCoord+(direction*-3.0))*0.0776744219933;
	s+=texture2D(texture, TexCoord+(direction*-2.0))*0.115876621105;
	s+=texture2D(texture, TexCoord+(direction*-1.0))*0.147308056121;
	s+=texture2D(texture, TexCoord                 )*0.159576912161;
	s+=texture2D(texture, TexCoord+(direction* 1.0))*0.147308056121;
	s+=texture2D(texture, TexCoord+(direction* 2.0))*0.115876621105;
	s+=texture2D(texture, TexCoord+(direction* 3.0))*0.0776744219933;
	s+=texture2D(texture, TexCoord+(direction* 4.0))*0.0443683338718;
	s+=texture2D(texture, TexCoord+(direction* 5.0))*0.0215963866053;
	s+=texture2D(texture, TexCoord+(direction* 6.0))*0.00895781211794;
	s+=texture2D(texture, TexCoord+(direction* 7.0))*0.0044299121055113265;

	Output=s;
}
