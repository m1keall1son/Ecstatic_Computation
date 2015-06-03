#version 330

in vec3 inPosition;
in vec4 inData;

uniform mat4 ciModelViewProjection;

void main(){
    gl_PointSize = 10.;
    gl_Position = ciModelViewProjection * vec4(inPosition,1.);
}