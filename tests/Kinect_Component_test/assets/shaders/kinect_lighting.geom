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
//uniform mat3 ciNormalMatrix;

void main(){
    
    if(
       length( gl_in[0].gl_Position - gl_in[1].gl_Position ) > uTriangleCutoff ||
       length( gl_in[0].gl_Position - gl_in[2].gl_Position ) > uTriangleCutoff ||
       length( gl_in[1].gl_Position - gl_in[2].gl_Position ) > uTriangleCutoff
       ){
    }else{
        
//        vec3 d1 = vPosition[1].xyz - vPosition[0].xyz;
//        vec3 d2 = vPosition[2].xyz - vPosition[0].xyz;
//        
//        vec3 nrm = ciNormalMatrix * normalize( cross( d1, d2 ) );
        
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