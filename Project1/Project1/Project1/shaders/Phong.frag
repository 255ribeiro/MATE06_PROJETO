#version 440

precision mediump float; 

uniform vec3 uLPos1;
uniform vec3 uCamPos1;




in vec4 vColor;
in vec3 vNormal; 
in vec3 vPosW;

void main(void) {
	vec4 lColor		= vec4(1.0, 1.0, 1.0, 1.0); 
	vec4 matAmb		= vec4(0.1, 0.1, 0.1, 1.0);
	
	vec4 matDif 	= vColor; 
	vec4 matSpec	= vec4(1.0, 1.0, 1.0, 1.0);

	vec4 ambient = vec4(lColor.rgb * matAmb.rgb, matAmb.a); 

	vec3 vL = normalize(uLPos1 - vPosW); 
	float cTeta = max(dot(vL, vNormal), 0.0); 
			
	vec4 diffuse = vec4(lColor.rgb * matDif.rgb * cTeta, matDif.a);
	//vec4 diffuse = vColor; 

	vec3 vV = normalize(uCamPos1 - vPosW); 
	vec3 vR = normalize(reflect(-vL, vNormal)); 
	float cOmega = max(dot(vV, vR), 0.0); 
	vec4 specular = vec4(lColor.rgb * matSpec.rgb * pow(cOmega,20.0), matSpec.a); 
	
	gl_FragColor = clamp(ambient + diffuse + specular, 0.0, 1.0); 
	} 	

	//