#include <string>
#include "lib/pez/pez.h"
#include "glm/gtc/type_ptr.hpp"

GLuint loadProgram(std::string vs, std::string gs, std::string fs);
GLuint loadTexture(std::string filename);
GLuint loadBuffer(
    GLenum target,
    std::string filename,
    bool addPadding,
    size_t* byteCount = 0,
    GLuint* inverseBuffer = 0);

inline
GLuint CurrentProgram()
{
    GLuint p;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &p);
    return p;
}

struct Attr {
    enum {
        Position,
        Normal,
        Position0,
        Position1,
    };
};

struct Frag {
    enum {
        Color,
        Normal,
    };
};

// A couple commonly used helper macros
#define u(x) glGetUniformLocation(CurrentProgram(), x)
#define ptr(x) glm::value_ptr(x)
#define offset(x) ((const GLvoid*)(x))

