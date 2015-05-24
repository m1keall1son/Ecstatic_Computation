#version 330

in vec4 ciPosition;
in vec2 ciTexCoord0;

uniform mat4 ciModelViewProjection;

#include "kinect_util.glsl"

void main(){
    
    vec4 pos		= ciPosition;
    uint rawdepth	= texture( uDepthTexture, ciTexCoord0 ).r;
    pos.xyz         = depth_mm_to_world( ciTexCoord0.x*640., (1.-ciTexCoord0.y)*480., raw_to_mm( float(rawdepth) ) );
    
    gl_Position = ciModelViewProjection * pos;
}
