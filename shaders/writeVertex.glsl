
-- blend

uniform mat4 Projection;
uniform mat4 Modelview;
uniform float Lerp;

in vec4 Position0;
in vec4 Position1;

out vec4 Feedback;

void main()
{
    vec4 p = mix(Position0, Position1, Lerp);
    Feedback = p;
    gl_Position = Projection * Modelview * p;
}

-- centroid

in ivec4 Quad;
uniform float Lerp;

out vec4 Feedback;

uniform samplerBuffer Frame0;
uniform samplerBuffer Frame1;

void main()
{
    vec3 a = texelFetch(Frame0, Quad[0]).xyz;
    vec3 b = texelFetch(Frame0, Quad[1]).xyz;
    vec3 c = texelFetch(Frame0, Quad[2]).xyz;
    vec3 d = texelFetch(Frame0, Quad[3]).xyz;
    vec3 c0 = 0.25 * (a + b + c + d);

    a = texelFetch(Frame1, Quad[0]).xyz;
    b = texelFetch(Frame1, Quad[1]).xyz;
    c = texelFetch(Frame1, Quad[2]).xyz;
    d = texelFetch(Frame1, Quad[3]).xyz;
    vec3 c1 = 0.25 * (a + b + c + d);

    Feedback = vec4(mix(c0, c1, Lerp), 1.0);
}

-- facetNormal

in ivec4 Quad;
uniform float Lerp;

out vec4 Feedback;

uniform samplerBuffer Frame0;
uniform samplerBuffer Frame1;

void main()
{
    vec3 a0 = texelFetch(Frame0, Quad[0]).xyz;
    vec3 b0 = texelFetch(Frame0, Quad[1]).xyz;
    vec3 c0 = texelFetch(Frame0, Quad[2]).xyz;

    vec3 a1 = texelFetch(Frame1, Quad[0]).xyz;
    vec3 b1 = texelFetch(Frame1, Quad[1]).xyz;
    vec3 c1 = texelFetch(Frame1, Quad[2]).xyz;

    vec3 a = mix(a0, a1, Lerp);
    vec3 b = mix(b0, b1, Lerp);
    vec3 c = mix(c0, c1, Lerp);

    Feedback = vec4(normalize(cross(b - a, c - a)), 1.0);
}
