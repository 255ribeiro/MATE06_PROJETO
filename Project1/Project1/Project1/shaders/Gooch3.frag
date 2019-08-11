#version 440

precision mediump float;

uniform vec3 uLPos;
uniform vec3 uCamPos;

in vec3 vNormal; 
in vec3 vPosW;

in vec3 vecL;
in vec3 vecV;

void main(void) {

// vetores normalizados

vec3 vecNormU = normalize(vNormal.xyz);
vec3 vecLU = normalize(vecL.xyz);
vec3 vecVU = normalize(vecV.xyz);

// Cores
 	
vec3 cSurf 	= vec3(0.2,0.2, 0.60); 	
vec3 cCool  = vec3(0.0, 0.0, 0.55) + 0.25 * cSurf;
vec3 cWarm  = vec3(0.3, 0.3, 0.00) + 0.25 *cSurf;
vec3 cHil  = vec3(1.0 ,1.0 ,1.00);

// parametros da equaçãao de Gooch

float tGoo = ( dot( vecNormU, vecLU ) + 1.0) / 2.0;

vec3 rGoo = 2.0 * dot( vecNormU, vecLU ) * vecNormU - vecLU;

float sGoo =  abs(100.0 * dot( rGoo, vecVU ) - 97.0); 

// equação de Gooch

vec3 vColor3 =  clamp( sGoo * cHil + (1.0 - sGoo) * (tGoo * cWarm + (1.0 - tGoo ) * cCool), 0.0, 1.0);

// transformando de vec3 para vec4

vec4 vColor4 = vec4(vColor3, 1.0);

gl_FragColor =  vColor4;
	} 	

//