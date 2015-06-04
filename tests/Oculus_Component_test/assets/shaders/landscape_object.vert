#version 330

in vec4 ciPosition;
in vec2 ciTexCoord0;
in vec3 ciNormal;

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelMatrix;

out vec4 vPosition;
out vec2 vTexCoord0;
out vec3 vNormal;

void main()
{
    vTexCoord0 = ciTexCoord0;
    vPosition = ciModelMatrix * ciPosition;
    vNormal = mat3(transpose(inverse( ciModelMatrix ))) * ciNormal;
    
    gl_Position = ciModelViewProjection * ciPosition;
    
}