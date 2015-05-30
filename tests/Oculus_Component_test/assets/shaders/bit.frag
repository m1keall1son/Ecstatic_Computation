#version 330

in F_DATA{
    vec4 Position;
    vec3 Normal;
    vec2 TexCoord0;
    float DistanceFromCenter;
}frag;

uniform float ciElapsedSeconds;

#include "lighting_util.glsl"

out vec4 FragColor;

void main(){
    
    Atmosphere atmosphere;
    atmosphere.scattering = 0.025;
    
    Material material;
    material.diffuse = vec3( 1.0 );
    material.specular = vec3( 0.25 );
    material.shininess = 200.0;
    
    vec3 ambient = vec3(0);
    vec3 diffuse = vec3(0);
    vec3 specular = vec3(0);

    calcLighting( material, atmosphere, frag.Position, frag.Normal, ambient, diffuse, specular );
    
    ambient = mix(vec3( 0.,.5,.75 ), ambient, pow(frag.DistanceFromCenter,2.) );
    
    // Output gamma-corrected color.
    FragColor.rgb = sqrt( ambient + diffuse + specular );
    FragColor.a = 1.0;
    
}