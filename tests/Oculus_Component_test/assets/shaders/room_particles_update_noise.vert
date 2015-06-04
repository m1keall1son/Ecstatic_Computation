#version 330

in vec4 inPosition;
in vec4 inVelocity;
in vec2 inTexCoord;

out vec4 outPosition;
out vec4 outVelocity;

#include "linear_map.glsl"
#include "curl_noise.glsl"
const int OCTAVES = 1;

uniform float uDeltaTime;
uniform vec3 uTarget;
uniform mat4 ciModelMatrix;

uniform float ciElapsedSeconds;

void main(){
    
    vec2 tc = inTexCoord;
    
    vec3 desired = uTarget - inPosition.xyz;
    //desired = clamp( desired, vec3(.0001), vec3(10000.) );
    float d = length(desired);
    desired = normalize(desired);
    //arrive
    if( d < .1){
        desired *= map( d, 0.,.1, 0., 40. );//maxspeed
    }else{
        desired *= 40.;
    }
    //steer
    vec3 steer = desired - inVelocity.xyz;
    if(length(steer) > 20.)steer = normalize(steer)*20.;//maxforce
    

    vec3 noisePosition = inPosition.xyz; //noise position scale
    
    float noiseTime = ciElapsedSeconds; //noise time scale
    
    vec4 xNoisePotentialDerivatives = vec4(0.0);
    vec4 yNoisePotentialDerivatives = vec4(0.0);
    vec4 zNoisePotentialDerivatives = vec4(0.0);
    
    for (int i = 0; i < OCTAVES; ++i) {
        
        float scale = (1.0 / 2.0) * pow(2.0, float(i));
        
        float noiseScale = pow( .01 , float(i));
        
        xNoisePotentialDerivatives += simplexNoiseDerivatives(vec4(noisePosition * pow(2.0, float(i)), noiseTime)) * noiseScale * scale;
        yNoisePotentialDerivatives += simplexNoiseDerivatives(vec4((noisePosition + vec3(123.4, 129845.6, -1239.1)) * pow(2.0, float(i)), noiseTime)) * noiseScale * scale;
        zNoisePotentialDerivatives += simplexNoiseDerivatives(vec4((noisePosition + vec3(-9519.0, 9051.0, -123.0)) * pow(2.0, float(i)), noiseTime)) * noiseScale * scale;
        
    }
    
    //compute curl
    vec3 noiseVelocity = vec3( zNoisePotentialDerivatives[1] - yNoisePotentialDerivatives[2],
                              xNoisePotentialDerivatives[2] - zNoisePotentialDerivatives[0],
                              yNoisePotentialDerivatives[0] - xNoisePotentialDerivatives[1] )*10.; //noise scale
    
    ////here

    
    outVelocity.xyz = (steer + noiseVelocity) / inPosition.w;
    outVelocity.w = inVelocity.w;
    outPosition.xyz = inPosition.xyz + outVelocity.xyz;
    outPosition.w = inPosition.w;


}
