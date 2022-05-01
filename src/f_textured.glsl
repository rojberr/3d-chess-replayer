#version 330


out vec4 pixelColor;

in vec2 i_tc;
in float i_nl;

uniform sampler2D tex;


void main(void) {
	vec4 color = texture(tex, i_tc);
	pixelColor = vec4(color.rgb * i_nl, color.a);
}
