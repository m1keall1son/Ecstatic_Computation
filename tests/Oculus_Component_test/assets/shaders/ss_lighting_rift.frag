#version 330

#include "lighting_util.glsl"
#include "rift_util.glsl"

in vec2 vTexCoord0;

uniform sampler2D uData;
uniform sampler2D uAlbedo;
uniform sampler2D uGBufferDepthTexture;

vec3 decodeNormal( in vec2 enc )
{
    vec4 n				= vec4( enc.xy, 0.0, 0.0 ) * vec4( 2.0, 2.0, 0.0, 0.0 ) + vec4( -1.0, -1.0, 1.0, -1.0 );
    float l				= dot( n.xyz, -n.xyw );
    n.z					= l;
    n.xy				*= sqrt( l );
    return n.xyz * 2.0 + vec3( 0.0, 0.0, -1.0 );
}

vec4 getPosition( in vec2 uv, uint eye )
{
    
    mat4 uProjMatrixInverse = inverse( rift.matrices[ 2u * eye + 1u ] );
    float nearClip = rift.nearClip;
    float farClip = rift.farClip;
    vec2 uProjectionParams = vec2( farClip / ( farClip - nearClip ),
                                  ( -farClip * nearClip ) / ( farClip - nearClip ) );

    float depth			= texture( uGBufferDepthTexture, uv ).x;
    float linearDepth 	= uProjectionParams.y / ( depth - uProjectionParams.x );
    vec4 posProj		= vec4( ( uv.x - 0.5 ) * 2.0, ( uv.y - 0.5 ) * 2.0, 0.0, 1.0 );
    vec4 viewRay		= uProjMatrixInverse * posProj;
    return vec4( viewRay.xyz * linearDepth, 1.0 );
}

out vec4 FragColor;

void main(){
    
    
    vec4 data = texture( uData, vTexCoord0 );
    vec3 color = texture( uAlbedo, vTexCoord0 ).rgb;
    
    vec3 normal = normalize(decodeNormal( data.xy ));
    
    int material_id = int( data.z );
    uint eye = uint( data.w );

    vec4 position = getPosition(vTexCoord0, eye);
    
    Atmosphere atmosphere;
    atmosphere.scattering = 0.025;
    
    Material material;
    material.diffuse = vec3( 1. );
    material.specular = vec3( 1. * 0.25 );
    material.shininess = 20.0;
    
    // Initialize ambient, diffuse and specular colors.
    vec3 ambient = vec3( 0 );
    vec3 diffuse = vec3( 0 );
    vec3 specular = vec3( 0 );
    
    calcLighting( material, atmosphere, position, normal, ambient, diffuse, specular );
    
//    vec3 L = vec3( uViewMatrix * vec4(0,3,-10,1.)) - position.xyz; float dist = length( L ); L /= dist;
    //vec3( max(dot( L, normal ),0.) );
    // Output gamma-corrected color.
    FragColor.rgb = sqrt( ambient + diffuse + specular );
    FragColor.a = 1.0;
    
}