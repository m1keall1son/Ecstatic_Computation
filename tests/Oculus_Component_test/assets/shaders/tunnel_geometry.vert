#version 150

uniform mat4 ciModelView;
uniform mat4 ciModelViewProjection;

in vec4 ciPosition;
in vec2 ciTexCoord0;

uniform float ciElapsedSeconds;

#include "curl_noise.glsl"

out vec4 vPosition;
out vec2 vTexCoord0;
out float vId;

const int OCTAVES = 3;

uniform float uNoiseScale;

void main(void)
{
    vTexCoord0 = ciTexCoord0;
    vId = float( gl_VertexID );
    
    vec4 pos = ciPosition;
    
    vec3 noisePosition = pos.xyz; //noise position scale
    
    float noiseTime = ciElapsedSeconds; //noise time scale
    
    vec4 xNoisePotentialDerivatives = vec4(0.0);
    vec4 yNoisePotentialDerivatives = vec4(0.0);
    vec4 zNoisePotentialDerivatives = vec4(0.0);
    
    for (int i = 0; i < OCTAVES; ++i) {
        
        float scale = (1.0 / 2.0) * pow(2.0, float(i));
        
        float noiseScale = pow( .03 , float(i));
        
        xNoisePotentialDerivatives += simplexNoiseDerivatives(vec4(noisePosition * pow(2.0, float(i)), noiseTime)) * noiseScale * scale;
        yNoisePotentialDerivatives += simplexNoiseDerivatives(vec4((noisePosition + vec3(123.4, 129845.6, -1239.1)) * pow(2.0, float(i)), noiseTime)) * noiseScale * scale;
        zNoisePotentialDerivatives += simplexNoiseDerivatives(vec4((noisePosition + vec3(-9519.0, 9051.0, -123.0)) * pow(2.0, float(i)), noiseTime)) * noiseScale * scale;
        
    }
    
    //compute curl
    vec3 noiseVelocity = vec3( zNoisePotentialDerivatives[1] - yNoisePotentialDerivatives[2],
                              xNoisePotentialDerivatives[2] - zNoisePotentialDerivatives[0],
                              yNoisePotentialDerivatives[0] - xNoisePotentialDerivatives[1] ) * uNoiseScale; //noise scale
    
    ////here
    
    pos.xyz     += noiseVelocity;
    
    vPosition = ciModelView * pos;
    
    gl_Position = ciModelViewProjection * pos;
}