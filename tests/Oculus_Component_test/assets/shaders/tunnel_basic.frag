#version 330

#include "lighting_util.glsl"
#include "simplex_noise.glsl"

uniform float ciElapsedSeconds;

in F_DATA{
    vec3 Normal;
    vec4 Position;
    vec2 TexCoord0;
//    vec3 Color;
}frag;

//uniform int uEye;

out vec4 FragColor;

void main(void)
{
    Atmosphere atmosphere;
    atmosphere.scattering = 0.025;
    
    Material material;
    material.diffuse = vec3( max(random( frag.TexCoord0.x ),.65) );
    material.specular = material.diffuse*0.25;
    material.shininess = 20.0;
    material.ambient = vec3( .001 );

	// Initialize ambient, diffuse and specular colors.
	vec3 ambient = vec3( 0 );
	vec3 diffuse = vec3( 0 );
	vec3 specular = vec3( 0 );

    calcLighting( material, atmosphere, frag.Position, normalize(-frag.Normal), ambient, diffuse, specular );
    
	// Output gamma-corrected color.
    FragColor.rgb = sqrt( ambient + diffuse + specular );
	FragColor.a = 1.0;
}