#version 330

layout(location=0) in vec4 vPosition;

uniform mat4 mvp;

out vec3 UV;

void main()
{
	gl_Position=mvp*vPosition;
	UV=vec3(vPosition);
}
