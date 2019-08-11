#version 440
// IN
in vec3 aPosition;
in vec3 aNormal;
in float aC1d;
// UNIFORM 
// matriz u modelo
uniform mat4 uM2;
// matriz u normal
uniform mat4 uN2;
// matriz u de de vista * matriz u de projeção
uniform mat4 uMVP2;

// Posição da luz
uniform vec3 uLPos2;
// posição da camera
uniform vec3 uCamPos2;
uniform sampler1D tex1dID;
// OUT
out vec3 vNormal; 
out vec3 vPosW;

out vec3 vecL;
out vec3 vecV;
out vec4 corMat;

void main(void) { 

// Convertendo posição e normal para coordenadas globais

	vPosW = (uM2 * vec4(aPosition, 1.0)).xyz; 
	vNormal = normalize((uN2 * vec4(aNormal, 1.0)).xyz); 			
	
	gl_Position = uMVP2 * vec4( aPosition, 1.0 );

// vetores de saida

   corMat = texture(tex1dID, aC1d);

	vecL = uLPos2 - aPosition.xyz;

	vecV = uCamPos2 - aPosition.xyz;

	} 

//