#version 330

#include "gbuffer_util.glsl"

in F_DATA{
    vec3 Normal;
    vec4 Position;
    vec2 TexCoord0;
}frag;

layout (location = 0) out vec3	oAlbedo;
layout (location = 1) out vec4	oData;

void main(void)
{
    oAlbedo = vec3(1.);
    //xy = normal encoded
    //z = material id
    //w ?
    oData = vec4( encodeNormal( -frag.Normal ), 0., 0.);
     
}