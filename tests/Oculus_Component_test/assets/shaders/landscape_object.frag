#version 330

#include "lighting_util.glsl"

in vec3 vNormal;
in vec4 vPosition;
in vec2 vTexCoord0;

out vec4 FragColor;

#include "linear_map.glsl"

uniform float uFogDensity;

void main(void)
{
    Atmosphere atmosphere;
    atmosphere.scattering = 0.025;
    atmosphere.fog.color = vec3(1.);
    atmosphere.fog.density = uFogDensity;
    
    Material material;
    material.diffuse = vec3( 1. );
    material.specular = vec3( material.diffuse * 0.25 );
    material.shininess = 2.0;
    material.ambient = vec3( .01 );
    
    // Initialize ambient, diffuse and specular colors.
    vec3 ambient = vec3( 0 );
    vec3 diffuse = vec3( 0 );
    vec3 specular = vec3( 0 );
    
    calcLighting( material, atmosphere, vPosition, normalize(vNormal), ambient, diffuse, specular );
    
    // Output gamma-corrected color.
    FragColor.rgb = sqrt( ambient + diffuse + specular );
    FragColor.a = 1.0;
    
    const float LOG2 = 1.442695;
    float fogCoord = abs( gl_FragCoord.z / gl_FragCoord.w );
    FragColor.xyz = mix( mix( vec3(.01), atmosphere.fog.color, clamp(map(vPosition.y,40.,-1000.,1.,0. ), 0.,1. )), FragColor.xyz, max(min( exp2( -atmosphere.fog.density * atmosphere.fog.density * fogCoord * fogCoord * LOG2 ) ,1.),0.) );
}