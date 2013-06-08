# Title

Quad Meshes and Blend Shapes

# Summary

Efficient rendering of animated quad meshes with OpenGL and computing smooth normals on the GPU with transform feedback.

# Description

When working with Catmull-Clark subdivision surfaces in the absence of tessellation shaders, you'll typically perform some sort of uniform refinement on the CPU, perhaps using [OpenSubdiv](http://graphics.pixar.com/opensubdiv/index.html).

One of the nice properties of Catmull-Clark is that you always get a clean quad mesh after one level of subdivision, even though the input topology can be composed of arbitrary polygons.

Subdivision surfaces aren't the only source of quad meshes nowadays; they can also crop up with terrain rendering and visualization of parametric functions.

## Vertex Submission for Quad Meshes

Ideally, you could send an indexed quad mesh to OpenGL, but QUADS aren't in Modern OpenGL.  You could use indexed TRIANGLES, but that's less than ideal because of the repeated indices.

Fret not, it's easy enough to emulate indexed QUADS using a LINE_ADJACENCY trick:

    glDrawElements(GL_LINES_ADJACENCY, numQuads * 4, GL_UNSIGNED_SHORT, 0);

To make this work, use a geometry shader that generates a small triangle strip for every four vertices, like so:

    in vec4 vPosition[4];

    layout(lines_adjacency) in;
    layout(triangle_strip, max_vertices = 4) out;

    void emit(int index)
    {
        gl_Position = ObjectToClip * vPosition[index];
        EmitVertex(); 
    }

    void main()
    {
        emit(0); emit(1); emit(3); emit(2);
        EndPrimitive();
    }
    
While you're at it, you can also compute facet normals in the geometry shader:

    out vec3 gNormal;
    ...
    void main()
    {
        vec3 A = vPosition[0].xyz;
        vec3 B = vPosition[1].xyz;
        vec3 C = vPosition[2].xyz;
        gNormal = InverseTranspose * normalize(cross(B - A, C - A));
        ...
    }

## Computing Smooth Normals on the GPU

If the faceted look isn't what you need, computing smooth normals is also possible in OpenGL with a little more work.  In the first pass, compute facet normals in the geometry shader and dump them into a transform feedback buffer:

        glEnable(GL_RASTERIZER_DISCARD);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buffers.facetNormals);
        glBeginTransformFeedback(GL_POINTS);
        glDrawArrays(GL_POINTS, 0, numQuads);
        glEndTransformFeedback();
        glDisable(GL_RASTERIZER_DISCARD);

For the second pass, bind the facet normals as texture buffer, along with a vertex-to-face lookup table:

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_BUFFER, textures.vertexToFace);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_BUFFER, textures.facetNormals);

The vertex-to-face lookup table is easy to build on the CPU; it's just the inverse of the usual index buffer.  The vertex shader in your second pass can use the *vertexToFace* mapping to find adjoining faces and sum up their facet normals:

    uniform samplerBuffer FacetNormals;
    uniform usamplerBuffer VertexToFace;
    
    void main()
    {
        vCrease = 0;
        uvec4 faces = texelFetch(VertexToFace, int(gl_VertexID));
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
        ...

Note that naive smoothing like this can result in undesirable lighting near sharp creases:

[[image]]

This could be improved by creating a buffer with sharpness information, similar to the sharpness tags that you can give to OpenSubdiv.  The vertex shader could use this give weights to the facets that iterates over.

# Building

To build the demo on github, just type `make -j`.  But first, you might need to install some of the following packages with `apt-get`:

- build-essential
- libmotif-dev
- libgl1-mesa-dev
- libglm-dev
- libpng-dev
