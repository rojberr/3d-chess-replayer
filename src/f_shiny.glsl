#version 330


out vec4 pixelColor; //Zmienna wyjsciowa fragment shadera. Zapisuje sie do niej ostateczny (prawie) kolor piksela
in vec4 l;
in vec4 n;
in vec4 v;

in vec2 i_tc;
uniform sampler2D tex;

void main(void) {
	vec4 color = texture(tex, i_tc);

	vec4 r = reflect(-l, n);
	float nl = clamp(dot(n, l), 0, 1);
	float rv = pow(clamp(dot(r, v), 0, 1), 25);
	vec4 ks = vec4(1, 1, 1, 1);

	pixelColor= color * nl + ks * rv;
}
