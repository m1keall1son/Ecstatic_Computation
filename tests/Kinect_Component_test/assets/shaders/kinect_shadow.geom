#version 330

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform float uTriangleCutoff;

void main(){
    
    if(
       length( gl_in[0].gl_Position - gl_in[1].gl_Position ) > uTriangleCutoff ||
       length( gl_in[0].gl_Position - gl_in[2].gl_Position ) > uTriangleCutoff ||
       length( gl_in[1].gl_Position - gl_in[2].gl_Position ) > uTriangleCutoff
       ){
    }else{
        
        
        for(int i=0;i<gl_in.length();i++){
            
            gl_Position = gl_in[i].gl_Position;
            EmitVertex();
        }
        
        EndPrimitive();
    }
    
}