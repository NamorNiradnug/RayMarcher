#version 440

in vec4 attr_pos;
out vec2 uv;

void main()
{
    gl_Position = attr_pos;
    uv = attr_pos.xy;
}
