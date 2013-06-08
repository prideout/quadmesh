
-- vert

uniform mat4 Projection;
uniform mat4 Modelview;
uniform float Lerp;

in vec4 Position0;
in vec4 Position1;

out vec4 vPosition;

void main()
{
    vec4 p = mix(Position0, Position1, Lerp);
    vPosition = p;
    gl_Position = Projection * Modelview * p;
}

-- geom

uniform mat4 Projection;
uniform mat4 Modelview;

in vec4 vPosition[4];

layout(lines_adjacency) in;
layout(line_strip, max_vertices = 5) out;

void emit(int index)
{
    gl_Position = Projection * Modelview * vPosition[index];
    EmitVertex(); 
}

void main()
{
    emit(0);
    emit(1);
    emit(2);
    emit(3);
    emit(0);
}

-- frag

out vec4 Color;
uniform vec4 WireframeColor;

void main()
{
    Color = WireframeColor;
}
