#include "common.h"
#include "lib/pez/pez.h"
#include <string>

using namespace std;

static void _bind_Attr_and_Frag(GLuint p, bool doTransformFeedback)
{
    glBindAttribLocation(p, Attr::Position, "Position");
    glBindAttribLocation(p, Attr::Position0, "Position0");
    glBindAttribLocation(p, Attr::Position1, "Position1");
    glBindAttribLocation(p, Attr::Normal, "Normal");

    glBindFragDataLocation(p, Frag::Color, "Color");
    glBindFragDataLocation(p, Frag::Normal, "Normal");

    if (doTransformFeedback) {
        const char* varyings[1] = { "Feedback" };
        glTransformFeedbackVaryings(p, 1, varyings, GL_SEPARATE_ATTRIBS);
    }
}

GLuint loadProgram(string vsKey, string gsKey, string fsKey)
{
    GLchar spew[2048];
    GLint compileSuccess;
    GLuint programHandle = glCreateProgram();

    const char* vsSource = pezGetShader(vsKey.c_str());
    pezCheck(vsSource != 0, "Can't find vshader: %s\n", vsKey.c_str());
    GLuint vsHandle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsHandle, 1, &vsSource, 0);
    glCompileShader(vsHandle);
    glGetShaderiv(vsHandle, GL_COMPILE_STATUS, &compileSuccess);
    glGetShaderInfoLog(vsHandle, sizeof(spew), 0, spew);
    pezCheck(compileSuccess, "Can't compile vshader [%s]:\n%s", vsKey.c_str(),
             spew);
    glAttachShader(programHandle, vsHandle);

    if (gsKey.size()) {
        const char* gsSource = pezGetShader(gsKey.c_str());
        pezCheck(gsSource != 0, "Can't find gshader: %s\n", gsKey.c_str());
        GLuint gsHandle = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(gsHandle, 1, &gsSource, 0);
        glCompileShader(gsHandle);
        glGetShaderiv(gsHandle, GL_COMPILE_STATUS, &compileSuccess);
        glGetShaderInfoLog(gsHandle, sizeof(spew), 0, spew);
        pezCheck(compileSuccess, "Can't compile gshader [%s]:\n%s",
                 gsKey.c_str(), spew);
        glAttachShader(programHandle, gsHandle);
    }

    if (fsKey.size()) {
        const char* fsSource = pezGetShader(fsKey.c_str());
        pezCheck(fsSource != 0, "Can't find fshader: %s\n", fsKey.c_str());
        GLuint fsHandle = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fsHandle, 1, &fsSource, 0);
        glCompileShader(fsHandle);
        glGetShaderiv(fsHandle, GL_COMPILE_STATUS, &compileSuccess);
        glGetShaderInfoLog(fsHandle, sizeof(spew), 0, spew);
        pezCheck(compileSuccess, "Can't compile fshader [%s]:\n%s",
                 fsKey.c_str(), spew);
        glAttachShader(programHandle, fsHandle);
    }

    bool doTransformFeedback = fsKey.size() == 0;
    _bind_Attr_and_Frag(programHandle, doTransformFeedback);

    glLinkProgram(programHandle);
    GLint linkSuccess;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
    glGetProgramInfoLog(programHandle, sizeof(spew), 0, spew);
    pezCheck(linkSuccess, "Can't link shaders [fs: %s, vs: %s]:\n%s",
             fsKey.c_str(), vsKey.c_str(), spew);
    glUseProgram(programHandle);

    pezCheckGL("Shader compilation");

    return programHandle;
}
