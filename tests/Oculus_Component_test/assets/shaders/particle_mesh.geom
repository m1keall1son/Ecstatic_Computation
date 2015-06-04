#version 330

layout (points) in;
layout (triangle_strip, max_vertices = 3) out;

in vec4 vPosition[];
in vec2 vTexCoord0[];
in vec3 vDirection[];
in float vId[];
flat in int  vDiscard[];

out F_DATA{
    vec3 Normal;
    vec4 Position;
    vec2 TexCoord0;
    float Dist;
}frag;

uniform mat4 ciViewMatrix;
uniform mat4 ciProjectionMatrix;
uniform mat4 ciModelMatrix;
uniform float uTriangleCutoff;
uniform vec2 uThresholds;
uniform float ciElapsedSeconds;

#include "simplex_noise.glsl"

void main(){
    
    if( vDiscard[0] == 0 ){
        
        vec3 me = vPosition[0].xyz;
        
        float noise = snoise( vec3(vId[0]) + ciElapsedSeconds*.5 );
        float noisex = snoise( vec3(vId[0])+ 300. + ciElapsedSeconds*.5 );
        float noisey = snoise( vec3(vId[0]) + 700. + ciElapsedSeconds*.5 );

        vec3 perp = normalize( cross( vDirection[0] + vec3( noise, noisex, noisey  ) , me ) );
        
        vec3 right = perp*.25+noise*.2 + me;
        vec3 left = -perp*.075+noisex*.2 + me;
        vec3 up = vec3(.25, .5,.25)*perp*.5+noisey*.2 + me;
        
        vec3 d1 = right - up;
        vec3 d2 = left - up;
        
        vec3 nrm = -mat3( transpose( inverse( ciModelMatrix ) ) ) * normalize( cross( d1, d2 ) );
        
        float d =  length( vec3( inverse(ciModelMatrix) * vec4(me,1.)) );
        
        if( d > .1 ){
        
        frag.Normal = nrm;
        frag.Position = vec4(up,1.);
        frag.TexCoord0 = vTexCoord0[0];
        frag.Dist = noise;
        
        gl_Position = ciProjectionMatrix * ciViewMatrix * vec4(up,1.);
        EmitVertex();
        
        frag.Normal = nrm;
        frag.Position = vec4(left,1.);
        frag.TexCoord0 = vTexCoord0[0];
        frag.Dist = noise;

        gl_Position = ciProjectionMatrix * ciViewMatrix * vec4(left,1.);
        EmitVertex();
        
        frag.Normal = nrm;
        frag.Position = vec4(right,1.);
        frag.TexCoord0 = vTexCoord0[0];
        frag.Dist = noise;
            
        gl_Position = ciProjectionMatrix * ciViewMatrix * vec4(right,1.);
        EmitVertex();
        
        EndPrimitive();
        }
    }
    
}