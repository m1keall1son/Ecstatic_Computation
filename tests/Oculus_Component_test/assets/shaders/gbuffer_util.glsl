vec2 encodeNormal( vec3 n )
{
    vec2 enc	= normalize( n.xy ) * ( sqrt( -n.z * 0.5 + 0.5 ) );
    enc			= enc * 0.5 + 0.5;
    return enc;
}

