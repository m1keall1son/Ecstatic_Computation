#version 150

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelMatrix;

in vec4 ciPosition;
in vec3 ciNormal;

out vec4 vertPosition;
out vec3 vertNormal;

void main(void)
{
	vertPosition = ciModelMatrix * ciPosition;
	vertNormal = mat3(transpose(inverse(ciModelMatrix))) * ciNormal;
	gl_Position = ciModelViewProjection * ciPosition;
}