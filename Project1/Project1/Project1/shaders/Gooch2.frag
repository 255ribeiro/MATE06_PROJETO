#version 440

precision mediump float; 

uniform vec3 uLPos;
uniform vec3 uCamPos;

in vec3 vNormal; 
in vec3 vPosW;

in vec3 vecL;
in vec3 vecV;

void main(void) {

	vec3 vecNormU = normalize(vNormal);
	vec3 vecLU = normalize(vecL);
	vec3 vecVU = normalize(vecV);

	vec3 cSurf 	= vec3(0.3, 1.0, 0.60);
	vec3 cCool  = vec3(0.0, 0.0, 0.55) + 0.25 * cSurf;
	vec3 cWarm  = vec3(0.3, 0.3, 0.00) + 0.25 *cSurf;
	vec3 cHil  = vec3(1.0 ,1.0 ,1.00);

	vec4 lColor		= vec4(1.0, 1.0, 1.0, 1.0); 
	vec4 matAmb		= vec4(0.1, 0.1, 0.1, 1.0); 
	vec4 matDif 	= vec4(1.0, 0.0, 0.6, 1.0); 
	vec4 matSpec	= vec4(1.0, 1.0, 1.0, 1.0);
	
	vec4 ambient = vec4(lColor.rgb * matAmb.rgb, matAmb.a); 

	vec3 vL = normalize(uLPos - vPosW); 
	float cTeta = max(dot(vL, vNormal), 0.0); 

	vec4 diffuse = vec4(lColor.rgb * matDif.rgb * cTeta, matDif.a); 

	vec3 vV = normalize(uCamPos - vPosW); 
	vec3 vR = normalize(reflect(-vL, vNormal)); 
	float cOmega = max(dot(vV, vR), 0.0); 
	vec4 specular = vec4(lColor.rgb * matSpec.rgb * pow(cOmega,20.0), matSpec.a);
	//gl_FragColor = clamp(ambient + diffuse + specular, 0.0, 1.0);

	vec4 teste = vec4(cWarm, 1.0);
	gl_FragColor = clamp(teste, 0.0, 1.0);
	} 	

	//