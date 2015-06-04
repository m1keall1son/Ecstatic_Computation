#version 330

layout (points) in;
layout (triangle_strip, max_vertices = 3) out;

in vec4 vPosition[];
in vec2 vTexCoord0[];
in vec3 vDirection[];
in float vId[];
flat in int  vDiscard[];

uniform mat4 ciViewMatrix;
uniform mat4 ciProjectionMatrix;
uniform mat4 ciModelMatrix;
uniform float uTriangleCutoff;
uniform vec2 uThresholds;
uniform float ciElapsedSeconds;

void main(){
    
    if( vDiscard[0] == 0 ){
        
        vec3 me = vPosition[0].xyz;
        vec3 perp = normalize( cross( cross(vDirection[0],vec3(0,1,0))  , me ) );
        
        vec3 right = perp*.3 + me;
        vec3 left = -perp*.3 + me;
        vec3 up = vec3(.25, .5,.25)*perp*.3 + me;
        
        if( length( vec3( inverse(ciModelMatrix) * vec4(me,1.)) ) > .1 ){
        
        
        gl_Position = ciProjectionMatrix * ciViewMatrix * vec4(up,1.);
        EmitVertex();
        
        gl_Position = ciProjectionMatrix * ciViewMatrix * vec4(left,1.);
        EmitVertex();
        
        gl_Position = ciProjectionMatrix * ciViewMatrix * vec4(right,1.);
        EmitVertex();
        
        EndPrimitive();
        }
    }
    
}