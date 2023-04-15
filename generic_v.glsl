#version 330

layout(location=0) in vec4 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec3 instancePosition;
layout(location=3) in vec3 instanceColor;

uniform mat4 mvp;
uniform vec4 eye;

out vec3 NormalU, ViewU, Color;

void main(void)
{
	gl_Position=mvp*(vPosition+vec4(instancePosition, 0.0f));

	NormalU=vNormal;
	ViewU=vec3(eye-(vPosition+vec4(instancePosition, 0.0f)));
	Color=instanceColor;
}