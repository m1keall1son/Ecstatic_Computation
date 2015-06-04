#version 330

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec4 vPosition[];
in vec2 vTexCoord0[];

out F_DATA{
    vec3 Normal;
    vec4 Position;
    vec2 TexCoord0;
    vec3 Color;
}frag;

#include "simplex_noise.glsl"

uniform mat4 ciModelMatrix;
uniform float uColorScale;
uniform float ciElapsedSeconds;

void main(){
    
    
    vec3 d1 = vPosition[1].xyz - vPosition[0].xyz;
    vec3 d2 = vPosition[2].xyz - vPosition[0].xyz;
    
    vec3 nrm = mat3( transpose( inverse( ciModelMatrix ) ) ) * normalize( cross( d1, d2 ) );
    
    float color = snoise( vPosition[0].xyz + ciElapsedSeconds*2. )*uColorScale;
    
    for(int i=0;i<gl_in.length();i++){
        
        frag.Normal = nrm;
        frag.Position = vPosition[i];
        frag.TexCoord0 = vTexCoord0[i];
        frag.Color = vec3(0.,color,color)*.3;
        
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    
    EndPrimitive();
    
    
}