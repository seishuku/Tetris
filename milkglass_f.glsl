#version 330

in vec3 NormalU, ViewU, Color;

uniform samplerCube specular;

layout(location=0) out vec4 Output;

void main()
{
	vec3 Normal, View, Reflect, Refract;

	// Renormalize normal
	Normal=normalize(NormalU);

	// View direction
	View=normalize(ViewU);

	// Reflection
	Reflect=texture(specular, reflect(-View, Normal)).xyz;

	// Refraction
	Refract=texture(specular, refract(-View, Normal, 1.0/1.1)).xyz;

	Output=vec4(Refract*Color+Reflect*clamp(1.0, 0.0, 1.0-pow(dot(View, Normal), 2.0)), 1.0);
}