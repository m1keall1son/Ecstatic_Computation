
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

///---Material---///

struct Material
{
    vec3 diffuse;
    vec3 specular;
    float shininess;
    vec3 ambient;
};

struct FogData { float min; float max; float density; vec3 color; };

struct Atmosphere
{
    FogData fog;
    float scattering;
};

const float kPi = 3.14159265;

//uniform int             lights.uLightCount = 1;
uniform sampler2DShadow uShadowMap;
uniform mat4 ciViewMatrix;

float saturate( in float v )
{
    return clamp( v, 0.0, 1.0 );
}

float calcShadow( in sampler2DShadow map, in vec4 sc )
{
    sc.z -= 0.0025; // Apply shadow bias.
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

void calcLighting(      const in Material material,
                        const in Atmosphere atmosphere,
                        const in vec4 vertPosition,
                        const in vec3 vertNormal,
                        inout vec3 ambient,
                        inout vec3 diffuse,
                        inout vec3 specular
                        )
{
    
    // Calculate normal and eye vector.
    vec3 N = normalize( vertNormal );
    vec3 E = normalize( -vertPosition.xyz );
    
    // Hemispherical ambient lighting.
    float hemi = 0.5 + 0.5 * dot(  vec3(0,1,0) , N );
    ambient = mix( vec3( 0 ), material.ambient, hemi ) * material.diffuse;
    
    // Calculate lighting.
    for( int i=0; i<lights.numLights; ++i )
    {
        int ind =  i;
        // Fetch light type.
        int flags = lights.uLight[ ind ].flags;
        int type = flags & 0xF;
        
        bool isDirectional = ( type == 0 );
        bool isSpotOrPoint = ( type & 0x5 ) > 0;
        bool isSpotOrWedge = ( type & 0xC ) > 0;
        bool isLinearLight = ( type & 0xA ) > 0;
        
        bool hasShadows = ( flags & 0x20 ) > 0;
        bool hasModulation = ( flags & 0x10 ) > 0;
        
        // Calculate end-points of the light for convenience.
        
        vec3 lPosition =  lights.uLight[ind].position;
        vec3 lightStart = lPosition;
        vec3 lightEnd   = lPosition + lights.uLight[ind].width * lights.uLight[ind].horizontal;
        vec3 lightColor = lights.uLight[ind].color.rgb * lights.uLight[ind].intensity;
        
        // Calculate direction and dist to light.
        float dist = 1.0;
        vec3 lDirection = lights.uLight[ind].direction;
        vec3  L = -lDirection;
        
        
        if( !isDirectional )
        {
            float t = clamp( dot( vertPosition.xyz - lightStart, lights.uLight[ind].horizontal ), 0.0, lights.uLight[ind].width );
            vec3 lightPosition = lightStart + t * lights.uLight[ind].horizontal;
            L = lightPosition - vertPosition.xyz; dist = length( L ); L /= dist;
        }
        
        // Calculate shadow.
        float shadow = 1.0;
        if( hasShadows )
        {
            vec4 shadowCoord = lights.uLight[ind].shadowMatrix * vertPosition;
            
            vec2 size = textureSize( uShadowMap, 0 );
            vec2 ratio = lights.uLight[i].mapping.zw / size;
            vec2 offset = lights.uLight[i].mapping.xy / size;
            shadowCoord.xy *= ratio;
            shadowCoord.xy += offset;
            //[ lights.uLight[i].shadowIndex ]
            shadow = calcShadowPCF4x4( uShadowMap, shadowCoord );
        }
        
        // Calculate modulation.
        vec3 modulation = vec3( 1 );
        //		if( hasModulation )
        //		{
        //			if( ( type & 0x1 ) > 0 ) // Point light: based on latlong map.
        //			{
        //				vec4 D = lights.uLight[i].modulationMatrix * vec4( -L, 0 );
        //				vec2 modulationCoord = vec2(0.5) + vec2( 0.5 * atan( D.z, D.x ), atan( D.y, length( D.xz ) ) ) / kPi;
        //				modulation = texture( uModulationMap[ lights.uLight[i].modulationIndex ], modulationCoord  ).rgb;
        //			}
        //			else if( ( type & 0x4 ) > 0 ) // Spot light
        //			{
        //				vec4 modulationCoord = lights.uLight[i].modulationMatrix * vertPosition;
        //				modulation = textureProj( uModulationMap[ lights.uLight[i].modulationIndex ], modulationCoord ).rgb;
        //			}
        //		}
        
        // Calculate attenuation.
        float angularAttenuation = mix( 1.0, calcAngularAttenuation( L, lights.uLight[ind].direction, lights.uLight[ind].angle ), isSpotOrWedge );
        float distAttenuation = mix( 1.0, calcdistAttenuation( dist, lights.uLight[ind].attenuation ), !isDirectional );
        vec3  colorAttenuation = modulation * lightColor;
        
        // Calculate diffuse color (clamp it to the light's range).
        float lambert = max( 0.0, dot( N, L ) );
        float range = mix( 1.0, step( dist, lights.uLight[ind].range ), !isDirectional );
        diffuse += shadow * range * colorAttenuation * distAttenuation * angularAttenuation * lambert * material.diffuse;
        
        // Calculate light Scattering.
        if( !isDirectional )
        {
            vec3 lightPosition = calcRepresentativePoint( lightStart, lightEnd, vertPosition.xyz, -E );
            float scatter = calcScattering( vec4(ciViewMatrix * vertPosition).xyz, vec3(ciViewMatrix * vec4(lightPosition,1)) );
            diffuse += atmosphere.scattering * scatter * lightColor;
        }
        
        // Calculate representative light vector (for linear lights only). TODO: works for standard Phong only, Blinn-Phong seems wrong.
        if( isLinearLight ) {
            vec3 lightPosition = calcRepresentativePoint( lightStart, lightEnd, vertPosition.xyz, -reflect( E, N ) );
            L = lightPosition - vertPosition.xyz; dist = length( L ); L /= dist;
            
            distAttenuation = calcdistAttenuation( dist, lights.uLight[ind].attenuation );
        }
        
        float shine = material.shininess;
        
        // Calculate specular color.
#define USE_BLINN_PHONG 1
#if USE_BLINN_PHONG
        
        float normalization = ( material.shininess + 8.0 ) / ( 3.14159265 * 8.0 );
        
        vec3 H = normalize( L + E );
        float reflection = normalization * pow( max( 0.0, dot( N, H ) ), material.shininess );
        specular += colorAttenuation * distAttenuation * angularAttenuation * reflection * material.specular;
#else
        float normalization = ( material.shininess + 2.0 ) / ( 3.14159265 * 2.0 );
        
        vec3 R = normalize( -reflect( L, N ) );
        float reflection = normalization * pow( max( 0.0, dot( R, E ) ), material.shininess );
        specular += colorAttenuation * distAttenuation * angularAttenuation * reflection * material.specular;
#endif
    }
    
}



