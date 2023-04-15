#version 330

in vec2 UV;

uniform sampler2D blur;
uniform sampler2D original, original1, original2, original3;
uniform sampler2D original4, original5, original6, original7;
uniform vec2 coords;
uniform float exposure;

layout(location=0) out vec4 Output;

const vec4 gamma=vec4(0.45, 0.45, 0.45, 0.0);

void main(void)
{
	vec4
	ms =texture2D(original,  UV*coords)*0.125;
	ms+=texture2D(original1, UV*coords)*0.125;
	ms+=texture2D(original2, UV*coords)*0.125;
	ms+=texture2D(original3, UV*coords)*0.125;
	ms+=texture2D(original4, UV*coords)*0.125;
	ms+=texture2D(original5, UV*coords)*0.125;
	ms+=texture2D(original6, UV*coords)*0.125;
	ms+=texture2D(original7, UV*coords)*0.125;

	Output=pow(mix(texture2D(blur, UV*coords), ms, 0.6)*exposure, gamma);
}
