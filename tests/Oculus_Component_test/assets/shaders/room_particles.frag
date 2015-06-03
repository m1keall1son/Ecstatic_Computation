#version 330

out vec4 FragColor;
uniform vec3 uColor;

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

void main(){
    
    float alpha = pow( exp(  -pow( length( gl_PointCoord.xy - vec2(.5) ), 2. )), 32. );
    
    FragColor.xyz = uColor;
    FragColor.a = alpha;
    
}