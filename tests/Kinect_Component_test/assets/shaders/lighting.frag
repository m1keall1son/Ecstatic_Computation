#version 150

#include "lighting_util.glsl"

in vec4 vertPosition;
in vec3 vertNormal;

out vec4 fragColor;

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