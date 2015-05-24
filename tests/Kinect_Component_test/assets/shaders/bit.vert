#version 330

in vec4 ciPosition;
in vec2 ciTexCoord0;
in vec3 ciTangent;
in vec3 ciBitangent;

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelView;
uniform mat3 ciNormalMatrix;

#include "util.glsl"

out vec4 vPosition;
out vec3 vNormal;

const int OCTAVES = 3;

void main(){

    vec4 pos = ciPosition;
    
    vec3 noisePosition = pos.xyz; //noise position scale
    vec3 tan_noisePosition = ciTangent;
    vec3 bi_noisePosition = ciBitangent;

    float noiseTime = ciElapsedSeconds; //noise time scale
    
    vec4 xNoisePotentialDerivatives = vec4(0.0);
    vec4 yNoisePotentialDerivatives = vec4(0.0);
    vec4 zNoisePotentialDerivatives = vec4(0.0);
    
    vec4 xTan = vec4(0.0);
    vec4 yTan = vec4(0.0);
    vec4 zTan = vec4(0.0);
    
    vec4 xBiTan = vec4(0.0);
    vec4 yBiTan = vec4(0.0);
    vec4 zBiTan = vec4(0.0);
    
    for (int i = 0; i < OCTAVES; ++i) {
        
        float scale = (1.0 / 2.0) * pow(2.0, float(i));
        
        float noiseScale = pow( .03 , float(i));
        
        xNoisePotentialDerivatives += simplexNoiseDerivatives(vec4(noisePosition * pow(2.0, float(i)), noiseTime)) * noiseScale * scale;
        yNoisePotentialDerivatives += simplexNoiseDerivatives(vec4((noisePosition + vec3(123.4, 129845.6, -1239.1)) * pow(2.0, float(i)), noiseTime)) * noiseScale * scale;
        zNoisePotentialDerivatives += simplexNoiseDerivatives(vec4((noisePosition + vec3(-9519.0, 9051.0, -123.0)) * pow(2.0, float(i)), noiseTime)) * noiseScale * scale;
        
        xTan += simplexNoiseDerivatives(vec4(tan_noisePosition * pow(2.0, float(i)), noiseTime)) * noiseScale * scale;
        yTan += simplexNoiseDerivatives(vec4((tan_noisePosition + vec3(123.4, 129845.6, -1239.1)) * pow(2.0, float(i)), noiseTime)) * noiseScale * scale;
        zTan += simplexNoiseDerivatives(vec4((tan_noisePosition + vec3(-9519.0, 9051.0, -123.0)) * pow(2.0, float(i)), noiseTime)) * noiseScale * scale;
        
        xBiTan += simplexNoiseDerivatives(vec4(bi_noisePosition * pow(2.0, float(i)), noiseTime)) * noiseScale * scale;
        yBiTan += simplexNoiseDerivatives(vec4((bi_noisePosition + vec3(123.4, 129845.6, -1239.1)) * pow(2.0, float(i)), noiseTime)) * noiseScale * scale;
        zBiTan += simplexNoiseDerivatives(vec4((bi_noisePosition + vec3(-9519.0, 9051.0, -123.0)) * pow(2.0, float(i)), noiseTime)) * noiseScale * scale;
        
    }
    
    
    //compute curl
    vec3 noiseVelocity = vec3( zNoisePotentialDerivatives[1] - yNoisePotentialDerivatives[2],
                               xNoisePotentialDerivatives[2] - zNoisePotentialDerivatives[0],
                               yNoisePotentialDerivatives[0] - xNoisePotentialDerivatives[1] ) * .075; //noise scale
    
    vec3 tan_noiseVelocity = vec3( zTan[1] - yTan[2],
                              xTan[2] - zTan[0],
                              yTan[0] - xTan[1] ) * .075; //noise scale
    
    vec3 bi_noiseVelocity = vec3( zBiTan[1] - yBiTan[2],
                              xBiTan[2] - zBiTan[0],
                              yBiTan[0] - xBiTan[1] ) * .075; //noise scale
    
    ////here
    
    pos.xyz += noiseVelocity;

    float theta = 0.000001;
    
    vec3 ptTangentSample = noisy(tePosition + theta * normalize(vecTangent));
    vec3 ptBitangentSample = noisy(tePosition + theta * normalize(vecBitangent));

    vNormal= normalize(cross(ptTangentSample - tePosition, ptBitangentSample - tePosition));

}