#version 330

in vec4 ciPosition;
in vec2 ciTexCoord0;

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelMatrix;

uniform sampler2D uMap;

void main()
{
    float h = texture( uMap, ciTexCoord0 ).r;
    
    vec4 pos = ciPosition;
    pos.y += h*400.;
    
    gl_Position = ciModelViewProjection * pos;
    
}