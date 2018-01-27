#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

// matlab interface
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
void PezRender(enum CULL_FACE);                 /* Interface to mex_function */
void ViewFromZaxis(float zCoor);

// Here's the mesh thickness configuration section.  Modify these constants to your liking!
//

#define DEPTH_SCALE 1.0			// Set up depth scale. This value is used to control light scale. 
#define PEZ_EYE_POSITION 10		// For normal thickness, this value set the eye position on the z-axis. From there depth map is rendered.
								// Set small if rendered depth map is narrow and set large if whole image is not captured.
#define PEZ_VIEWPORT_WIDTH 2048	// The width and height of output depth map 
#define PEZ_VIEWPORT_HEIGHT 2048
#define PEZ_GL_VERSION_TOKEN "GL2"
extern int mode;

// mesh thickness also defines a small handful of fixed constants and macros:
//
enum CULL_FACE {CULL_FRONT, CULL_BACK};	// Define which face is culled
enum {THK_NORMAL, THK_Z};				// Thickness mode
#define Pi (3.14159265f)
#define ROTATION_DIMENSION 3

// Additionally, Pez provides the following utility functions as alternatives to printf, exceptions, and asserts.
//
void PezFatalError(const char* pStr, ...);
void PezFatalErrorW(const wchar_t* pStr, ...);
void PezCheckCondition(int condition, ...);
void PezCheckConditionW(int condition, ...);

#ifdef __cplusplus
}
#endif
