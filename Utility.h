#include "Platform.h"

typedef struct MeshRec
{
    GLuint Positions;
	GLuint PositionCount;
    GLuint Normals;
    GLuint Faces;
    GLsizei FaceCount;
} Mesh;

Mesh CreateMesh(const char* ctmFile, double rotationMatrix[3][3]);
GLuint CreateProgram(const char* vsKey, const char* fsKey);