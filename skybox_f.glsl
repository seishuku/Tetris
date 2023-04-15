#version 330

in vec3 UV;

uniform samplerCube skybox;

layout(location=0) out vec4 Output;

void main()
{
	Output=texture(skybox, UV);
}
