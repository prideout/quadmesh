
-- v1

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

-- g1

uniform mat4 Projection;
uniform mat4 Modelview;
uniform mat3 NormalMatrix;

in vec4 vPosition[4];
out vec3 gNormal;

layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 4) out;

void emit(int index)
{
    gl_Position = Projection * Modelview * vPosition[index];
    EmitVertex(); 
}

void main()
{
    vec3 A = vPosition[0].xyz;
    vec3 B = vPosition[1].xyz;
    vec3 C = vPosition[2].xyz;
    gNormal = NormalMatrix * normalize(cross(B - A, C - A));

    emit(0); emit(1); emit(3); emit(2);
    EndPrimitive();
}

-- v2

uniform mat4 Projection;
uniform mat4 Modelview;
uniform float Lerp;

in vec4 Position0;
in vec4 Position1;

out vec4 vPosition;
out vec3 vNormal;
out int vCrease;

uniform samplerBuffer FacetNormals;
uniform usamplerBuffer InverseTopo;

void main()
{
    vCrease = 0;
    uvec4 faces = texelFetch(InverseTopo, int(gl_VertexID));
    vec3 n = vec3(0);
    vec3 previous = vec3(1);
    for (int c = 0; c < 4; c++) {
        if (faces[c] != 0xffffu) {
            int quad = int(faces[c]);
            vec3 facetNormal = texelFetch(FacetNormals, quad).xyz;
            previous = facetNormal;
            n += facetNormal;
        }
    }
    n = normalize(n);

    vec4 p = mix(Position0, Position1, Lerp);
    vPosition = p;
    vNormal = n;
    gl_Position = Projection * Modelview * p;
}

-- g2

uniform mat4 Projection;
uniform mat4 Modelview;
uniform mat3 NormalMatrix;

uniform samplerBuffer FacetNormals;

in float vId[4];
in vec4 vPosition[4];
in vec3 vNormal[4];
in int vCrease[4];

out vec3 gNormal;

bool smoothn = true;

layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 4) out;

void emit(int index)
{
    if (smoothn) {
        gNormal = NormalMatrix * vNormal[index];
    }
    if (vCrease[index] == 1) {
        gNormal = vec3(0);
    }
    gl_Position = Projection * Modelview * vPosition[index];
    EmitVertex(); 
}

void main()
{
    if (true || !smoothn) {
        int i = gl_PrimitiveIDIn;    
        vec3 n = texelFetch(FacetNormals, i).xyz;
        gNormal = NormalMatrix * n;
    }
    emit(0); emit(1); emit(3); emit(2);
    EndPrimitive();
}


-- frag

uniform mat4 Projection;
uniform mat4 Modelview;
uniform mat3 NormalMatrix;

in vec3 gNormal;
out vec4 FragColor;

uniform vec4 LightPosition = vec4(0.75, -0.25, 1, 1);
uniform vec3 AmbientMaterial = vec3(0.2, 0.1, 0.1);
uniform vec4 DiffuseMaterial = vec4(1.0, 209.0/255.0, 54.0/255.0, 1.0);
uniform vec3 SpecularMaterial = vec3(0.4, 0.4, 0.3);
uniform float Shininess = 200.0;
uniform float Fresnel = 0.1;

void main()
{
    vec3 N = normalize(gNormal);
    vec3 L = normalize((LightPosition).xyz);

    vec3 Eye = vec3(0, 0, 1);
    vec3 H = normalize(L + Eye);

    float df = max(0.0, dot(N, L));
    float sf = pow(max(0.0, dot(N, H)), Shininess);
    float rfTheta = Fresnel + (1-Fresnel) * pow(1-dot(N,Eye), 5);

    vec3 color = AmbientMaterial +
        df * DiffuseMaterial.rgb +
        sf * SpecularMaterial +
        rfTheta;

    FragColor = vec4(color, DiffuseMaterial.a);
}
