#version 440
// in
in vec3 aPosition;
in vec3 aNormal;
in vec4 aColor;
in vec2 aCoords;
// uniform
uniform mat4 uM1;
uniform mat4 uN1;
uniform mat4 uMVP1;
uniform sampler2D textureID;
uniform int utexLoad;

//out
out vec3 vNormal; 
out vec3 vPosW;
out vec4 vColor;

//main
void main(void) { 		
	vPosW = (uM1 * vec4(aPosition, 1.0)).xyz; 
	vNormal = normalize((uN1 * vec4(aNormal, 1.0)).xyz);			
	
	gl_Position = uMVP1 * vec4( aPosition, 1.0 );

	
	if (utexLoad > 0){
	vColor = texture(textureID, aCoords);
	}
		else {
		vColor = aColor;
		}
	} 


//