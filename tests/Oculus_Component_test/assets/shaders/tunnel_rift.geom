#version 330

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec4 vPosition[];
in vec2 vTexCoord0[];
in float vEye[];
in mat3 vNormalMatrix[];

out F_DATA{
    vec3 Normal;
    vec4 Position;
    vec2 TexCoord0;
    float Eye;
}frag;

#include "simplex_noise.glsl"

void main(){
    
    
    vec3 d1 = vPosition[1].xyz - vPosition[0].xyz;
    vec3 d2 = vPosition[2].xyz - vPosition[0].xyz;
    
    vec3 nrm = normalize( cross( d1, d2 ) );
    
    //    vec3 color = vec3( snoise( vec3( vId[0]*.1 )*.5+.5 ) );
    
    for(int i=0;i<gl_in.length();i++){
        
        frag.Normal = vNormalMatrix[i] * nrm;
        frag.Position = vPosition[i];
        frag.TexCoord0 = vTexCoord0[i];
        frag.Eye = vEye[i];
        
        gl_Position = gl_in[i].gl_Position;
        gl_ClipDistance[0] = gl_in[i].gl_ClipDistance[0];
        EmitVertex();
    }
    
    EndPrimitive();
    
    
}