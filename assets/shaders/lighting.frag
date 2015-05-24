#version 150

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
//uniform sampler2D       uModulationMap[8];
//uniform vec4            uSkyDirection = vec4(0,1,0,0);

in vec4 vertPosition;
in vec3 vertNormal;

out vec4 fragColor;

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

void main(void)
{
	const float kPi = 3.14159265;
	const vec3  kMaterialDiffuseColor = vec3( 1 );
	const vec3  kMaterialSpecularColor = vec3( 0.25 );
	const float kMaterialShininess = 200.0;
	const float kAtmosphericScattering = 0.025;

	// Initialize ambient, diffuse and specular colors.
	vec3 ambient = vec3( 0 );
	vec3 diffuse = vec3( 0 );
	vec3 specular = vec3( 0 );

	// Calculate normal and eye vector.
	vec3 N = normalize( vertNormal );
	vec3 E = normalize( -vertPosition.xyz );

	// Hemispherical ambient lighting.
	float hemi = 0.5 + 0.5 * dot( lights.upDirection.xyz, N );
	ambient = mix( vec3( 0 ), vec3( 0.01 ), hemi ) * kMaterialDiffuseColor;

	// Calculate lighting.
	for( int i=0; i<lights.numLights; ++i )
	{
		// Fetch light type.
		int flags = lights.uLight[i].flags;
        int type =  0x4;//flags & 0xF;

        bool isDirectional = ( type == 0 );
        bool isSpotOrPoint = ( type & 0x5 ) > 0;
        bool isSpotOrWedge = ( type & 0xC ) > 0;
        bool isLinearLight = ( type & 0xA ) > 0;

        bool hasShadows = ( flags & 0x20 ) > 0;
        bool hasModulation = ( flags & 0x10 ) > 0;

		// Calculate end-points of the light for convenience.
		vec3 lightStart = lights.uLight[i].position;
		vec3 lightEnd   = lights.uLight[i].position + lights.uLight[i].width * lights.uLight[i].horizontal;
		vec3 lightColor = lights.uLight[i].color.rgb * lights.uLight[i].intensity;

		// Calculate direction and dist to light.
		float dist = 1.0;
		vec3  L = -lights.uLight[i].direction;

		if( !isDirectional ) 
		{
			float t = clamp( dot( vertPosition.xyz - lightStart, lights.uLight[i].horizontal ), 0.0, lights.uLight[i].width );
			vec3 lightPosition = lightStart + t * lights.uLight[i].horizontal;
			L = lightPosition - vertPosition.xyz; dist = length( L ); L /= dist;
		}

		// Calculate shadow.
		float shadow = 1.0;
		if( hasShadows )
		{
            //[ lights.uLight[i].shadowIndex ]
			vec4 shadowCoord = lights.uLight[i].shadowMatrix * vertPosition;
			shadow = calcShadow( uShadowMap, shadowCoord );
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
		float angularAttenuation = mix( 1.0, calcAngularAttenuation( L, lights.uLight[i].direction, lights.uLight[i].angle ), isSpotOrWedge );
		float distAttenuation = mix( 1.0, calcdistAttenuation( dist, lights.uLight[i].attenuation ), !isDirectional );
		vec3  colorAttenuation = modulation * lightColor;

		// Calculate diffuse color (clamp it to the light's range).
		float lambert = max( 0.0, dot( N, L ) );
		float range = mix( 1.0, step( dist, lights.uLight[i].range ), !isDirectional );
		diffuse += shadow * range * colorAttenuation * distAttenuation * angularAttenuation * lambert * kMaterialDiffuseColor;

		// Calculate light Scattering.
		if( !isDirectional )
		{
			vec3 lightPosition = calcRepresentativePoint( lightStart, lightEnd, vertPosition.xyz, -E );
            float scatter = calcScattering( vertPosition.xyz, lightPosition );
			diffuse += kAtmosphericScattering * scatter * lightColor;
		}

		// Calculate representative light vector (for linear lights only). TODO: works for standard Phong only, Blinn-Phong seems wrong.
		if( isLinearLight ) {
			vec3 lightPosition = calcRepresentativePoint( lightStart, lightEnd, vertPosition.xyz, -reflect( E, N ) );
			L = lightPosition - vertPosition.xyz; dist = length( L ); L /= dist;

			distAttenuation = calcdistAttenuation( dist, lights.uLight[i].attenuation );
		} 

		// Calculate specular color.
#define USE_BLINN_PHONG 1
#if USE_BLINN_PHONG
		const float normalization = ( kMaterialShininess + 8.0 ) / ( 3.14159265 * 8.0 );		
		
		vec3 H = normalize( L + E );
		float reflection = normalization * pow( max( 0.0, dot( N, H ) ), kMaterialShininess );
		specular += colorAttenuation * distAttenuation * angularAttenuation * reflection * kMaterialSpecularColor;
#else
		const float normalization = ( kMaterialShininess + 2.0 ) / ( 3.14159265 * 2.0 );

		vec3 R = normalize( -reflect( L, N ) );
		float reflection = normalization * pow( max( 0.0, dot( R, E ) ), kMaterialShininess );
		specular += colorAttenuation * distAttenuation * angularAttenuation * reflection * kMaterialSpecularColor;
#endif
	}

	// Output gamma-corrected color.
	fragColor.rgb = sqrt( ambient + diffuse + specular );
	fragColor.a = 1.0;
}