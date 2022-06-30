#version 330


uniform mat4 P;
uniform mat4 V;
uniform mat4 M;


layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;



out vec4 l1;
out vec4 l2;
out vec4 v;
out vec4 n;

out vec2 iTexCoord;

void main(void) {

    vec4 lp1 = vec4(0, 0, 8, 1);
    vec4 lp2 = vec4(0, 4, 0, 1);

    l1 = normalize(V * lp1 - V*M*vec4(vertex.xyz, 1));
    l2 = normalize(V * lp2 - V*M*vec4(vertex.xyz, 1));
    v = normalize(vec4(0, 0, 0, 1) - V * M * vec4(vertex.xyz, 1));
    n = normalize(V * M * vec4(normal.xyz, 0));
    iTexCoord = texCoord;
    gl_Position=P*V*M*vec4(vertex.xyz, 1);

}
