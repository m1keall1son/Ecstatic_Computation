#version 330

#include "simplex_noise.glsl"

uniform mat4 ciModelMatrix;
uniform mat4 ciModelViewProjection;

uniform float uTime;

uniform float uNoiseScale;
uniform float uNoiseMax;

in vec4 ciPosition;
in vec2 ciTexCoord0;
in vec3 ciNormal;

out vec4 vPosition;
out vec2 vTexCoord0;
out float vGrad;

void main(void)
{
    float noise = (snoise( ciPosition.xyz*10.+ uTime )*.5 + .5 ) * uNoiseMax * uNoiseScale;
    float grad = max(pow( exp(  -pow( length( ciTexCoord0 - vec2(.5) ), 2. )), 64. ), .2) * uNoiseScale;
    
    vec4 pos = ciModelMatrix * ciPosition;
    pos.xyz += noise*grad*ciNormal;
    
    vPosition = pos;
    vTexCoord0 = ciTexCoord0;
    vGrad = grad;
    
    gl_Position = ciModelViewProjection * pos;
}