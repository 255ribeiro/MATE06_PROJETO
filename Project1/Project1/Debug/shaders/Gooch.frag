#version 440

precision mediump float;
// IN
in vec3 vNormal; 
in vec3 vecL;
in vec3 vecV;
in vec4 corMat;


// fun��o gooch
vec4 gooch_shading(vec4 m_color,       // cor do objeto                 
                   vec3 l_direction,   // dire��o da luz
                   vec3 v_normal,      // normal
                   vec3 c_direction,   // dire��o da camera
				   vec3 ambient        // luz ambiente
				   )
{
    // Multiplicadores
    float a = 0.25;
    float b = 0.25;

	// Cool e Warm
    float NL = dot(normalize(v_normal), normalize(l_direction));
    
    float it = ((1 + NL) / 2);

    vec3 color = (1-it) * (vec3(0, 0, 0.55) + a*m_color.xyz) 
                  +  it * (vec3(0.3, 0.3, 0) + b*m_color.xyz);
 
    // Highlights
	
    vec3 R = 2.0 * NL * normalize( v_normal ) - normalize( l_direction );
    vec4 spec =  clamp((100 * dot(normalize( R ),  normalize( v_normal )) -97) * vec4(1.0, 1.0, 1.0, 1.0),0,1);

	// Cor
    return vec4(color +  spec.xyz + ambient, m_color.a);
}

// main
void main(void) {

vec4 vColor4 = gooch_shading(corMat, vecL,  vNormal, vecV,  vec3(0.1, 0.1, 0.1));

gl_FragColor = vColor4;

	} 	

//