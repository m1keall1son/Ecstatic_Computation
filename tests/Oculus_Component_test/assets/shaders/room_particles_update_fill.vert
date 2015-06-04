#version 330

in vec4 inPosition;
in vec4 inVelocity;
in vec2 inTexCoord;

out vec4 outPosition;
out vec4 outVelocity;

#include "kinect_util.glsl"

uniform vec2 uThresholds;

void main(){
    
        vec4 pos		= inPosition;
        uint rawdepth	= texture( uDepthTexture, inTexCoord ).r;
        float mm        = raw_to_mm( float(rawdepth) );
        pos.xyz         = depth_mm_to_world( inTexCoord.x*640., (1.-inTexCoord.y)*480., mm );
    
        float dis = 0.;
        if( pos.z < uThresholds.x || pos.z > uThresholds.y || rawdepth == 0u ){
            dis = 1.;
        }
    
        outPosition = pos;
        outVelocity = vec4(0, 0, 0, dis);
//
}
