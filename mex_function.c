#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "Platform.h"
#include <wglew.h>
#include <time.h>

/* Input variables */
char* model;
double rotationMatrix[3][3];
int mode;

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	if (nrhs != 3)
		mexErrMsgIdAndTxt("StringIO:WrongNumArgs",
			"Three inputs are needed\n1.	(array)Full path to model (ex:'D:\\dev\\MESH-THK\\model\\venus.ctm','D:/dev/MESH-THK/model/bunny.ctm', or 'D:\dev\MESH-THK\model\venus.ctm' are acceptable)\n2.	(matrix)Euler rotation matrix (ex: eye(3))\n3.	(int)Thickness mode (0: nomal thickness, 1: z-axis depth map)\n\n"
		);
	if (nlhs > 1)  mexErrMsgIdAndTxt("StringIO:WrongNumArgs","zero or one output is available");
	
	/* Model information */
	model = mxArrayToString(prhs[0]);

	/* Euler rotation matrix */
	PezCheckCondition(mxGetN(prhs[1]) == ROTATION_DIMENSION & mxGetM(prhs[1]) == ROTATION_DIMENSION, "Euler rotation matrix should have 3x3 dimension");
	for (int j = 0; j < mxGetM(prhs[1]); j++)
		for (int k = 0; k < mxGetN(prhs[1]); k++) 
			rotationMatrix[j][k] = mxGetPr(prhs[1])[k + j*mxGetN(prhs[1])];

	/* Thickness mode */
	mode = (unsigned short)*(double*)mxGetData(prhs[2]);
	PezCheckCondition(mode == 0 | mode == 1, "Check the mode argument\n(int)Thickness mode (0: nomal thickness, 1: z-axis depth map)\n");

	/* Variable initialization */
    LPCSTR szName = "Mesh thickness";
    WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(0), 0, 0, 0, 0, szName, 0 };
    DWORD dwStyle = WS_SYSMENU | WS_VISIBLE | WS_POPUP;
    DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    RECT rect;
    int windowWidth, windowHeight, windowLeft, windowTop;
    HWND hWnd;
    PIXELFORMATDESCRIPTOR pfd;
    HDC hDC;
    HGLRC hRC;
    int pixelFormat;
    MSG msg = {0};
    LARGE_INTEGER previousTime;
    LARGE_INTEGER freqTime;

	GLfloat pixelMapFront[PEZ_VIEWPORT_HEIGHT][PEZ_VIEWPORT_WIDTH] = {0,};
	GLfloat pixelMapBack[PEZ_VIEWPORT_HEIGHT][PEZ_VIEWPORT_WIDTH] = {0,};
	GLfloat thicknessMap[PEZ_VIEWPORT_WIDTH][PEZ_VIEWPORT_HEIGHT] = {0,};
	GLenum err;
	time_t timer;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    RegisterClassExA(&wc);

	/* Create window */
    SetRect(&rect, 0, 0, PEZ_VIEWPORT_WIDTH, PEZ_VIEWPORT_HEIGHT);
    AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);
    windowWidth = rect.right - rect.left;
    windowHeight = rect.bottom - rect.top;
    windowLeft = GetSystemMetrics(SM_XVIRTUALSCREEN) + GetSystemMetrics(SM_CXSCREEN) / 2 - windowWidth / 2;
    windowTop = GetSystemMetrics(SM_CYSCREEN) / 2 - windowHeight / 2;
    hWnd = CreateWindowExA(0, szName, szName, dwStyle, windowLeft, windowTop, windowWidth, windowHeight, 0, 0, 0, 0);

    /* Create the GL context */
    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    hDC = GetDC(hWnd);
    pixelFormat = ChoosePixelFormat(hDC, &pfd);

    SetPixelFormat(hDC, pixelFormat, &pfd);
    hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);

    err = glewInit();
    if (GLEW_OK != err) PezFatalError("GLEW Error: %s\n", glewGetErrorString(err));
    
    const char* szWindowTitle = PezInitialize(PEZ_VIEWPORT_WIDTH, PEZ_VIEWPORT_HEIGHT, model, rotationMatrix);
    SetWindowTextA(hWnd, szWindowTitle);

    QueryPerformanceFrequency(&freqTime);
    QueryPerformanceCounter(&previousTime);

    /* Start to render */
	GLfloat max;
	switch (mode) {
		case THK_NORMAL:
		{
			ViewFromZaxis(PEZ_EYE_POSITION);
			PezRender(CULL_FRONT);
			SwapBuffers(hDC);
			PezRender(CULL_BACK);
			SwapBuffers(hDC);
			glReadPixels(0, 0, PEZ_VIEWPORT_WIDTH, PEZ_VIEWPORT_HEIGHT, GL_RED, GL_FLOAT, pixelMapFront);
			PezRender(CULL_FRONT);
			SwapBuffers(hDC);
			glReadPixels(0, 0, PEZ_VIEWPORT_WIDTH, PEZ_VIEWPORT_HEIGHT, GL_RED, GL_FLOAT, pixelMapBack);
			//  (0,0) => bottom left corner in pixel map, upper left corner in thicknessMap

			/* Find max to normalize thickness */
			float pv;
			max = -1;
			for (int y = PEZ_VIEWPORT_HEIGHT - 1; y >= 0; y--) {
				for (int x = 0; x < PEZ_VIEWPORT_WIDTH; x++) {
					pv = pixelMapFront[y][x] - pixelMapBack[y][x];
					if (pv > max) max = pv;
				}
			}

			/* Normalize thickness with max thickness */
			for (int y = PEZ_VIEWPORT_HEIGHT - 1; y >= 0; y--) {
				for (int x = 0; x < PEZ_VIEWPORT_WIDTH; x++) {
					pv = (pixelMapFront[y][x] - pixelMapBack[y][x]) / max;
					if(pv>0) thicknessMap[x][PEZ_VIEWPORT_HEIGHT - y - 1] = pv;
					else thicknessMap[x][PEZ_VIEWPORT_HEIGHT - y - 1] = 0;
				}
			}

			/* Return normal depth map to matlab */
			plhs[0] = mxCreateNumericMatrix(PEZ_VIEWPORT_WIDTH, PEZ_VIEWPORT_HEIGHT, mxSINGLE_CLASS, mxREAL);
			memcpy(mxGetPr(plhs[0]), thicknessMap, PEZ_VIEWPORT_WIDTH * PEZ_VIEWPORT_HEIGHT * sizeof(GL_FLOAT));
			break;
		}
		case THK_Z:
		{
			PezRender(CULL_FRONT);
			SwapBuffers(hDC);
			PezRender(CULL_BACK);
			SwapBuffers(hDC);
			glReadPixels(0, 0, PEZ_VIEWPORT_WIDTH, PEZ_VIEWPORT_HEIGHT, GL_RED, GL_FLOAT, pixelMapFront);
			PezRender(CULL_FRONT);
			SwapBuffers(hDC);
			glReadPixels(0, 0, PEZ_VIEWPORT_WIDTH, PEZ_VIEWPORT_HEIGHT, GL_RED, GL_FLOAT, pixelMapBack);
			//  (0,0) => bottom left corner in pixel map, upper left corner in thicknessMap

			/* Find max to normalize thickness */
			float pv;
			max = -1;
			for (int y = PEZ_VIEWPORT_HEIGHT - 1; y >= 0; y--) {
				for (int x = 0; x < PEZ_VIEWPORT_WIDTH; x++) {
					pv = pixelMapFront[y][x] - pixelMapBack[y][x];
					if (pv > max) max = pv;
				}
			}

			/* Store thickness to output of mexFunction */
			for (int y = PEZ_VIEWPORT_HEIGHT - 1; y >= 0; y--) {
				for (int x = 0; x < PEZ_VIEWPORT_WIDTH; x++) {
					pv = (pixelMapFront[y][x] - pixelMapBack[y][x]) / max;
					if (pv>0) thicknessMap[x][PEZ_VIEWPORT_HEIGHT - y - 1] = pv;
					else thicknessMap[x][PEZ_VIEWPORT_HEIGHT - y - 1] = 0;
				}
			}

			/* Return z-along depth map to matlab */ 
			plhs[0] = mxCreateNumericMatrix(PEZ_VIEWPORT_WIDTH, PEZ_VIEWPORT_HEIGHT, mxSINGLE_CLASS, mxREAL);
			memcpy(mxGetPr(plhs[0]), thicknessMap, PEZ_VIEWPORT_WIDTH * PEZ_VIEWPORT_HEIGHT * sizeof(GL_FLOAT));
			break;
		}
	}	// end case

	/* Destroy objects */
	UnregisterClassA(szName, wc.hInstance);
	DestroyWindow(hWnd);

    return;
}

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void _PezFatalErrorW(const wchar_t* pStr, va_list a)
{
    wchar_t msg[1024] = {0};
    _vsnwprintf_s(msg, _countof(msg), _TRUNCATE, pStr, a);
	mexErrMsgIdAndTxt("mexError:runtimeError", strcat(msg, "\n"));
}

void _PezFatalError(const char* pStr, va_list a)
{
    char msg[1024] = {0};
    _vsnprintf_s(msg, _countof(msg), _TRUNCATE, pStr, a);
	mexErrMsgIdAndTxt("mexError:runtimeError",strcat(msg,"\n"));
}

void PezFatalError(const char* pStr, ...)
{
    va_list a;
    va_start(a, pStr);
    _PezFatalError(pStr, a);
}

void PezFatalErrorW(const wchar_t* pStr, ...)
{
    va_list a;
    va_start(a, pStr);
    _PezFatalErrorW(pStr, a);
}

void PezCheckCondition(int condition, ...)
{
    va_list a;
    const char* pStr;

    if (condition)
        return;

    va_start(a, condition);
    pStr = va_arg(a, const char*);
    _PezFatalError(pStr, a);
}

void PezCheckConditionW(int condition, ...)
{
    va_list a;
    const wchar_t* pStr;

    if (condition)
        return;

    va_start(a, condition);
    pStr = va_arg(a, const wchar_t*);
    _PezFatalErrorW(pStr, a);
}
