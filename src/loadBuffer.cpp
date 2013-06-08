#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>
#include <pez/pez.h>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <cstring>

using namespace std;
using namespace glm;

static void addFace(u16vec4* faces, unsigned short face);

GLuint loadBuffer(
    GLenum target,
    string filename,
    bool addPadding,
    size_t* byteCount,
    GLuint* inverseBuffer)
{
    string errorMsg = "loadBuffer " + filename;
    filename = "data/" + filename;
    ifstream input(filename.c_str(), ios::binary );
    
    vector<char> buffer(
        (istreambuf_iterator<char>(input)), 
        (istreambuf_iterator<char>()));

    pezCheck(buffer.size() > 0, errorMsg.c_str());

    // TODO factor this into a new function
    if (addPadding) {
        vector<char> paddedBuffer(buffer.size() * 16 / 12);
        size_t i = 0, j = 0;
        for (; i < buffer.size(); i += 12, j += 16) {
            memcpy(&paddedBuffer[j], &buffer[i], 12);
        }
        buffer.swap(paddedBuffer);
    }

    GLuint name;
    glGenBuffers(1, &name);
    glBindBuffer(target, name);
    glBufferData(target, buffer.size(), &buffer[0], GL_STATIC_DRAW);

    if (byteCount) {
        *byteCount = buffer.size();
    }

    // TODO factor this into a new function
    if (inverseBuffer) {

        unsigned short* index = reinterpret_cast<unsigned short*>(&buffer[0]);
        unsigned short maxIndex = 0;
        for (size_t i = 0; i < buffer.size() / 2; ++i, ++index) {
            if (*index > maxIndex) {
                maxIndex = *index;
            }
        }

        vector<u16vec4> vertexToFace(maxIndex + 1);
        for (size_t v = 0; v < maxIndex + 1u; ++v) {
            vertexToFace[v] = u16vec4(0xffff);
        }

        size_t numQuads = buffer.size() / 8;
        cout << numQuads << " quads.\n";

        u16vec4* quads = reinterpret_cast<u16vec4*>(&buffer[0]);
        for (size_t q = 0; q < numQuads; ++q) {
            addFace(&vertexToFace[quads[q].x], q);
            addFace(&vertexToFace[quads[q].y], q);
            addFace(&vertexToFace[quads[q].z], q);
            addFace(&vertexToFace[quads[q].w], q);
        }

        bool debug = false;
        if (debug) {
            for (size_t v = 0; v < maxIndex + 1u; ++v) {
                cout << v <<  " " << vertexToFace[v].x << " " <<
                    vertexToFace[v].y << " " <<
                    vertexToFace[v].z << " " <<
                    vertexToFace[v].w << endl;
            }
        }

        target = GL_TEXTURE_BUFFER;
        glGenBuffers(1, inverseBuffer);
        glBindBuffer(target, *inverseBuffer);
        glBufferData(target, 8 * vertexToFace.size(), &vertexToFace[0],
                     GL_STATIC_DRAW);
    }

    pezCheckGL(errorMsg.c_str());
    return name;
}

static void addFace(u16vec4* faces, unsigned short face) 
{
    if (faces->x == 0xffff) {
        faces->x = face;
        return;
    }
    if (faces->y == 0xffff) {
        faces->y = face;
        return;
    }
    if (faces->z == 0xffff) {
        faces->z = face;
        return;
    }
    if (faces->w == 0xffff) {
        faces->w = face;
        return;
    }
    cerr << "Vertex has more than 4 incident faces.\n";
    exit(1);
}
