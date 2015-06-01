#version 330

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec4 vPosition[];
in vec2 vTexCoord0[];
in vec3 vNormal[];

out F_DATA{
    vec3 Normal;
    vec4 Position;
    vec2 TexCoord0;
}frag;

uniform float uTriangleCutoff;
uniform vec2 uThresholds;

void main(){
    
    if(
       length( gl_in[0].gl_Position - gl_in[1].gl_Position ) > uTriangleCutoff ||
       length( gl_in[0].gl_Position - gl_in[2].gl_Position ) > uTriangleCutoff ||
       length( gl_in[1].gl_Position - gl_in[2].gl_Position ) > uTriangleCutoff ||
       vPosition[0].z < uThresholds.x || vPosition[0].z > uThresholds.y ||
       vPosition[1].z < uThresholds.x || vPosition[1].z > uThresholds.y ||
       vPosition[2].z < uThresholds.x || vPosition[2].z > uThresholds.y
       ){
    }else{
        
        for(int i=0;i<gl_in.length();i++){
            
            frag.Normal = vNormal[i];
            frag.Position = vPosition[i];
            frag.TexCoord0 = vTexCoord0[i];
            
            gl_Position = gl_in[i].gl_Position;
            EmitVertex();
        }
        
        EndPrimitive();
    }
    
}