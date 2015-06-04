#version 330

in vec4 ciPosition;
in vec2 ciTexCoord0;

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelMatrix;

uniform sampler2D uMap;
uniform float uSinkHoleScale;

out vec4 vPosition;
out vec2 vTexCoord0;

void main()
{
    float h = texture( uMap, ciTexCoord0 ).r;
    
    vec4 pos = ciPosition;
    pos.y += h*4000.;
    
    vTexCoord0 = ciTexCoord0;
    
    float grad = pow( exp(  -pow( length( ciTexCoord0 - vec2(.5) ), 2. )), 300. )*uSinkHoleScale;
    
    pos.y -= grad*40.;
    
    vPosition = ciModelMatrix * pos;
    
    gl_Position = ciModelViewProjection * pos;
    
}