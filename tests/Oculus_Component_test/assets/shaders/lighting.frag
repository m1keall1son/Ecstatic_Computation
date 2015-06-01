#version 330

#include "lighting_util.glsl"

in vec4 vertPosition;
in vec3 vertNormal;

out vec4 fragColor;

void main(void)
{
    Atmosphere atmosphere;
    atmosphere.scattering = 0.025;
    
    Material material;
    material.diffuse = vec3( 1.0 );
    material.specular = vec3( 0.25 );
    material.shininess = 200.0;

	// Initialize ambient, diffuse and specular colors.
	vec3 ambient = vec3( 0 );
	vec3 diffuse = vec3( 0 );
	vec3 specular = vec3( 0 );

    calcLighting( material, atmosphere, vertPosition, vertNormal, ambient, diffuse, specular );
    
	// Output gamma-corrected color.
	fragColor.rgb = sqrt( ambient + diffuse + specular );
	fragColor.a = 1.0;
}