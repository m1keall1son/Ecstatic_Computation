#version 330

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec4 vPosition[];
in vec2 vTexCoord0[];
in float vDistanceFromCenter[];

out F_DATA{
    vec3 Normal;
    vec4 Position;
    vec2 TexCoord0;
    float DistanceFromCenter;
}frag;

uniform mat4 ciModelMatrix;

void main(){
    
    
    vec3 d1 = vPosition[1].xyz - vPosition[0].xyz;
    vec3 d2 = vPosition[2].xyz - vPosition[0].xyz;
    
    vec3 nrm = mat3( transpose( inverse( ciModelMatrix ) ) ) * normalize( cross( d1, d2 ) );
    
    for(int i=0;i<gl_in.length();i++){
        
        frag.Normal = nrm;
        frag.Position = vPosition[i];
        frag.TexCoord0 = vTexCoord0[i];
        frag.DistanceFromCenter = vDistanceFromCenter[i];
        
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    
    EndPrimitive();
    
    
}