#version 330


uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

uniform vec4 lightDir = vec4(0, 0, 1, 0);

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out vec2 i_tc;
out vec4 l;
out vec4 n;
out vec4 v;

void main(void) {
	i_tc = vec2(texCoord.x, -texCoord.y);

	vec4 lp = vec4(0, 0, -5, 1);
	l = normalize(V*lp - V*M* vec4(vertex.xyz, 1.0) + 0.3);
	n = normalize(V * M * vec4(normal.xyz, 0.0));
	v = normalize(vec4(0, 0, 0, 1) - V*M* vec4(vertex.xyz, 1.0));

    gl_Position=P*V*M* vec4(vertex.xyz, 1.0);
}
