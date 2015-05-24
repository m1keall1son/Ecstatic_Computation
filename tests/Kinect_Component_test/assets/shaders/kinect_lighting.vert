#version 330

in vec4 ciPosition;
in vec2 ciTexCoord0;

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelView;
uniform mat3 ciNormalMatrix;

out vec4 vPosition;
out vec3 vNormal;
out vec2 vTexCoord0;

#include "kinect_util.glsl"

void main(){
    
    vTexCoord0 = ciTexCoord0;
    
    vec4 pos		= ciPosition;
    uint rawdepth	= texture( uDepthTexture, ciTexCoord0 ).r;
    pos.xyz         = depth_mm_to_world( ciTexCoord0.x*640., (1.-ciTexCoord0.y)*480., raw_to_mm( float(rawdepth) ) );
    
    vec2 inc = vec2(1.)/vec2(640., 480.);
    
    vec3 neighbor_rt = vec3(0.);

    vec2 tc_rt = vec2(ciTexCoord0.x + inc.x, ciTexCoord0.y);
    uint nr_rawdepth = texture( uDepthTexture, tc_rt ).r;
    neighbor_rt.xyz  = depth_mm_to_world( tc_rt.x*640., (1.-tc_rt.y)*480., raw_to_mm( float(nr_rawdepth) ) );
    
    vec3 neighbor_down = vec3(0.);
    vec2 tc_dn = vec2(ciTexCoord0.x + inc.x, ciTexCoord0.y);
    uint nd_rawdepth   = texture( uDepthTexture, tc_dn ).r;
    neighbor_down.xyz    = depth_mm_to_world( tc_dn.x*640., (1.-tc_dn.y)*480., raw_to_mm( float(nd_rawdepth) ) );
    
    vec3 d1 = neighbor_rt - pos.xyz;
    vec3 d2 = neighbor_down - pos.xyz;
    vec3 norm = normalize( cross( d1, d2 ) );

    vNormal = ciNormalMatrix * norm;
    vPosition = ciModelView * vPosition;
    
    gl_Position = ciModelViewProjection * pos;
}
