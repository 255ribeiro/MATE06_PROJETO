#version 330 core 

in vec3 vPosition;
in vec4 vColor;

uniform mat4 uMVP;

out vec4 oColor;

void main() {
	oColor = vColor;

    gl_Position = uMVP * vec4(vPosition, 1.0);
    
}
