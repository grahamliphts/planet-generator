#pragma once

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "include\GL\glut.h"
#include <iostream>
#include <ctime>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

class XYZ {
	public: 
		double x, y, z,inc,angle;
		XYZ(double a, double b, double c, float a_inc = 1.0)
		{
			x = a;
			y = b;
			z = c;
			inc = a_inc;
			angle = 0;
		}

		XYZ()
		{
			x = 0;
			y = 0;
			z = 0;
			inc = 0;
			angle = 0;
		}

} ;

typedef struct {
	double r, g, b;
} COLOUR;

typedef struct {
	XYZ p[3];   /* Vertices */
	int c[3];   /* Hight Counts */
} TF;

typedef struct {
	XYZ vp;              /* View position           */
	XYZ vd;              /* View direction vector   */
	XYZ vu;              /* View up direction       */
	XYZ pr;              /* Point to rotate about   */
	double focallength;  /* Focal Length along vd   */
	double aperture;     /* Camera aperture         */
	double eyesep;       /* Eye separation          */
	int screenwidth, screenheight;
} CAMERA;

#define DTOR            0.0174532925
#define RTOD            57.2957795
#define TWOPI           6.283185307179586476925287
#define PI              3.141592653589793238462643
#define PID2            1.570796326794896619231322
#define ESC 27
#define CROSSPROD(p1,p2,p3) \
   p3.x = p1.y*p2.z - p1.z*p2.y; \
   p3.y = p1.z*p2.x - p1.x*p2.z; \
   p3.z = p1.x*p2.y - p1.y*p2.x
#define DOTPRODUCT(v1,v2) ( v1.x*v2.x + v1.y*v2.y + v1.z*v2.z )
#define ABS(x) (x < 0 ? -(x) : (x))
#define MIN(x,y) (x < y ? x : y)
#define MAX(x,y) (x > y ? x : y)

void Display(void);
void CreateEnvironment(void);
/*
void CreateInitialPlanet(void);
void CalcBounds(void);
void MakeGeometry(int,int,bool sun = false);
void DrawHistogram(void);
*/
void MakeLighting(void);
void HandleKeyboard(unsigned char key, int x, int y);
void HandleSpecialKeyboard(int key, int x, int y);
void HandleMouse(int, int, int, int);
void HandleMainMenu(int);
void HandleIterMenu(int);
void HandleMethodMenu(int);
void HandleResolMenu(int);
void HandleHeightMenu(int);
void HandleColourMenu(int);
void HandleVisibility(int vis);
void HandleReshape(int, int);
void HandleMouseMotion(int, int);
void HandleTimer(int);
void GiveUsage(char *);
void RotateCamera(int, int, int);
void TranslateCamera(int, int);
void CameraHome(int);
void FlyCamera(int);
int  FormSphere(int,int);
int  MakeNSphere(TF *, int);

void skybox(void);
GLuint LoadTexture(const char * filename, int width, int height);
void FreeTexture(GLuint texture);

double DotProduct(XYZ, XYZ);
double Modulus(XYZ);
void Multiply(XYZ *p, float x);
void   Normalise(XYZ *);
XYZ    VectorSub(XYZ, XYZ);
XYZ    VectorAdd(XYZ, XYZ);
XYZ    MidPoint(XYZ, XYZ);
COLOUR GetColour(double, double, double, int);
void CreateSimpleSphere(XYZ, double, int, int);
void initializeGlutMenu();
void glutParametersInitialization();
void planetOrbits();
void drawByShader();
int FormSphereAsteroid(int depth, int asteroidId);
void MakeGeometryAsteroid(int GLlistIndex, long seed);