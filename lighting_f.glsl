#version 330

in vec3 NormalU, ViewU, Color;

uniform samplerCube specular;
uniform vec4 matrix[12];

layout(location=0) out vec4 Output;

void main()
{
	vec3 Normal, View, Reflect, Diffuse;
	vec4 Temp;

	// Renormalize normal
	Normal=normalize(NormalU);

	// View direction
	View=normalize(ViewU);

	// Calculate diffuse from spherical harmonics
	Temp.x=dot(matrix[ 0], vec4(Normal, 1.0));
	Temp.y=dot(matrix[ 1], vec4(Normal, 1.0));
	Temp.z=dot(matrix[ 2], vec4(Normal, 1.0));
	Temp.w=dot(matrix[ 3], vec4(Normal, 1.0));
	Diffuse.x=dot(vec4(Normal, 1.0), Temp);
	Temp.x=dot(matrix[ 4], vec4(Normal, 1.0));
	Temp.y=dot(matrix[ 5], vec4(Normal, 1.0));
	Temp.z=dot(matrix[ 6], vec4(Normal, 1.0));
	Temp.w=dot(matrix[ 7], vec4(Normal, 1.0));
	Diffuse.y=dot(vec4(Normal, 1.0), Temp);
	Temp.x=dot(matrix[ 8], vec4(Normal, 1.0));
	Temp.y=dot(matrix[ 9], vec4(Normal, 1.0));
	Temp.z=dot(matrix[10], vec4(Normal, 1.0));
	Temp.w=dot(matrix[11], vec4(Normal, 1.0));
	Diffuse.z=dot(vec4(Normal, 1.0), Temp);

	// Reflection
	Reflect=texture(specular, reflect(-View, Normal)).xyz;

	Output=vec4(Diffuse*Color+Reflect*clamp(1.0, 0.0, pow(1.0-dot(View, Normal), 2.0)), 1.0);
}