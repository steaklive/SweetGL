#version 430 core
// can be used for HDR

in vec2 TextureCoordinates;

uniform sampler2D colorBuffer;

out vec4 fragColor;


void main() {
	vec3 color = texture(colorBuffer, TextureCoordinates).rgb;
	fragColor = vec4(color, 1.0);
}