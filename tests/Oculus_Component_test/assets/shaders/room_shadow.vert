#version 330

#include "simplex_noise.glsl"

uniform mat4 ciModelViewProjection;

uniform float ciElapsedSeconds;

uniform float uNoiseScale;
uniform float uNoiseMax;

in vec4 ciPosition;
in vec2 ciTexCoord0;
in vec3 ciNormal;

void main(void)
{
    float noise = (snoise( ciPosition.xyz*10.+ ciElapsedSeconds )* .5 + .5) * uNoiseMax * uNoiseScale;
    float grad = pow( exp(  -pow( length( ciTexCoord0 - vec2(.5) ), 2. )), 64. )*uNoiseScale;
    
    vec4 pos = ciPosition;
    pos.xyz += noise*grad*ciNormal;
    
    gl_Position = ciModelViewProjection * pos;
}