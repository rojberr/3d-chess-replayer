#version 330


uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

uniform vec4 lightDir=vec4(0, 0, 1, 0);


layout (location=0) in vec3 vertex;
layout (location=1) in vec3 normal;
layout (location=2) in vec2 texCoord;


out vec2 i_tc;
out float i_nl;

void main(void) {
    i_tc = vec2(texCoord.x, -texCoord.y);

    gl_Position  = P * V * M * vec4(vertex.xyz, 1.0);

    mat4 G = mat4(inverse(transpose(mat3(M))));
    vec4 n = normalize(V * G * vec4(normal.xyz, 0.0));

    i_nl = clamp(dot(n, lightDir), 0, 1);
}
