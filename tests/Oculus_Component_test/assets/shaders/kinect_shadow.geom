#version 330

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec4 vPosition[];

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
            
            gl_Position = gl_in[i].gl_Position;
            EmitVertex();
        }
        
        EndPrimitive();
    }
    
}