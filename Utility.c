#include "Platform.h"
#include "Utility.h"
#include <glsw.h>
#include <openctm.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h> 
#include <time.h>
Mesh CreateMesh(const char* ctmFile, double rotationMatrix[3][3])
{
    Mesh mesh = {0, 0, 0, 0};
    char qualifiedPath[256] = {0};
	strcpy(qualifiedPath, ctmFile);

    // Open the CTM file:
    CTMcontext ctmContext = ctmNewContext(CTM_IMPORT);
    ctmLoad(ctmContext, qualifiedPath);
    PezCheckCondition(ctmGetError(ctmContext) == CTM_NONE, "OpenCTM issue with loading %s", qualifiedPath);
    CTMuint vertexCount = ctmGetInteger(ctmContext, CTM_VERTEX_COUNT);
    CTMuint faceCount = ctmGetInteger(ctmContext, CTM_TRIANGLE_COUNT);
    
    // Create the VBO for positions:
    const CTMfloat* positions = ctmGetFloatArray(ctmContext, CTM_VERTICES);
	CTMfloat* zeroCenteredPositions =(CTMfloat*)malloc(sizeof(CTMfloat) * 3 * vertexCount);
	CTMfloat* cylidericalPositions = (CTMfloat*)malloc(sizeof(CTMfloat) * 3 * vertexCount); // in theta, z, r order 
    if (positions) {
        GLuint handle;
		GLfloat avr_x =0, avr_y=0, avr_z = 0;
		for (GLuint vertex = 0; vertex < vertexCount; vertex++) {
			avr_x += positions[3 * vertex];
			avr_y += positions[3 * vertex + 1];
			avr_z += positions[3 * vertex + 2];
		}
		avr_x = avr_x / vertexCount;
		avr_y = avr_y / vertexCount;
		avr_z = avr_z / vertexCount;

		for (GLuint vertex = 0; vertex < vertexCount; vertex++) {
			zeroCenteredPositions[3 * vertex] = positions[3 * vertex] - avr_x;
			zeroCenteredPositions[3 * vertex + 1] = positions[3 * vertex + 1] - avr_y;
			zeroCenteredPositions[3 * vertex + 2] = positions[3 * vertex + 2] - avr_z;
		}
		GLfloat maxDis = -1;
		GLfloat dis;
		for (GLuint vertex = 0; vertex < vertexCount; vertex++) {
			dis = sqrt(pow(zeroCenteredPositions[3 * vertex], 2) + pow(zeroCenteredPositions[3 * vertex + 1], 2) + pow(zeroCenteredPositions[3 * vertex + 2], 2));
			if (dis > maxDis) maxDis = dis;
		}
		for (GLuint vertex = 0; vertex < vertexCount; vertex++) {
			zeroCenteredPositions[3 * vertex] = zeroCenteredPositions[3 * vertex]/ maxDis;
			zeroCenteredPositions[3 * vertex + 1] = zeroCenteredPositions[3 * vertex + 1] / maxDis;
			zeroCenteredPositions[3 * vertex + 2] = zeroCenteredPositions[3 * vertex + 2] / maxDis;
		}
		for (GLuint vertex = 0; vertex < vertexCount; vertex++) {
			GLfloat x = zeroCenteredPositions[3 * vertex];
			GLfloat y = zeroCenteredPositions[3 * vertex + 1];
			GLfloat z = zeroCenteredPositions[3 * vertex + 2];
			zeroCenteredPositions[3 * vertex] = rotationMatrix[0][0] * x + rotationMatrix[0][1] * y + rotationMatrix[0][2] * z;
			zeroCenteredPositions[3 * vertex + 1] = rotationMatrix[1][0] * x + rotationMatrix[1][1] * y + rotationMatrix[1][2] * z;
			zeroCenteredPositions[3 * vertex + 2] = rotationMatrix[2][0] * x + rotationMatrix[2][1] * y + rotationMatrix[2][2] *z;
		}
		// TO DO : Refactoring 
		if (mode == THK_Z) {
			GLfloat maxR = -1;
			GLfloat minR = 987654321;

			GLfloat maxTheta = -1;
			GLfloat minTheta = 987654321;

			GLfloat maxZ = -1;
			GLfloat minZ = 987654321;
			for (GLuint vertex = 0; vertex < vertexCount; vertex++) {
				cylidericalPositions[3 * vertex] = (atan2(zeroCenteredPositions[3 * vertex + 1], zeroCenteredPositions[3 * vertex]) * 180 / Pi) / 180.0; //theta; from [-pi/2,pi/2] to [-1.1]
				cylidericalPositions[3 * vertex + 1] = zeroCenteredPositions[3 * vertex + 2]; // z
				cylidericalPositions[3 * vertex + 2] = sqrt(pow(zeroCenteredPositions[3 * vertex], 2) + pow(zeroCenteredPositions[3 * vertex + 1], 2)); // r

				if (cylidericalPositions[3 * vertex] > maxTheta) maxTheta = cylidericalPositions[3 * vertex];
				if (cylidericalPositions[3 * vertex] < minTheta) minTheta = cylidericalPositions[3 * vertex];
				if (cylidericalPositions[3 * vertex + 1] > maxZ) maxZ = cylidericalPositions[3 * vertex + 1];
				if (cylidericalPositions[3 * vertex + 1] < minZ) minZ = cylidericalPositions[3 * vertex + 1];
				if (cylidericalPositions[3 * vertex + 2] > maxR) maxR = cylidericalPositions[3 * vertex + 2];
				if (cylidericalPositions[3 * vertex + 2] < minR) minR = cylidericalPositions[3 * vertex + 2];
			}
			for (GLuint vertex = 0; vertex < vertexCount; vertex++) {
				cylidericalPositions[3 * vertex + 2] = cylidericalPositions[3 * vertex + 2] - minR;
				cylidericalPositions[3 * vertex + 2] = cylidericalPositions[3 * vertex + 2] / 1.25*(maxR - minR);
			}
		}
        GLsizeiptr size = vertexCount * sizeof(float) * 3;
        glGenBuffers(1, &handle);
        glBindBuffer(GL_ARRAY_BUFFER, handle);
		switch (mode) {
		case THK_NORMAL:
			glBufferData(GL_ARRAY_BUFFER, size, zeroCenteredPositions, GL_STATIC_DRAW);
			break;
		case THK_Z:
			glBufferData(GL_ARRAY_BUFFER, size, cylidericalPositions, GL_STATIC_DRAW);
			break;
		}
       
        mesh.Positions = handle;
    }
    
    // Create the VBO for normals:
    const CTMfloat* normals = ctmGetFloatArray(ctmContext, CTM_NORMALS);
	unsigned int* faceBuffer;
    if (normals) {
        GLuint handle;

        GLsizeiptr size = vertexCount * sizeof(float) * 3;
        glGenBuffers(1, &handle);
        glBindBuffer(GL_ARRAY_BUFFER, handle);
        glBufferData(GL_ARRAY_BUFFER, size, normals, GL_STATIC_DRAW);
        mesh.Normals = handle;
    }
    
    // Create the VBO for indices:
    const CTMuint* indices = ctmGetIntegerArray(ctmContext, CTM_INDICES);
    if (indices) {
        
        GLsizeiptr bufferSize = faceCount * 3 * sizeof(unsigned int);
        
        // Convert indices from 8-bit to 16-bit:
        unsigned int* remainFaceBuffer = (unsigned int*) malloc(bufferSize);
		
        unsigned int* pDest = remainFaceBuffer;
        const CTMuint* pSrc = indices;
        unsigned int remainingFaces = faceCount;
		
		// TO DO: refactoring ; hard to read 
		while (remainingFaces--)
		{
			// boundaryIndicator indicates whether a face is on a boundary line of spliited model. When a face is on boundary line, its value is set to 1 or -1.
			int boundaryIndicator = 0;
			for (int vertice = 0; vertice < 3; vertice++) {
				if (cylidericalPositions[3 * *(pSrc + vertice)] > 0) boundaryIndicator += 1;
				else boundaryIndicator -= 1;
			}
			if (mode==THK_NORMAL || !(boundaryIndicator == -1 || boundaryIndicator == 1) || pow(cylidericalPositions[3 * *pSrc], 2) < 0.5) { // when mode is normal or a face is not on the boundary
				*pDest++ = (unsigned int)*pSrc++;
				*pDest++ = (unsigned int)*pSrc++;
				*pDest++ = (unsigned int)*pSrc++;
			}
			else {
				faceCount -= 1;
				pSrc += 3;
			}
		}

		bufferSize = faceCount * 3 * sizeof(unsigned int);

		// Convert indices from 8-bit to 16-bit:
		faceBuffer = (unsigned int*)malloc(bufferSize);
		pDest = faceBuffer;
		const CTMuint* pSrc1 = remainFaceBuffer;
		remainingFaces = faceCount;
		while (remainingFaces--)
		{
			*pDest++ = (unsigned int)*pSrc1++;
			*pDest++ = (unsigned int)*pSrc1++;
			*pDest++ = (unsigned int)*pSrc1++;
		}
        GLuint handle;
        glGenBuffers(1, &handle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize, faceBuffer, GL_STATIC_DRAW);
        mesh.Faces = handle;

		free(remainFaceBuffer);
       
    }
    
    ctmFreeContext(ctmContext);

    mesh.FaceCount = faceCount;
	mesh.PositionCount = vertexCount;
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    

	CTMcontext context;

	// Create a new context
	context = ctmNewContext(CTM_EXPORT);

	// Define our mesh representation to OpenCTM (store references to it in
	// the context)
	if(mode == THK_NORMAL) ctmDefineMesh(context, zeroCenteredPositions, vertexCount, faceBuffer, faceCount, NULL);
	else ctmDefineMesh(context, cylidericalPositions, vertexCount, faceBuffer, faceCount, NULL);

	char buf[100];
	int timer = time(NULL);
	sprintf(buf, "%s-%d.ctm", ctmFile, timer);
	//Save the OpenCTM file if needed
	//ctmSave(context, buf);

	// Free the context
	ctmFreeContext(context);
	free(zeroCenteredPositions);
	free(cylidericalPositions);
	free(faceBuffer);
	return mesh;
}

GLuint CreateProgram(const char* vsKey, const char* fsKey)
{
    static int first = 1;
    if (first)
    {
        glswInit();
		glswAddPath("../../../", ".glsl");
		glswAddPath("../../", ".glsl");
        glswAddPath("../", ".glsl");
        glswAddPath("./", ".glsl");
        first = 0;
    }
    const char* vsSource = glswGetShader(vsKey);
    const char* fsSource = glswGetShader(fsKey);
    const char* msg = "Can't find %s shader: '%s'.\n";
    PezCheckCondition(vsSource != 0, msg, "vertex", vsKey);
    PezCheckCondition(fsSource != 0, msg, "fragment", fsKey);
    GLint compileSuccess;
    GLchar compilerSpew[256];

    GLuint vsHandle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsHandle, 1, &vsSource, 0);
    glCompileShader(vsHandle);
    glGetShaderiv(vsHandle, GL_COMPILE_STATUS, &compileSuccess);
    glGetShaderInfoLog(vsHandle, sizeof(compilerSpew), 0, compilerSpew);
    PezCheckCondition(compileSuccess, "Can't compile %s:\n%s", vsKey, compilerSpew);
    GLuint fsHandle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsHandle, 1, &fsSource, 0);
    glCompileShader(fsHandle);
    glGetShaderiv(fsHandle, GL_COMPILE_STATUS, &compileSuccess);
    glGetShaderInfoLog(fsHandle, sizeof(compilerSpew), 0, compilerSpew);
    PezCheckCondition(compileSuccess, "Can't compile %s:\n%s", fsKey, compilerSpew);
    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vsHandle);
    glAttachShader(programHandle, fsHandle);
    glLinkProgram(programHandle);
    
    GLint linkSuccess;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
    glGetProgramInfoLog(programHandle, sizeof(compilerSpew), 0, compilerSpew);
    PezCheckCondition(linkSuccess, "Can't link %s with %s:\n%s", vsKey, fsKey, compilerSpew);
    return programHandle;
}