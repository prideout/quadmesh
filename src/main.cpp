#include <iostream>
#include "common.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

using namespace std;
using namespace glm;

static struct {
    GLuint wireframe;
    GLuint onePassFacets;
    GLuint blend;
    GLuint pointCloud;
    GLuint centroids;
    GLuint genFacetNormals;
    GLuint useFacetNormals;
    GLuint overlay;
} programs;

static struct {
    GLuint frames[2];
    GLuint topology;
    GLuint inverseTopology;
    GLuint blendVerts;
    GLuint centroids;
    GLuint facetNormals;
} buffers;

static struct {
    GLuint framePair;
    GLuint blended;
    GLuint inverseMapping;
    GLuint centroids;
} arrays;

static struct {
    GLuint frame0;
    GLuint frame1;
    GLuint facetNormals;
    GLuint inverseTopology;
    GLuint overlays[5];
} textures;

size_t numQuads;
size_t numVerts;
float worldTime = 0;

void PezInitialize()
{
    pezCheckGL("startup");

    // Create program objects
    // ----------------------

    pezSwAddPath("./shaders/", ".glsl");

    programs.wireframe = loadProgram(
        "wireframe.vert",
        "wireframe.geom",
        "wireframe.frag");

    programs.onePassFacets = loadProgram(
        "facets.v1",
        "facets.g1",
        "facets.frag");

    programs.useFacetNormals = loadProgram(
        "facets.v2",
        "facets.g2",
        "facets.frag");

    programs.blend = loadProgram("writeVertex.blend", "", "");
    programs.centroids = loadProgram("writeVertex.centroid", "", "");
    programs.genFacetNormals = loadProgram("writeVertex.facetNormal", "", "");
    programs.pointCloud = loadProgram("points.vert", "", "points.frag");
    programs.overlay = loadProgram("overlay.vert", "", "overlay.frag");
    pezCheckGL("creation of program objects");

    // Create buffer objects
    // ---------------------

    size_t byteCount;
    buffers.frames[0] = loadBuffer(GL_ARRAY_BUFFER, "gizmo.0.positions",
                                   true, &byteCount);
    buffers.frames[1] = loadBuffer(GL_ARRAY_BUFFER, "gizmo.1.positions", true);
    numVerts = byteCount / 16;

    glGenBuffers(1, &buffers.blendVerts);
    glBindBuffer(GL_ARRAY_BUFFER, buffers.blendVerts);
    glBufferData(GL_ARRAY_BUFFER, numVerts * 16, 0, GL_STATIC_DRAW);

    buffers.topology = loadBuffer(
        GL_ELEMENT_ARRAY_BUFFER, "gizmo.quads", false, &byteCount,
        &buffers.inverseTopology);
    numQuads = (byteCount / 2) / 4;

    glGenBuffers(1, &buffers.centroids);
    glBindBuffer(GL_ARRAY_BUFFER, buffers.centroids);
    glBufferData(GL_ARRAY_BUFFER, numQuads * 16, 0, GL_STATIC_DRAW);

    glGenBuffers(1, &buffers.facetNormals);
    glBindBuffer(GL_ARRAY_BUFFER, buffers.facetNormals);
    glBufferData(GL_ARRAY_BUFFER, numQuads * 16, 0, GL_STATIC_DRAW);
    pezCheckGL("creation of buffer objects");

    // Create texture objects
    // ----------------------

    glGenTextures(1, &textures.frame0);
    glBindTexture(GL_TEXTURE_BUFFER, textures.frame0);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, buffers.frames[0]);

    glGenTextures(1, &textures.frame1);
    glBindTexture(GL_TEXTURE_BUFFER, textures.frame1);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, buffers.frames[1]);

    glGenTextures(1, &textures.facetNormals);
    glBindTexture(GL_TEXTURE_BUFFER, textures.facetNormals);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, buffers.facetNormals);

    glGenTextures(1, &textures.inverseTopology);
    glBindTexture(GL_TEXTURE_BUFFER, textures.inverseTopology);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA16UI, buffers.inverseTopology);

    textures.overlays[0] = loadTexture("data/captured1.png");
    textures.overlays[1] = loadTexture("data/captured2.png");
    textures.overlays[2] = loadTexture("data/captured3.png");
    textures.overlays[3] = loadTexture("data/captured4.png");
    textures.overlays[4] = loadTexture("data/captured5.png");

    glBindTexture(GL_TEXTURE_BUFFER, 0);
    pezCheckGL("creation of texture objects");

    // Create array objects
    // --------------------

    glGenVertexArrays(1, &arrays.framePair);
    glBindVertexArray(arrays.framePair);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.topology);
    glBindBuffer(GL_ARRAY_BUFFER, buffers.frames[0]);
    glVertexAttribPointer(Attr::Position0, 3, GL_FLOAT, GL_FALSE, 16, 0);
    glEnableVertexAttribArray(Attr::Position0);
    glBindBuffer(GL_ARRAY_BUFFER, buffers.frames[1]);
    glVertexAttribPointer(Attr::Position1, 3, GL_FLOAT, GL_FALSE, 16, 0);
    glEnableVertexAttribArray(Attr::Position1);

    glGenVertexArrays(1, &arrays.blended);
    glBindVertexArray(arrays.blended);
    glBindBuffer(GL_ARRAY_BUFFER, buffers.blendVerts);
    glVertexAttribPointer(Attr::Position, 3, GL_FLOAT, GL_FALSE, 16, 0);
    glEnableVertexAttribArray(Attr::Position);

    glGenVertexArrays(1, &arrays.centroids);
    glBindVertexArray(arrays.centroids);
    glBindBuffer(GL_ARRAY_BUFFER, buffers.centroids);
    glVertexAttribPointer(Attr::Position, 3, GL_FLOAT, GL_FALSE, 16, 0);
    glEnableVertexAttribArray(Attr::Position);

    glGenVertexArrays(1, &arrays.inverseMapping);
    glBindVertexArray(arrays.inverseMapping);
    glBindBuffer(GL_ARRAY_BUFFER, buffers.topology);
    glVertexAttribIPointer(Attr::Position, 4, GL_UNSIGNED_SHORT, 0, 0);
    glEnableVertexAttribArray(Attr::Position);
    pezCheckGL("creation of array objects");

    // Misc Initialization
    // -------------------

    glClearColor(0, 0.25, 0.5, 1);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

PezConfig PezGetConfig()
{
    PezConfig config;
    config.Title = "quadmesh";
    config.Fullscreen = false;
    config.Width = 600;
    config.Height = 500;
    config.Multisampling = true;
    config.VerticalSync = true;
    return config;
}

void PezHandleMouse(int x, int y, int action)
{
    if (action == PEZ_DOWN) {
    } else if (action == PEZ_UP) {
    }
}

void PezRender()
{
    PezConfig config = PezGetConfig();
    float aspect = float(config.Width) / config.Height;
    float fovy = 15;
    float near = 5;
    float far = 200;
    mat4 projection = glm::perspective(fovy, aspect, near, far);

    vec3 eye(0, 0, 10);
    vec3 target(0, 0, 0);
    vec3 up(0, 1, 0);
    mat4 view = glm::lookAt(eye, target, up);

    float theta = worldTime * 12;
    mat4 model = glm::rotate(mat4(1), theta, vec3(0,1,0));
    mat4 modelView = view * model;
    mat3 normalMatrix = mat3(modelView); // <-- assumes uniform scale
    float lerp = 0.5 + 1.5 * sin(worldTime * 4);

    GLuint program = 0;
    float cycleDuration = 7.0f; // <-- in seconds!
    int numCycles = 5;
    int cycle = int(fmod(worldTime / cycleDuration, numCycles));
    switch (cycle) {
    case 0: program = programs.wireframe; break;
    case 1: program = programs.onePassFacets; break;
    case 2: program = programs.blend; break;
    case 3: program = programs.centroids; break;
    case 4: program = programs.genFacetNormals; break;
    }

    glUseProgram(program);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, config.Width, config.Height);

    glUseProgram(program);
    glUniformMatrix4fv(u("Projection"), 1, 0, ptr(projection));
    glUniformMatrix4fv(u("Modelview"), 1, 0, ptr(modelView));
    glUniformMatrix3fv(u("NormalMatrix"), 1, 0, ptr(normalMatrix));
    glUniform1f(u("Lerp"), lerp);

    glBindVertexArray(arrays.framePair);

    if (program == programs.blend) {

        glEnable(GL_RASTERIZER_DISCARD);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buffers.blendVerts);
        glBeginTransformFeedback(GL_POINTS);
        glDrawArrays(GL_POINTS, 0, numVerts);
        glEndTransformFeedback();

        glDisable(GL_RASTERIZER_DISCARD);
        glUseProgram(programs.pointCloud);
        glUniformMatrix4fv(u("Projection"), 1, 0, ptr(projection));
        glUniformMatrix4fv(u("Modelview"), 1, 0, ptr(modelView));
        glBindVertexArray(arrays.blended);
        glDrawArrays(GL_POINTS, 0, numVerts);
    }

    if (program == programs.centroids) {

        glUniform1i(u("Frame0"), 0);
        glUniform1i(u("Frame1"), 1);

        glEnable(GL_RASTERIZER_DISCARD);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_BUFFER, textures.frame0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_BUFFER, textures.frame1);
        glBindVertexArray(arrays.inverseMapping);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buffers.centroids);
        glBeginTransformFeedback(GL_POINTS);
        glDrawArrays(GL_POINTS, 0, numQuads);
        glEndTransformFeedback();

        glDisable(GL_RASTERIZER_DISCARD);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_BUFFER, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_BUFFER, 0);
        glUseProgram(programs.pointCloud);
        glUniformMatrix4fv(u("Projection"), 1, 0, ptr(projection));
        glUniformMatrix4fv(u("Modelview"), 1, 0, ptr(modelView));
        glBindVertexArray(arrays.centroids);
        glDrawArrays(GL_POINTS, 0, numQuads);
    }

    if (program == programs.genFacetNormals) {

        glUniform1i(u("Frame0"), 0);
        glUniform1i(u("Frame1"), 1);

        glEnable(GL_RASTERIZER_DISCARD);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_BUFFER, textures.frame0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_BUFFER, textures.frame1);
        glBindVertexArray(arrays.inverseMapping);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buffers.facetNormals);
        glBeginTransformFeedback(GL_POINTS);
        glDrawArrays(GL_POINTS, 0, numQuads);
        glEndTransformFeedback();

        glDisable(GL_RASTERIZER_DISCARD);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_BUFFER, textures.inverseTopology);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_BUFFER, textures.facetNormals);

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1, 1);

        glUseProgram(programs.useFacetNormals);
        glUniform1i(u("FacetNormals"), 0);
        glUniform1i(u("InverseTopo"), 1);
        glUniformMatrix4fv(u("Projection"), 1, 0, ptr(projection));
        glUniformMatrix4fv(u("Modelview"), 1, 0, ptr(modelView));
        glUniformMatrix3fv(u("NormalMatrix"), 1, 0, ptr(normalMatrix));
        glUniform1f(u("Lerp"), lerp);
        glBindVertexArray(arrays.framePair);
        glDrawElements(GL_LINES_ADJACENCY, numQuads * 4, GL_UNSIGNED_SHORT, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_BUFFER, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_BUFFER, 0);

        glUseProgram(programs.wireframe);
        glUniform4f(u("WireframeColor"), 0, 0, 0, 0.25);
        glUniformMatrix4fv(u("Projection"), 1, 0, ptr(projection));
        glUniformMatrix4fv(u("Modelview"), 1, 0, ptr(modelView));
        glUniformMatrix3fv(u("NormalMatrix"), 1, 0, ptr(normalMatrix));
        glUniform1f(u("Lerp"), lerp);
        glDrawElements(GL_LINES_ADJACENCY, numQuads * 4, GL_UNSIGNED_SHORT, 0);
        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    if (program == programs.wireframe) {
        glUniform4f(u("WireframeColor"), 1, 1, 1, 0.25);
        glDrawElements(GL_LINES_ADJACENCY, numQuads * 4, GL_UNSIGNED_SHORT, 0);
    }

    if (program == programs.onePassFacets) {
        glDrawElements(GL_LINES_ADJACENCY, numQuads * 4, GL_UNSIGNED_SHORT, 0);
    }

    // Draw the HUD
    glDisable(GL_DEPTH_TEST);
    glUseProgram(programs.overlay);
    glUniform2f(u("InverseViewport"),
                1.0f / PezGetConfig().Width,
                1.0f / PezGetConfig().Height);
    glBindTexture(GL_TEXTURE_2D, textures.overlays[cycle]);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);

    pezCheckGL("Render Loop");
}

void PezUpdate(float seconds)
{
    worldTime += seconds;
    //worldTime = 360 / 12 - 5; // screenshot!
}
