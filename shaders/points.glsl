
-- vert

uniform mat4 Projection;
uniform mat4 Modelview;

in vec3 Position;

void main()
{
    gl_Position = Projection * Modelview * vec4(Position, 1);
}

-- frag

out vec4 Color;

void main()
{
    Color = vec4(1, 1, 1, 1);
}
