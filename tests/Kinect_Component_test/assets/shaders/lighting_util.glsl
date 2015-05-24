
///---LIGHT UBO---///

struct LightData
{
    vec3  position;
    float intensity;
    vec3  direction;
    float range;
    vec3  horizontal;
    float width;
    vec3  vertical;
    float height;
    vec4  color;
    vec2  attenuation;
    vec2  angle;
    mat4  shadowMatrix;
    mat4  modulationMatrix;
    int   shadowIndex;
    int   modulationIndex;
    int   flags;
    int   reserved;
    vec4  mapping;
};

struct LightSet {
    
    LightData uLight[24];
    int numLights;
    vec4 upDirection;
};

layout (std140) uniform uLights
{
    LightSet lights;
};

//uniform int             lights.uLightCount = 1;
uniform sampler2DShadow uShadowMap;

float saturate( in float v )
{
    return clamp( v, 0.0, 1.0 );
}

float calcShadow( in sampler2DShadow map, in vec4 sc )
{
    sc.z -= 0.001; // Apply shadow bias.
    return textureProj( map, sc );
}

float calcShadowPCF4x4( in sampler2DShadow map, in vec4 sc )
{
    const int r = 1;
    const int s = 2 * r;
    
    sc.z -= 0.0025; // Apply shadow bias.
    
    float shadow = 0.0;
    shadow += textureProjOffset( map, sc, ivec2(-s,-s) );
    shadow += textureProjOffset( map, sc, ivec2(-r,-s) );
    shadow += textureProjOffset( map, sc, ivec2( r,-s) );
    shadow += textureProjOffset( map, sc, ivec2( s,-s) );
    
    shadow += textureProjOffset( map, sc, ivec2(-s,-r) );
    shadow += textureProjOffset( map, sc, ivec2(-r,-r) );
    shadow += textureProjOffset( map, sc, ivec2( r,-r) );
    shadow += textureProjOffset( map, sc, ivec2( s,-r) );
    
    shadow += textureProjOffset( map, sc, ivec2(-s, r) );
    shadow += textureProjOffset( map, sc, ivec2(-r, r) );
    shadow += textureProjOffset( map, sc, ivec2( r, r) );
    shadow += textureProjOffset( map, sc, ivec2( s, r) );
    
    shadow += textureProjOffset( map, sc, ivec2(-s, s) );
    shadow += textureProjOffset( map, sc, ivec2(-r, s) );
    shadow += textureProjOffset( map, sc, ivec2( r, s) );
    shadow += textureProjOffset( map, sc, ivec2( s, s) );
    
    return shadow * (1.0 / 16.0);
}

float calcAngularAttenuation( in vec3 L, in vec3 D, in vec2 cutoffs )
{
    return smoothstep( cutoffs.x, cutoffs.y, dot( -D, L ) );
}

float calcdistAttenuation( in float dist, in vec2 coeffs )
{
    return 1.0 / ( 1.0 + dist * coeffs.x + dist * dist * coeffs.y );
}

vec3 calcRepresentativePoint( in vec3 lightStart, in vec3 lightEnd, in vec3 v, in vec3 r )
{
    // See: http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf (page 17)
    vec3 l0 = lightStart - v;
    vec3 l1 = lightEnd - v;
    vec3 ld = l1 - l0;
    float a = dot( r, ld );
    float t = ( dot( r, l0 ) * a - dot( l0, ld ) ) / ( dot( ld, ld ) - a * a );
    
    // Calculate a dist factor. Multiply it by the light's length to find the dist to the light source.
    //d = max( max( 0.0, 0.0 - t ), max( 0.0, t - 1.0 ) );
    
    return lightStart + saturate( t ) * (lightEnd - lightStart);
}

// Scattering Implementation (currently only point lights are supported)
// See code From Miles Macklin: http://blog.mmacklin.com/2010/05/29/in-scattering-demo/
float calcScattering( vec3 vertPos, vec3 lightPos )
{
    vec3 r = vertPos - vec3( 0 );
    vec3 q = vec3( 0 ) - lightPos;
    
    float b = dot( normalize( r ), q );
    float c = dot( q, q );
    float d = length( r );
    
    // Evaluate integral.
    float s = inversesqrt( max( 0.0001, c - b * b ) );	
    return s * ( atan( ( d + b ) * s ) - atan( b * s ) );
}
