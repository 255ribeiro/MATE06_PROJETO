#version 330 core 

in vec3 vPosition;
in vec4 vColor;

out vec4 oColor;

void main() {
	oColor = vColor;
    gl_Position = vec4(vPosition, 1.0);
    
}
