#version 330


layout (location=0) in vec2 vertex;
layout (location=1) in vec2 texCoord;
uniform float width;
uniform float height;

out vec2 i_tc;

void main(void) {
    i_tc = texCoord;
    gl_Position  = vec4((vertex.x/width*2)-1, (vertex.y/height*2)-1, 0.0, 1.0);
}
