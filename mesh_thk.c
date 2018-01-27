#include "Platform.h"
#include "Utility.h"
#include <vectormath.h>
#include <stdlib.h>
#include <string.h>

static GLuint DepthProgram;
static GLuint AbsorptionProgram;
static Matrix4 ProjectionMatrix;
static Matrix4 ModelviewMatrix;
static GLuint OffscreenFbo;
static Mesh mesh;
static GLuint QuadVbo;
static GLint depthScale;
static GLfloat depthMap[PEZ_VIEWPORT_WIDTH*PEZ_VIEWPORT_HEIGHT] = {0,};

static void LoadUniforms(GLuint program)
{
    GLint modelview = glGetUniformLocation(program, "Modelview");
    if (modelview > -1)
    {
        glUniformMatrix4fv(modelview, 1, 0, &ModelviewMatrix.col0.x);
    }

    GLint normalMatrix = glGetUniformLocation(program, "NormalMatrix");
    if (normalMatrix > -1)
    {
        Matrix3 nm = M3Transpose(M4GetUpper3x3(ModelviewMatrix));
        float packed[9] = {
            nm.col0.x, nm.col1.x, nm.col2.x,
            nm.col0.y, nm.col1.y, nm.col2.y,
            nm.col0.z, nm.col1.z, nm.col2.z };
        glUniformMatrix3fv(normalMatrix, 1, 0, &packed[0]);
    }

    GLint projection = glGetUniformLocation(program, "Projection");
    if (projection > -1)
    {
        glUniformMatrix4fv(projection, 1, 0, &ProjectionMatrix.col0.x);
    }
	GLint depth = glGetUniformLocation(DepthProgram, "DepthScale");

	if (depth > -1)
	{
		glUniform1f(depth, depthScale);
	}
	
}

static void RenderMesh(enum CULL_FACE face)
{
	/* Set up */
    glUseProgram(DepthProgram);
    LoadUniforms(DepthProgram);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.Faces);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.Positions);

    int positionSlot = glGetAttribLocation(DepthProgram, "Position");
    glVertexAttribPointer(positionSlot, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
    glEnableVertexAttribArray(positionSlot);

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);


	switch (face)
	{
	case CULL_FRONT:
		glCullFace(GL_BACK);
		glDrawElements(GL_TRIANGLES, mesh.FaceCount * 3, GL_UNSIGNED_INT, 0);
			break;
	case CULL_BACK:
		glCullFace(GL_FRONT);
		glDrawElements(GL_TRIANGLES, mesh.FaceCount * 3, GL_UNSIGNED_INT, 0);
			break;
	}
	
	/* Tear down */
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(positionSlot);
}

void PezRender(enum CULL_FACE face)
{
    glBindFramebuffer(GL_FRAMEBUFFER, OffscreenFbo);
    glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    RenderMesh(face);
}

const char* PezInitialize(int width, int height, char* model, double rotationMatrix[3][3])
{
    mesh = CreateMesh(model, rotationMatrix);

	switch (mode) {
	case THK_NORMAL:
	{
		DepthProgram = CreateProgram("Glass.Vertex.Normal", "Glass.Fragment.Normal" SUFFIX);

		// Create a floating-point render target:
		GLuint textureHandle;
		glGenTextures(1, &textureHandle);
		glBindTexture(GL_TEXTURE_2D, textureHandle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, PEZ_VIEWPORT_WIDTH, PEZ_VIEWPORT_HEIGHT, 0, GL_RED, GL_FLOAT, 0);

		GLuint fboHandle;
		glGenFramebuffers(1, &fboHandle);
		glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureHandle, 0);

		OffscreenFbo = fboHandle;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		break;
	}
	case THK_Z:
		DepthProgram = CreateProgram("Glass.Vertex.Z", "Glass.Fragment.Z" SUFFIX);
		break;
	}

    // Set up the projection matrix:
    const float HalfWidth = 0.5;
    const float HalfHeight = HalfWidth * PEZ_VIEWPORT_HEIGHT / PEZ_VIEWPORT_WIDTH;
    ProjectionMatrix = M4MakeFrustum(-HalfWidth, +HalfWidth, -HalfHeight, +HalfHeight, 5, 20);

	depthScale = DEPTH_SCALE;
    return "Mesh Thickness";
}

void ViewFromZaxis(float zCoor)
{
    Vector3 offset = V3MakeFromElems(0, 0, 0);
    Matrix4 model = M4MakeRotationZ(0);
    model = M4Mul(M4MakeTranslation(offset), model);
    model = M4Mul(model, M4MakeTranslation(V3Neg(offset)));

    Point3 eyePosition = P3MakeFromElems(0, zCoor, 0);
    Point3 targetPosition = P3MakeFromElems(0, 0, 0);
    Vector3 upVector = V3MakeFromElems(0, 0, 1);
    Matrix4 view = M4MakeLookAt(eyePosition, targetPosition, upVector);
    
    ModelviewMatrix = M4Mul(view, model);
}