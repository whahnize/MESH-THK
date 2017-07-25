// Pez was developed by Philip Rideout and released under the MIT License.

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "mex.h"
#include "matrix.h"

#ifdef __IPAD__
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#define SUFFIX ".ES2"
#else
#include <glew.h>
#define SUFFIX ".GL2"
#endif

// Pez is a platform abstraction layer for OpenGL; it's an extra-tiny alternative to GLUT.
// To create a cross-platform app, simply link against the Pez source and implement these four functions.
//
const char* PezInitialize(int width, int height, char* model, double roationMatrix[3][3]); // receive window size and return window title
void PezRender(enum CULL_FACE);                 // draw scene (Pez swaps the backbuffer for you)
void PezUpdate(unsigned int microseconds);        // receive elapsed time (e.g., update physics)
void PezHandleMouse(int x, int y, int action);    // handle mouse action: PEZ_DOWN, PEZ_UP, or PEZ_MOVE
    
const char* PezResourcePath(); // Implemented by the platform layer.  Returns a string like ".."

// Here's the Pez configuration section.  Modify these constants to your liking!
//
#define PEZ_VIEWPORT_WIDTH 720
#define PEZ_VIEWPORT_HEIGHT 720
#define PEZ_ENABLE_MULTISAMPLING 0
#define PEZ_VERTICAL_SYNC 1
#define PEZ_GL_VERSION_TOKEN "GL2"
#define PEZ_FORWARD_COMPATIBLE_GL 0
extern int mode;
// Pez also defines a small handful of fixed constants and macros:
//
enum CULL_FACE {CULL_FRONT, CULL_BACK, CULL_BACK_FRONT};
enum { THK_NORMAL, THK_Z};
enum {PEZ_DOWN, PEZ_UP, PEZ_MOVE};
#define TwoPi (6.28318531f)
#define Pi (3.14159265f)
#define countof(A) (sizeof(A) / sizeof(A[0]))

// Additionally, Pez provides the following utility functions as alternatives to printf, exceptions, and asserts.
// On Windows, strings get sent to the debugger, so use the VS debugger window or dbgview to see them.
//
void PezDebugString(const char* pStr, ...);
void PezDebugStringW(const wchar_t* pStr, ...);
void PezFatalError(const char* pStr, ...);
void PezFatalErrorW(const wchar_t* pStr, ...);
void PezCheckCondition(int condition, ...);
void PezCheckConditionW(int condition, ...);
int PezIsPressing(char key);

#ifdef __cplusplus
}
#endif
