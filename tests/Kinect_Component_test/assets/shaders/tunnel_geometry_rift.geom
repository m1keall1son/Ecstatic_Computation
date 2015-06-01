#version 330

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec4 vPosition[];
in vec2 vTexCoord0[];
in float vId[];
in mat3 vNormalMatrix[];

out F_DATA{
    vec3 Normal;
    vec4 Position;
    vec2 TexCoord0;
    //    vec3 Color;
}frag;

#include "simplex_noise.glsl"

void main(){
    
    
    vec3 d1 = vPosition[1].xyz - vPosition[0].xyz;
    vec3 d2 = vPosition[2].xyz - vPosition[0].xyz;
    
    vec3 nrm = vNormalMatrix * normalize( cross( d1, d2 ) );
    
    //    vec3 color = vec3( snoise( vec3( vId[0]*.1 )*.5+.5 ) );
    
    for(int i=0;i<gl_in.length();i++){
        
        frag.Normal = nrm;
        frag.Position = vPosition[i];
        frag.TexCoord0 = vTexCoord0[i];
        //        frag.Color = color*.001;
        
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    
    EndPrimitive();
    
    
}