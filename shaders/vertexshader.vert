attribute vec4 attr_pos;

varying vec2 uv;

void main()
{
    gl_Position = attr_pos;
    uv = attr_pos.xy;
}
