#version 330

in vec3 ciPosition;
in vec2 ciTexCoord0;

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelMatrix;

uniform samplerBuffer uPositions;
uniform samplerBuffer uVelocities;

out vec4 vPosition;
out vec2 vTexCoord0;
out vec3 vDirection;
out float vId;
flat out int vDiscard;

void main(){
    
    vId = float( gl_VertexID );
    vTexCoord0 = ciTexCoord0;
    
    vec2 tc = ciTexCoord0*vec2(640.,480.);
    
    vec3 pos = texelFetch( uPositions, int(tc.x*tc.y) ).xyz;
    vec4 velocity = texelFetch( uVelocities, int(tc.x*tc.y) );
    
    vDirection   = normalize(velocity.xyz);
    vPosition   = ciModelMatrix * vec4(pos,1.);
    vDiscard = int( velocity.w );
    
    gl_Position = ciModelViewProjection * vec4(pos,1.);
}
