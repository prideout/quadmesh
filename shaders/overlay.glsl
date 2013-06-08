-- vert

void main()
{
    gl_Position = vec4(
        mix(vec2(-1.0), vec2(3.0), bvec2(gl_VertexID == 1, gl_VertexID == 2)),
        0.0,
        1.0);
}

-- frag

uniform vec2 InverseViewport;
layout(location = 0) out vec4 FragColor;
uniform sampler2D Sampler;

void main()
{
    float margin = 0.05;
    vec2 shadowOffset = InverseViewport * 2;
    shadowOffset.y *= 2.0;

    vec2 tc = gl_FragCoord.xy * InverseViewport;
    tc.y = 1.0 - tc.y;
    tc.y *= 2.0;
    tc = -0.05 + tc * 1.1;

    vec4 shadow = vec4(0.0, 0.0, 0.0, texture(Sampler, tc).a);
    
    tc += shadowOffset;
    float a = texture(Sampler, tc).a;
    vec4 fill = vec4(1.0, 1.0, 1.0, a);

    if (tc.x < 0.0) discard;

    FragColor = (a == 0.0) ? shadow : fill;
}
