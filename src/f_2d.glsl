#version 330


out vec4 pixelColor;

in vec2 i_tc;

uniform sampler2D tex;

void main(void) {
	vec4 color = texture(tex, vec2(i_tc.x, -i_tc.y));
	if (color.r > 0.5f || color.g > 0.5f || color.b > 0.5f)
		discard;

	pixelColor = color;
}
