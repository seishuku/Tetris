#version 330

in vec3 NormalU, ViewU, Color;

uniform samplerCube skybox;

layout(location=0) out vec4 Output;

// GLSL's refraction function doesn't work for me, not sure why. I use my own.

void main()
{
	vec3 Normal, View, Reflect, Refract;

	// Renormalize normal
	Normal=normalize(NormalU);

	// View direction
	View=normalize(ViewU);

	// Refraction
	Refract=texture(skybox, refract(-View, Normal, 1.0/1.1)).xyz;

	// Reflection
	Reflect=texture(skybox, reflect(-View, Normal)).xyz;

	// out=color*refract+reflect*viginette
	Output=vec4(Color*Refract+Reflect*clamp(1.0, 0.0, pow(1.0-dot(View, Normal), 2.0)), 1.0);
}