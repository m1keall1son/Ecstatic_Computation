#version 330

#include "lighting_util.glsl"

in F_DATA{
    vec3 Normal;
    vec4 Position;
    vec2 TexCoord0;
    float Dist;
}frag;

uniform sampler2D uColorMap;

out vec4 FragColor;

void main()
{
    
    vec3 kinect_color = texture( uColorMap, frag.TexCoord0 ).rgb;
    
    Atmosphere atmosphere;
    atmosphere.scattering = 0.025;
    
    Material material;
    material.diffuse = kinect_color;
    material.specular = kinect_color*.25;
    material.shininess = 200.0;
    
    // Initialize ambient, diffuse and specular colors.
    vec3 ambient = vec3( 0 );
    vec3 diffuse = vec3( 0 );
    vec3 specular = vec3( 0 );
    
    // Calculate normal and eye vector.
    vec3 normal = frag.Normal;
    
    if(!gl_FrontFacing){
        material.diffuse = vec3(1.);
        material.specular = vec3(.25);
        normal = -normal;
    }
    
    calcLighting( material, atmosphere, frag.Position, normal, ambient, diffuse, specular );
        
    // Output gamma-corrected color.
    FragColor.rgb = sqrt( ambient + diffuse + specular );
    FragColor.a = 1.0;
    
}