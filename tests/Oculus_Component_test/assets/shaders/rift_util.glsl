
struct RiftData {
    mat4 matrices[ 4 ];
    float nearClip;
    float farClip;
    float reserved0;
    float reserved1;
};

layout (std140) uniform uRift
{
    RiftData rift;
};

out float gl_ClipDistance[1];

vec4 eyeClipEdge[2] = vec4[2]( vec4(1,0,0,0), vec4(-1,0,0,0) );
float eyeOffsetScale[2] = float[2]( 0.5, -0.5 );