
///---KINECT DEPTH TO WORLD---///

uniform usampler2D  uDepthTexture;
uniform float       uRef_pix_size;
uniform float       uRef_distance;
uniform float       uConst_shift;
uniform float       uDcmos_emitter_dist;

const float parameter_coefficient = 4;
const float shift_scale = 10;
const float pixel_size_factor = 1;
#define S2D_CONST_OFFSET 0.375

/// convert raw shift value to metric depth (in mm)
float raw_to_mm(float raw)
{
    float fixed_ref_x = ( ( raw - (parameter_coefficient * uConst_shift / pixel_size_factor ) ) / parameter_coefficient ) - S2D_CONST_OFFSET;
    float metric = fixed_ref_x * uRef_pix_size * pixel_size_factor;
    return shift_scale * ( ( metric * uRef_distance / ( uDcmos_emitter_dist - metric ) ) + uRef_distance );
}

///depth to world conversion
vec3 depth_mm_to_world( float cam_x, float cam_y, float depth_mm ){
    
    vec3  world;
    float factor    = 2. * uRef_pix_size * depth_mm / uRef_distance;
    world.x         = ( cam_x - 320. ) * factor;
    world.y         = ( cam_y - 240. ) * factor;
    world.z         = depth_mm;
    return          world;
    
}
