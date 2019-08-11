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
// matriz u de de vista * matriz u de proje��o
uniform mat4 uMVP2;
// Posi��o da luz
uniform vec3 uLPos2;
// posi��o da camera
uniform vec3 uCamPos2;
uniform sampler1D tex1dID;

// OUT
out vec3 vNormal; 
out vec3 vPosW;
out vec3 vecL;
out vec3 vecV;
out vec4 corMat;


// main
void main(void) { 

	
	// Convertendo posi��o e normal para coordenadas homogeneas

	vPosW = (uM2 * vec4(aPosition, 1.0)).xyz; 
	vNormal = normalize((uN2 * vec4(aNormal, 1.0)).xyz); 	
	
	gl_Position = uMVP2 * vec4( aPosition, 1.0 );

// vetores de saida

	// lendo cor da textura
	corMat = texture(tex1dID, aC1d);
	// vetor partindo do v�rtice em dire��o aa fonte de luz
	vecL = uLPos2 - aPosition.xyz;
	// vetor partindo do v�rtice em dire��o aa camera
	vecV = uCamPos2 - aPosition.xyz;

	} 

//