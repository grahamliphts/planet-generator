#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include "Shader.h"
#include "Icosahedron.h"
#include "planet.h"
#include "drand48.h"
#include <vector>


/* Flags */
int fullscreen = FALSE;
int stereo = FALSE;
int showconstruct = FALSE;
int drawwireframe = FALSE;
int uselights = TRUE;
int dosmooth = TRUE;
int windowdump = FALSE;
int record = FALSE;
int debug = FALSE;
int demomode = FALSE;
GLuint textureSkyBox[6]; //the array for our texture

#define NOTDIRTY      0
#define SLIGHTLYDIRTY 1
#define REALDIRTY     2
#define ADDONE        3
int geometrydirty = REALDIRTY;

/* Planet description */
TF *faces = NULL;
int nface = 0;
int spheredepth = 5;
int iterationdepth = 0;
double radiusmin = 1, radiusmax = 1;
int colourmap = 12;
int showocean = FALSE;
double deltaheight = 0.001;
//long seedvalue = 12345;
int whichmethod = 0;

int currentbutton = -1;
double dtheta = 1;
CAMERA camera;
double m_near, m_far;

XYZ origin = { 0.0,0.0,0.0 }; // DO NOT TOUCH

XYZ PlanetPosition = { 0.0,10,0.0 };

XYZ *PlanetPositions;
int *planetFaceList;
TF **planetFaces;
int *PlanetColourMap;
int *PlanetNbMoon;
float *PlanetSize;
int planetAmount = 4;
XYZ *planetRotationAngle;
float timmerMax;
float timmerIncrement = 10 ;

std::vector<XYZ> AsteroidPosition;
std::vector<int> AsteroidFaceList;
std::vector<TF*> AsteroidFaces;
std::vector<int> AsteroidColourMap;
std::vector<float> AsteroidSize;
std::vector<XYZ> AsteroidRotationAngle;
double asteroidAmoint = 0;

//Icosa
static const GLuint PositionSlot = 0;
typedef struct {
	GLuint Projection;
	GLuint Model;
	GLuint View;
	GLuint NormalMatrix;
	GLuint LightPosition;
	GLuint AmbientMaterial;
	GLuint DiffuseMaterial;
	GLuint TessLevelInner;
	GLuint TessLevelOuter;
} ShaderUniforms;
static float TessLevelInner = 2;
static float TessLevelOuter = 3;
static glm::mat4 ProjectionMatrix;
static glm::mat4 ModelMatrix;
static glm::mat4 ViewMatrix;
static glm::mat3 NormalMatrix;
static ShaderUniforms Uniforms;


int main(int argc, char **argv)
{
	int i;
	camera.screenwidth = 1200;
	camera.screenheight = 700;

	// Parse the command line arguments (seriously ?, will you ever use command line argument...)
	for (i = 1; i < argc; i++) {
		if (strstr(argv[i], "-h") != NULL)
			GiveUsage(argv[0]);
		if (strstr(argv[i], "-f") != NULL)
			fullscreen = TRUE;
		if (strstr(argv[i], "-d") != NULL)
			debug = TRUE;
		if (strstr(argv[i], "-D") != NULL)
			demomode = TRUE;
	}

	//int seed = 666;
	//int seed = 6111992;
	//int seed = 15646;
	//int seed = 88888; // Super 


	//int seed = 781583;
	int seed = 1234;
	srand(seed);

	// Moteur Procedural
	planetAmount = rand() % 10 + 5;
	PlanetPositions = new XYZ[planetAmount]();
	planetRotationAngle = new XYZ[planetAmount]();
	planetFaceList = new int[planetAmount]();
	PlanetSize = new float[planetAmount]();
	planetFaces = new TF*[planetAmount]();
	PlanetColourMap = new int[planetAmount]();
	PlanetNbMoon = new int[planetAmount]();

	float previousY = 5;
	for (int i = 0; i < planetAmount - 1; i++)
	{
		bool isAsteroidField = false;
		PlanetSize[i] = ((rand() % 20) + 1) / 10.0f;
		int position = 5 + rand() % 5;
		
		if (i > 3 && PlanetSize[i] < 1 && rand() % 4 == 0)
		{
			printf("asteroid belt\n");
			isAsteroidField = true;
			int width = position - 4;
			float rotationBelt = (rand() % 3 + 4) / previousY;
			int sens = (rand() % 2 == 0) ? -1 : 1;
			int nbAsteroidPerSegment = (rand() % 10 + 10);
			for (int j = 0; j < nbAsteroidPerSegment * 180; j++)
			{
				asteroidAmoint++;
				AsteroidPosition.push_back(XYZ(
					(previousY + rand() % 5 - 2 + (rand() % 3 == 0 ? rand() % 3 - 2 : 0)) * cos((j / 360.0 * 2 * PI)), 
					(previousY + rand() % 5 - 2 + (rand() % 3 == 0 ? rand() % 3 - 2 : 0)) * sin((j / 360.0 * 2 * PI)), 
					(rand() % 101 - 50) / 50.0));
				AsteroidFaceList.push_back(0);
				AsteroidFaces.push_back(NULL);
				AsteroidColourMap.push_back(0);
				AsteroidSize.push_back(((rand() % 2) + 1) / 10.0f);
				AsteroidRotationAngle.push_back(XYZ(0, 0, sens, rotationBelt));
			}
			i--;
		}

		if (!isAsteroidField)
		{
			PlanetPositions[i] = XYZ(0, previousY + position + PlanetSize[i], rand() % 11 - 5);
			planetRotationAngle[i] = XYZ(0, 0, (rand() % 2 == 0) ? -1 : 1, (rand() % 5 + 5) / previousY);
			planetFaceList[i] = 0;
			planetFaces[i] = NULL;
			PlanetNbMoon[i] = 0;
			PlanetColourMap[i] = rand() % 4 + 9;


			// Add Moon(s)
			if ((PlanetSize[i] >= 1.2) && (rand() % 2 == 1) && (i + 1 < planetAmount - 2))
			{
				PlanetNbMoon[i]++;

				PlanetSize[i + 1] = (rand() % 10 + 1) / 10.0f;
				PlanetPositions[i + 1] = XYZ(0, 2 + rand() % 4 + PlanetSize[i + 1], 0);
				planetRotationAngle[i + 1] = XYZ(0, 0, (rand() % 2 == 0) ? -1 : 1, (rand() % 10 + 5) / 10.0f);
				planetFaceList[i + 1] = 0;
				planetFaces[i + 1] = NULL;
				PlanetNbMoon[i + 1] = 0;
				PlanetColourMap[i + 1] = 8;
				previousY += PlanetPositions[i + 1].y;

				if ((PlanetSize[i] >= 1.7) && (rand() % 2 == 1) && (i + 2 < planetAmount - 2))
				{
					PlanetNbMoon[i]++;

					PlanetSize[i + 2] = (rand() % 6 + 1) / 10.0f;
					PlanetPositions[i + 2] = XYZ(0, 1 + rand() % 3 + PlanetSize[i + 2] + PlanetPositions[i + 1].y, 0);
					planetRotationAngle[i + 2] = XYZ(0, 0, (rand() % 2 == 0) ? -1 : 1, (rand() % 10 + 5) / 5.0f);
					planetFaceList[i + 2] = 0;
					planetFaces[i + 2] = NULL;
					PlanetNbMoon[i + 2] = 0;
					PlanetColourMap[i + 2] = 8;
					previousY += PlanetPositions[i + 2].y;

					i++;
				}
				i++;
			}
		}
		previousY += position * 2;
	}

	PlanetSize[planetAmount - 1] = 5;
	PlanetPositions[planetAmount - 1] = XYZ(0, 0, 0);
	planetRotationAngle[planetAmount - 1] = XYZ(0, 0, 0);
	planetFaceList[planetAmount - 1] = 0;
	planetFaces[planetAmount - 1] = NULL;
	PlanetColourMap[planetAmount - 1] = 16;
	PlanetNbMoon[planetAmount - 1] = 0;

	//Glut Settings


	glutInit(&argc, argv);
	
	glutParametersInitialization();
	GLint GlewInitResult = glewInit();
	if (GLEW_OK != GlewInitResult)
	{
		printf("ERROR: %s\n", glewGetErrorString(GlewInitResult));
		exit(EXIT_FAILURE);
	}
	initializeGlutMenu();

	for (int i = 0; i < planetAmount ; i++)
	{
		planetFaceList[i] = FormSphere(spheredepth,i+1);
	}

	for (int i = 0; i < asteroidAmoint; i++)
	{
		AsteroidFaceList[i] = FormSphereAsteroid(2, i + 1);
	}
	
	


	CreateEnvironment();
	CameraHome(0);
	
	timmerMax = clock() + timmerIncrement;
	// Launch and cross your fingers
	glutMainLoop();
	return(0);
}

#pragma region Terrain Generation (yup yup yup )
void MakeGeometry(int GLlistIndex,long seed,bool sun = false)
{
	/*
	Create the geometry
	- Create a surface
	- Turn it into an OpenGL list
	(Again, in theory it work so cross your fingers and suff here)
	*/
	/* All 3D terrain Generation is here, kind of implementation of
	the diamond square algorythm*/
	int i, j, k, niter = 1;
	double r, r1, r2, r3, dp, scale, offset;
	double len, sealevel = 0;

	int planetFaceAmount = planetFaceList[GLlistIndex - 1];
	TF* planetFacesList = planetFaces[GLlistIndex - 1];
	int ColourMapValue = PlanetColourMap[GLlistIndex - 1];

	COLOUR colour;
	XYZ p, p1, p2, p3, n;
	fprintf(stdout, "------ Generating planet %d--------- \n", GLlistIndex);
	/* First itteration of surface (you know, the dirty one ...
	BTW just look at the define name...)*/
	if (geometrydirty == REALDIRTY) {
		for (i = 0; i < 3; i++) {
			for (j = 0; j < planetFaceAmount; j++) {
				Normalise(&(planetFacesList[j].p[i]));
				planetFacesList[j].c[i] = 0;
			}
		}
		niter = iterationdepth;
		srand48(seed);
	}

	if (geometrydirty == REALDIRTY || geometrydirty == ADDONE) {

		/* Form the new surface */
		for (i = 0; i < niter; i++) {
			
			/* Choose a random normal */
			
			n.x = drand48() - 0.5;
			n.y = drand48() - 0.5;
			n.z = drand48() - 0.5;
			
			//printf("%f %f %f \n", n.x, n.y, n.z);
			Normalise(&n);

			offset = drand48() - 0.5;
			//offset = rand() - 0.5;
			//fprintf(stdout, "------ Offset %f--------- \n", offset);
			/* Applying vertical pertubation on point
			based on diamond square logic
			(any question ? wikipedia)*/

			for (j = 0; j < planetFaceAmount; j++) {
				for (k = 0; k < 3; k++) {
					if (whichmethod == 1) {
						p = planetFacesList[j].p[k];
					}
					else {
						p.x = planetFacesList[j].p[k].x - offset * n.x;
						p.y = planetFacesList[j].p[k].y - offset * n.y;
						p.z = planetFacesList[j].p[k].z - offset * n.z;
					}
					if ((dp = DotProduct(n, p)) > 0)
						planetFacesList[j].c[k]++;
					else
						planetFacesList[j].c[k]--;
				}
			}
		}

		/* Adjust the heights */
		for (j = 0; j < planetFaceAmount; j++) {
			for (k = 0; k < 3; k++) {
				Normalise(&(planetFacesList[j].p[k]));
				scale = 1 + deltaheight * planetFacesList[j].c[k];
				planetFacesList[j].p[k].x *= scale;
				planetFacesList[j].p[k].y *= scale;
				planetFacesList[j].p[k].z *= scale;
			}
		}
	}

	/* Find the range */
	/*
	radiusmin = 1;
	radiusmax = 1;
	*/
	for (i = 0; i < planetFaceAmount; i++) {
		for (k = 0; k < 3; k++) {
			r = Modulus(planetFacesList[i].p[k]);
			radiusmin = MIN(radiusmin, r);
			radiusmax = MAX(radiusmax, r);
		}
	}
	radiusmin -= deltaheight;
	radiusmax += deltaheight;
	if (debug)
		fprintf(stderr, "Radius range %g -> %g\n", radiusmin, radiusmax);

	/* Create the opengl data */
	glNewList(GLlistIndex, GL_COMPILE);

	/* Draw the ocean sphere */
	if (showocean && !sun) {
		sealevel = radiusmin + (radiusmax - radiusmin) / 2;
		glColor3f(0.4, 0.4, 1.0); // BLUE
		CreateSimpleSphere(origin, sealevel - 0.01, 60, 0);
		radiusmin = sealevel;
	}
	
	if (sun) {
		sealevel = PlanetSize[GLlistIndex - 1];// radiusmin + (radiusmax - radiusmin) / 2;
		glColor3f(255, 216, 0); // Yellow
		CreateSimpleSphere(origin, sealevel - 0.01, 60, 0);
		radiusmin = sealevel;
	}
	
	glBegin(GL_TRIANGLES);
	for (i = 0; i < planetFaceAmount; i++) {
		p1 = planetFacesList[i].p[0];
		r1 = Modulus(p1);
		p2 = planetFacesList[i].p[1];
		r2 = Modulus(p2);
		p3 = planetFacesList[i].p[2];
		r3 = Modulus(p3);
		if (r1 < sealevel && r2 < sealevel && r3 < sealevel)
			continue;

		Multiply(&p1, PlanetSize[GLlistIndex - 1]);
		Multiply(&p2, PlanetSize[GLlistIndex - 1]);
		Multiply(&p3, PlanetSize[GLlistIndex - 1]);

		colour = GetColour(r1, radiusmin, radiusmax, ColourMapValue);
		glColor4f(colour.r, colour.g, colour.b, 1.0);
		glNormal3f(p1.x, p1.y, p1.z);
		glVertex3f(p1.x, p1.y, p1.z);

		colour = GetColour(r2, radiusmin, radiusmax, ColourMapValue);
		glColor4f(colour.r, colour.g, colour.b, 1.0);
		glNormal3f(p2.x, p2.y, p2.z);
		glVertex3f(p2.x, p2.y, p2.z);

		colour = GetColour(r3, radiusmin, radiusmax, ColourMapValue);
		glColor4f(colour.r, colour.g, colour.b, 1.0);
		glNormal3f(p3.x, p3.y, p3.z);
		glVertex3f(p3.x, p3.y, p3.z);
	}
	glEnd();
	glEndList();
}

void MakeGeometryAsteroid(int GLlistIndex, long seed)
{
	/*
	Create the geometry
	- Create a surface
	- Turn it into an OpenGL list
	(Again, in theory it work so cross your fingers and suff here)
	*/
	/* All 3D terrain Generation is here, kind of implementation of
	the diamond square algorythm*/
	int i, j, k, niter = 1;
	double r, r1, r2, r3, dp, scale, offset;
	double len = 0;

	int asteroidFaceAmount = AsteroidFaceList[GLlistIndex - 1];
	TF* asteroidFacesList = AsteroidFaces[GLlistIndex - 1];
	int ColourMapValue = AsteroidColourMap[GLlistIndex - 1];

	COLOUR colour;
	XYZ p, p1, p2, p3, n;
	/* First itteration of surface (you know, the dirty one ...
	BTW just look at the define name...)*/
	if (geometrydirty == REALDIRTY) {
		for (i = 0; i < 3; i++) {
			for (j = 0; j < asteroidFaceAmount; j++) {
				Normalise(&(asteroidFacesList[j].p[i]));
				asteroidFacesList[j].c[i] = 0;
			}
		}
		niter = iterationdepth;
		srand48(seed);
	}
	
	if (geometrydirty == REALDIRTY || geometrydirty == ADDONE) {

		/* Form the new surface */
		for (i = 0; i < niter; i++) {

			/* Choose a random normal */

			n.x = drand48() - 0.5;
			n.y = drand48() - 0.5;
			n.z = drand48() - 0.5;

			//printf("%f %f %f \n", n.x, n.y, n.z);
			Normalise(&n);

			offset = drand48() - 0.5;
			//offset = rand() - 0.5;
			//fprintf(stdout, "------ Offset %f--------- \n", offset);
			/* Applying vertical pertubation on point
			based on diamond square logic
			(any question ? wikipedia)*/

			for (j = 0; j < asteroidFaceAmount; j++) {
				for (k = 0; k < 3; k++) {
					if (whichmethod == 1) {
						p = asteroidFacesList[j].p[k];
					}
					else {
						p.x = asteroidFacesList[j].p[k].x - offset * n.x;
						p.y = asteroidFacesList[j].p[k].y - offset * n.y;
						p.z = asteroidFacesList[j].p[k].z - offset * n.z;
					}
					if ((dp = DotProduct(n, p)) > 0)
						asteroidFacesList[j].c[k]++;
					else
						asteroidFacesList[j].c[k]--;
				}
			}
		}

		/* Adjust the heights */
		for (j = 0; j < asteroidFaceAmount; j++) {
			for (k = 0; k < 3; k++) {
				Normalise(&(asteroidFacesList[j].p[k]));
				scale = 1 + deltaheight * asteroidFacesList[j].c[k];
				asteroidFacesList[j].p[k].x *= scale;
				asteroidFacesList[j].p[k].y *= scale;
				asteroidFacesList[j].p[k].z *= scale;
			}
		}
	}

	/* Find the range */
	/*
	radiusmin = 1;
	radiusmax = 1;*/
	

	/* Create the opengl data */
	glNewList(GLlistIndex + planetAmount, GL_COMPILE);

	glBegin(GL_TRIANGLES);
	for (i = 0; i < asteroidFaceAmount; i++) {
		p1 = asteroidFacesList[i].p[0];
		r1 = Modulus(p1);
		p2 = asteroidFacesList[i].p[1];
		r2 = Modulus(p2);
		p3 = asteroidFacesList[i].p[2];
		r3 = Modulus(p3);

		Multiply(&p1, AsteroidSize[GLlistIndex - 1]);
		Multiply(&p2, AsteroidSize[GLlistIndex - 1]);
		Multiply(&p3, AsteroidSize[GLlistIndex - 1]);

		colour = GetColour(r1, radiusmin, radiusmax, ColourMapValue);
		glColor4f(colour.r, colour.g, colour.b, 1.0);
		glNormal3f(p1.x, p1.y, p1.z);
		glVertex3f(p1.x, p1.y, p1.z);

		colour = GetColour(r2, radiusmin, radiusmax, ColourMapValue);
		glColor4f(colour.r, colour.g, colour.b, 1.0);
		glNormal3f(p2.x, p2.y, p2.z);
		glVertex3f(p2.x, p2.y, p2.z);

		colour = GetColour(r3, radiusmin, radiusmax, ColourMapValue);
		glColor4f(colour.r, colour.g, colour.b, 1.0);
		glNormal3f(p3.x, p3.y, p3.z);
		glVertex3f(p3.x, p3.y, p3.z);
	}
	glEnd();
	glEndList();
}
#pragma endregion
#pragma region Rendering And stuff
void glutParametersInitialization()
{
	// DO NOT CHANGE 
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow("Planet Generator");
	glutReshapeWindow(camera.screenwidth, camera.screenheight);
	if (fullscreen)
		glutFullScreen();
	glutDisplayFunc(Display);
	glutReshapeFunc(HandleReshape);
	glutVisibilityFunc(HandleVisibility);
	glutKeyboardFunc(HandleKeyboard);
	glutSpecialFunc(HandleSpecialKeyboard);
	glutMouseFunc(HandleMouse);
	glutMotionFunc(HandleMouseMotion);

}
void initializeGlutMenu()
{
	//Initializing Glut Menu (change here for options)
	int mainmenu, itermenu, heightmenu, resolmenu;
	int methodmenu, colourmenu;
	/* Iteration menu */
	itermenu = glutCreateMenu(HandleIterMenu);
	glutAddMenuEntry("Decrease iteration depth", 1);
	glutAddMenuEntry("Increase iteration depth", 2);
	glutAddMenuEntry("Do 100 more", 3);
	glutAddMenuEntry("Do 1000 more", 4);
	glutAddMenuEntry("Reset", 5);

	/* Height menu */
	heightmenu = glutCreateMenu(HandleHeightMenu);
	glutAddMenuEntry("Low", 1);
	glutAddMenuEntry("Average", 2);
	glutAddMenuEntry("High", 3);
	glutAddMenuEntry("Higher", 4);

	/* Sphere resolution menu */
	resolmenu = glutCreateMenu(HandleResolMenu);
	glutAddMenuEntry("Low (8192 facets)", 5);
	glutAddMenuEntry("Average (32768 facets)", 6);
	glutAddMenuEntry("High (131072 facets)", 7);
	glutAddMenuEntry("Extreme (524288 facets)", 8);

	/* Colour map menu */
	colourmenu = glutCreateMenu(HandleColourMenu);
	glutAddMenuEntry("Mars 1", 11);
	glutAddMenuEntry("Mars 2", 12);
	glutAddMenuEntry("Earth (Sea to snow)", 15);
	glutAddMenuEntry("Extreme earth", 10);
	glutAddMenuEntry("Land to snow", 13);
	glutAddMenuEntry("Grey Scale", 3);
	glutAddMenuEntry("Hot to cold", 1);

	/* Algorithm menu */
	methodmenu = glutCreateMenu(HandleMethodMenu);
	glutAddMenuEntry("Plane through origin", 1);
	glutAddMenuEntry("Plane not through origin", 2);

	/* Set up the main menu */
	mainmenu = glutCreateMenu(HandleMainMenu);
	glutAddSubMenu("Iteration depth", itermenu);
	glutAddSubMenu("Height control", heightmenu);
	glutAddSubMenu("Sphere resolution", resolmenu);
	glutAddSubMenu("Colour map", colourmenu);
	glutAddMenuEntry("Toggle lights on/off", 1);
	glutAddMenuEntry("Toggle wireframe/solid", 2);
	glutAddMenuEntry("Toggle smooth shading on/off", 4);
	glutAddMenuEntry("Toggle ocean on/off", 5);
	glutAddMenuEntry("Quit", 10);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}
void CreateEnvironment(void)
{
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_POINT_SMOOTH);
	glDisable(GL_POLYGON_SMOOTH);
	glDisable(GL_DITHER);
	glDisable(GL_CULL_FACE);

	glLineWidth(1.0);
	glPointSize(1.0);

	glFrontFace(GL_CW);
	glClearColor(0.0, 0.0, 0.0, 0.0);         /* Background colour */
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
void Display(void)
{
	/*
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();

	textureSkyBox[0] = LoadTexture("Skybox/Back.bmp", 256, 256); //load the texture
	textureSkyBox[1] = LoadTexture("Skybox/Front.bmp", 256, 256); //load the texture
	textureSkyBox[2] = LoadTexture("Skybox/Left.bmp", 256, 256); //load the texture
	textureSkyBox[3] = LoadTexture("Skybox/Right.bmp", 256, 256); //load the texture
	textureSkyBox[4] = LoadTexture("Skybox/Bottom.bmp", 256, 256); //load the texture
	textureSkyBox[5] = LoadTexture("Skybox/Top.bmp", 256, 256); //load the texture
	glEnable(GL_TEXTURE_2D); //enable 2D texturing
	skybox();
	for (int i = 0; i < 6; i++) {
		FreeTexture(textureSkyBox[i]);
	}
	glutSwapBuffers();*/

	/* It should be a good idea to change my this ... but right now, i don't want to touch it again*/
	XYZ r, eyepos;
	double dist, ratio, radians, scale, wd2, ndfl;
	double left, right, top, bottom;
	

	/* Do we need to recreate the list ? */
	if (geometrydirty != NOTDIRTY) {
		for (int i = 0; i < planetAmount -1; i++)
		{
			MakeGeometry(i + 1, (long)rand());
		}
		MakeGeometry(planetAmount , (long)rand(),true);
		
		for (int i = 0; i < asteroidAmoint - 1; i++)
		{
			MakeGeometryAsteroid(i + 1, (long)rand());
		}
		geometrydirty = NOTDIRTY;
	}

	m_near = 0.1;
	m_far = 1000;

	/* Misc stuff */
	ratio = camera.screenwidth / (double)camera.screenheight;
	radians = DTOR * camera.aperture / 2;
	wd2 = m_near * tan(radians);
	ndfl = m_near / camera.focallength;

	/* Clear the buffers */
	glDrawBuffer(GL_BACK_LEFT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();


	left = -ratio * wd2;
	right = ratio * wd2;
	top = wd2;
	bottom = -wd2;
	glFrustum(left, right, bottom, top, m_near, m_far);
	glMatrixMode(GL_MODELVIEW);
	glDrawBuffer(GL_BACK_LEFT);

	glLoadIdentity();
	gluLookAt(camera.vp.x, camera.vp.y, camera.vp.z,
		camera.vp.x + camera.vd.x,
		camera.vp.y + camera.vd.y,
		camera.vp.z + camera.vd.z,
		camera.vu.x, camera.vu.y, camera.vu.z);
	MakeLighting();

#pragma region SIR REY YOU ARE LOOKING FOR THIS
	//Updating planet rotations
	
	if (timmerMax < clock())
	{
		planetOrbits();
		
		timmerMax = clock() + timmerIncrement;
	}
		

	//planetOrbits();

	for (int i = 0; i < planetAmount - 1; i++)
	{
		glPushMatrix(); // move isolation
		glRotatef(planetRotationAngle[i].angle, planetRotationAngle[i].x, planetRotationAngle[i].y, planetRotationAngle[i].z);
		glTranslatef(PlanetPositions[i].x, PlanetPositions[i].y, PlanetPositions[i].z); // move to draw point
		glRotatef(planetRotationAngle[i].angle * 2 * (1 / PlanetSize[i]), 0, 0, planetRotationAngle[i].z);
		glCallList(i + 1); // Draw Call

		for (int j = 1; j <= PlanetNbMoon[i]; j++)
		{
			glPushMatrix(); // move isolation
			glRotatef(planetRotationAngle[i + j].angle, planetRotationAngle[i + j].x, planetRotationAngle[i + j].y, planetRotationAngle[i + j].z);
			glTranslatef(PlanetPositions[i + j].x, PlanetPositions[i + j].y, PlanetPositions[i + j].z); // move to draw point
			glRotatef(planetRotationAngle[i + j].angle * 4, 0, 0, planetRotationAngle[i + j].z);
			glCallList(i + j + 1); // Draw Call
			glPopMatrix(); // end of move isolation
		}
		i += PlanetNbMoon[i];
		glPopMatrix(); // end of move isolation
	}
	
	// Sun
	glPushMatrix(); // move isolation
	glCallList(planetAmount);// Draw Call
	glPopMatrix(); // end of move isolation
	
	
	for (int i = 0; i < asteroidAmoint; i++)
	{
		glPushMatrix(); // move isolation
		glRotatef(AsteroidRotationAngle[i].angle, AsteroidRotationAngle[i].x, AsteroidRotationAngle[i].y, AsteroidRotationAngle[i].z);
		glTranslatef(AsteroidPosition[i].x, AsteroidPosition[i].y, AsteroidPosition[i].z); // move to draw point
		glRotatef(AsteroidRotationAngle[i].angle * 2 * (1 / AsteroidSize[i]), 0, 0, AsteroidRotationAngle[i].z);
		glCallList(planetAmount + i + 1); // Draw Call
		glPopMatrix(); // end of move isolation
	}

	//shader + location
	//drawByShader();
	glUseProgramObjectARB(0);
	

#pragma endregion	

	/* glFlush(); This isn't necessary for double buffers */
	glutSwapBuffers();
}

void drawByShader()
{
	for (int i = 0; i < asteroidAmoint; i++)
	{
		// apres glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); normalement
		glPatchParameteri(GL_PATCH_VERTICES, 3);
	
		GLfloat matrixb[16];
		GLfloat matrixa[16];
		auto sphere = Icosahedron();
		//Shader myShader("shader.vs", "shader.frag");
		Shader myShader("transform.vs", "transform.frag", "transform.geom", "transform.tcs", "transform.tes");
		myShader.Use();
		//light param PLAY HERE
		glUniform3f(Uniforms.AmbientMaterial, 0.04f, 0.04f, 0.04f);
		glUniform3f(Uniforms.DiffuseMaterial, 0, 0.75, 0.75);
		glm::vec4 lightPosition(0, 3, 0, 0); //light
		glUniform3fv(Uniforms.LightPosition, 1, glm::value_ptr(lightPosition));
	
		glRotatef(AsteroidRotationAngle[i].angle * 3, AsteroidRotationAngle[i].x, AsteroidRotationAngle[i].y, AsteroidRotationAngle[i].z);
		glTranslatef(AsteroidPosition[i].x, AsteroidPosition[i].y, AsteroidPosition[i].z); // move to draw point	

		Uniforms.Projection = glGetUniformLocation(myShader.Program, "Projection");
		Uniforms.Model = glGetUniformLocation(myShader.Program, "Model");
		Uniforms.View = glGetUniformLocation(myShader.Program, "View");
		Uniforms.NormalMatrix = glGetUniformLocation(myShader.Program, "NormalMatrix");
		Uniforms.LightPosition = glGetUniformLocation(myShader.Program, "LightPosition");
		Uniforms.AmbientMaterial = glGetUniformLocation(myShader.Program, "AmbientMaterial");
		Uniforms.DiffuseMaterial = glGetUniformLocation(myShader.Program, "DiffuseMaterial");
		Uniforms.TessLevelInner = glGetUniformLocation(myShader.Program, "TessLevelInner");
		Uniforms.TessLevelOuter = glGetUniformLocation(myShader.Program, "TessLevelOuter");
	

		glGetFloatv(GL_MODELVIEW_MATRIX, matrixa);
		ViewMatrix = glm::make_mat4(matrixa);

		glGetFloatv(GL_PROJECTION_MATRIX, matrixb);
		ProjectionMatrix = glm::make_mat4(matrixb);

		NormalMatrix = glm::mat3(ViewMatrix);

		//tess param
		glUniform1f(Uniforms.TessLevelInner, TessLevelInner);
		glUniform1f(Uniforms.TessLevelOuter, TessLevelOuter);
	

		glUniformMatrix4fv(Uniforms.View, 1, GL_FALSE, glm::value_ptr(ViewMatrix));
		glUniformMatrix4fv(Uniforms.Projection, 1, GL_FALSE, glm::value_ptr(ProjectionMatrix));
		glUniformMatrix4fv(Uniforms.Model, 1, GL_FALSE, glm::value_ptr(glm::mat4()));

		//Pass parameters to shader
		glm::mat3 nm = glm::transpose(NormalMatrix);
		glUniformMatrix3fv(Uniforms.NormalMatrix, 1, GL_FALSE, glm::value_ptr(nm));
		//glCallList(3);
		//glColor3f(0.4, 0.4, 1.0);
	
		sphere.draw();
	}
}
void planetOrbits()
{/*
	if (planetRotationAngle[2] < 360)
		planetRotationAngle[2] += 1;
	else
		planetRotationAngle[2] = 0;
*/
	
	for (int i = 0; i < planetAmount; i++)
	{
		if (planetRotationAngle[i].angle < 360)
			planetRotationAngle[i].angle += planetRotationAngle[i].inc;
		else
			planetRotationAngle[i].angle = 0;
	}

	for (int i = 0; i < asteroidAmoint; i++)
	{
		if (AsteroidRotationAngle[i].angle < 360)
			AsteroidRotationAngle[i].angle += AsteroidRotationAngle[i].inc;
		else
			AsteroidRotationAngle[i].angle = 0;
	}
}
void MakeLighting(void)
{
	/*
	Set up the lighing environment (a poor one, i know)
	*/
	int i;
	GLfloat globalambient[4] = { 0.3,0.3,0.3,1.0 };
	GLfloat white[4] = { 1.0,1.0,1.0,1.0 };
	GLfloat black[4] = { 0.0,0.0,0.0,1.0 };
	int deflightlist[8] = { GL_LIGHT0,GL_LIGHT1,GL_LIGHT2,GL_LIGHT3,
		GL_LIGHT4,GL_LIGHT5,GL_LIGHT6,GL_LIGHT7 };
	GLfloat p[4];
	XYZ q;
	GLfloat shiny[1] = { 100.0 };

	for (i = 0; i < 8; i++) {
		glDisable(deflightlist[i]);
		glLightfv(deflightlist[i], GL_AMBIENT, black);
		glLightfv(deflightlist[i], GL_DIFFUSE, white);
		glLightfv(deflightlist[i], GL_SPECULAR, black);
	}

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalambient);

	p[0] = 0;// camera.vp.x + camera.focallength * camera.vu.x;
	p[1] = 0;// camera.vp.y + camera.focallength * camera.vu.y;
	p[2] = 0;// camera.vp.z + camera.focallength * camera.vu.z;
	p[3] = 1;
	glLightfv(deflightlist[0], GL_DIFFUSE, white);
	/* glLightfv(deflightlist[0],GL_SPECULAR,white); */
	glLightfv(GL_LIGHT0, GL_POSITION, p);
	glEnable(GL_LIGHT0);

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shiny);

	if (drawwireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (dosmooth)
		glShadeModel(GL_SMOOTH);
	else
		glShadeModel(GL_FLAT);
	if (uselights)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);
}
#pragma endregion
#pragma region Camera Management
void RotateCamera(int ix, int iy, int iz)
{
	/*
	Camera move and stuff
	*/
	XYZ vp, vu, vd;
	XYZ right;
	XYZ newvp, newr;
	double radius, dd, radians;
	double dx, dy, dz;

	vu = camera.vu;
	Normalise(&vu);
	vp = camera.vp;
	vd = camera.vd;
	Normalise(&vd);
	CROSSPROD(vd, vu, right);
	Normalise(&right);
	radians = dtheta * PI / 180.0;

	/* Handle the roll */
	if (iz != 0) {
		camera.vu.x += iz * right.x * radians;
		camera.vu.y += iz * right.y * radians;
		camera.vu.z += iz * right.z * radians;
		Normalise(&camera.vu);
		return;
	}

	/* Distance from the rotate point */
	dx = camera.vp.x - camera.pr.x;
	dy = camera.vp.y - camera.pr.y;
	dz = camera.vp.z - camera.pr.z;
	radius = sqrt(dx*dx + dy*dy + dz*dz);

	/* Determine the new view point */
	dd = radius * radians;
	newvp.x = vp.x + dd * ix * right.x + dd * iy * vu.x - camera.pr.x;
	newvp.y = vp.y + dd * ix * right.y + dd * iy * vu.y - camera.pr.y;
	newvp.z = vp.z + dd * ix * right.z + dd * iy * vu.z - camera.pr.z;
	Normalise(&newvp);
	camera.vp.x = camera.pr.x + radius * newvp.x;
	camera.vp.y = camera.pr.y + radius * newvp.y;
	camera.vp.z = camera.pr.z + radius * newvp.z;

	/* Determine the new right vector */
	newr.x = camera.vp.x + right.x - camera.pr.x;
	newr.y = camera.vp.y + right.y - camera.pr.y;
	newr.z = camera.vp.z + right.z - camera.pr.z;
	Normalise(&newr);
	newr.x = camera.pr.x + radius * newr.x - camera.vp.x;
	newr.y = camera.pr.y + radius * newr.y - camera.vp.y;
	newr.z = camera.pr.z + radius * newr.z - camera.vp.z;

	camera.vd.x = camera.pr.x - camera.vp.x;
	camera.vd.y = camera.pr.y - camera.vp.y;
	camera.vd.z = camera.pr.z - camera.vp.z;
	Normalise(&camera.vd);

	/* Determine the new up vector */
	CROSSPROD(newr, camera.vd, camera.vu);
	Normalise(&camera.vu);
}
void TranslateCamera(int ix, int iy)
{
	/*
	Translate (pan) the camera view point
	In response to i,j,k,l keys
	Also move the camera rotate location in parallel
	REMEMBER ! THIS CAMERA IS A BIT CRAPY
	*/
	XYZ vp, vu, vd;
	XYZ right;
	XYZ newvp, newr;
	double radians, delta;

	vu = camera.vu;
	Normalise(&vu);
	vp = camera.vp;
	vd = camera.vd;
	Normalise(&vd);
	CROSSPROD(vd, vu, right);
	Normalise(&right);
	radians = dtheta * PI / 180.0;
	delta = dtheta * camera.focallength / 90.0;

	camera.vp.x += iy * vu.x * delta;
	camera.vp.y += iy * vu.y * delta;
	camera.vp.z += iy * vu.z * delta;
	camera.pr.x += iy * vu.x * delta;
	camera.pr.y += iy * vu.y * delta;
	camera.pr.z += iy * vu.z * delta;

	camera.vp.x += ix * right.x * delta;
	camera.vp.y += ix * right.y * delta;
	camera.vp.z += ix * right.z * delta;
	camera.pr.x += ix * right.x * delta;
	camera.pr.y += ix * right.y * delta;
	camera.pr.z += ix * right.z * delta;
}
void FlyCamera(int dir)
{
	// You really ask what this function do ?
	double delta = 1;

	camera.vp.x = camera.vp.x + dir * camera.vd.x * delta;
	camera.vp.y = camera.vp.y + dir * camera.vd.y * delta;
	camera.vp.z = camera.vp.z + dir * camera.vd.z * delta;
}
void CameraHome(int mode)
{
	//Reset Camera (more used than i expected
	camera.aperture = 50;
	camera.focallength = 4;
	camera.eyesep = camera.focallength / 20;
	camera.pr = origin;

	camera.vp.x = (camera.focallength - 1) - 30;
	camera.vp.y = -30;
	camera.vp.z = -30;
	camera.vd.x = -1;
	camera.vd.y = 0;
	camera.vd.z = 0;

	camera.vu.x = 0;
	camera.vu.y = 1;
	camera.vu.z = 0;
}
#pragma endregion
#pragma region Keyboard & mouse handle
void HandleKeyboard(unsigned char key, int x, int y)
{
	/*
	HIT ME !  said the keyboard
	I CAN HANDLE IT ! he said again
	.... it's late now....
	*/
	switch (key) {
	case ESC:                            /* Quit */
	case 'Q':
	case 'q':
		exit(0);
		break;
	case 'h':                           /* Go home    (you're drunk) */
	case 'H':
		CameraHome(0);
		break;
	case '[':                           /* Roll anti clockwise */
		RotateCamera(0, 0, -1);
		break;
	case ']':                           /* Roll clockwise */
		RotateCamera(0, 0, 1);
		break;
	case 'i':                           /* Translate camera up */
	case 'I':
		TranslateCamera(0, 1);
		break;
	case 'k':                           /* Translate camera down */
	case 'K':
		TranslateCamera(0, -1);
		break;
	case 'j':                           /* Translate camera left */
	case 'J':
		TranslateCamera(-1, 0);
		break;
	case 'l':                           /* Translate camera right */
	case 'L':
		TranslateCamera(1, 0);
		break;
	case '=':
	case '+':
		FlyCamera(1);
		break;
	case '-':
	case '_':
		FlyCamera(-1);
		break;
	case '<':
	case ',':
		iterationdepth--;
		if (iterationdepth < 0)
			iterationdepth = 0;
		geometrydirty = REALDIRTY;
		break;
	case '>':
	case '.':
		iterationdepth++;
		geometrydirty = REALDIRTY;
		break;
	}
}
void HandleSpecialKeyboard(int key, int x, int y)
{
	/*
	Thanks to the worldwide web, i found this but it should be absolutely useless
	*/
	switch (key) {
	case GLUT_KEY_LEFT:
		RotateCamera(-1, 0, 0);
		break;
	case GLUT_KEY_RIGHT:
		RotateCamera(1, 0, 0);
		break;
	case GLUT_KEY_UP:
		RotateCamera(0, 1, 0);
		break;
	case GLUT_KEY_DOWN:
		RotateCamera(0, -1, 0);
		break;
	case GLUT_KEY_F1:
		break;
	case GLUT_KEY_F2:
		break;
	}
}
void HandleMouseMotion(int x, int y)
{
	// OH LOOK !! This fuction does exactly what her name said !
	static int xlast = -1, ylast = -1;
	int dx, dy;

	dx = x - xlast;
	dy = y - ylast;
	if (dx < 0)      dx = -1;
	else if (dx > 0) dx = 1;
	if (dy < 0)      dy = -1;
	else if (dy > 0) dy = 1;

	if (currentbutton == GLUT_LEFT_BUTTON)
		RotateCamera(-dx, dy, 0);
	else if (currentbutton == GLUT_MIDDLE_BUTTON)
		RotateCamera(0, 0, dx);

	xlast = x;
	ylast = y;
}
#pragma endregion
#pragma region Handle Menu
void HandleMainMenu(int whichone)
{
	switch (whichone) {
	case 1:
		uselights = !uselights;
		geometrydirty = SLIGHTLYDIRTY;
		break;
	case 2:
		drawwireframe = !drawwireframe;
		geometrydirty = SLIGHTLYDIRTY;
		break;
	case 3:
		showconstruct = !showconstruct;
		geometrydirty = SLIGHTLYDIRTY;
		break;
	case 4:
		dosmooth = !dosmooth;
		geometrydirty = SLIGHTLYDIRTY;
		break;
	case 5:
		showocean = !showocean;
		geometrydirty = SLIGHTLYDIRTY;
		break;
	case 9:
		//seedvalue = rand();
		geometrydirty = REALDIRTY;
		break;
	case 10:
		exit(-1);
	}
}
void HandleColourMenu(int whichone)
{
	colourmap = whichone;
	geometrydirty = SLIGHTLYDIRTY;
}
void HandleMethodMenu(int whichone)
{
	whichmethod = whichone;
	geometrydirty = REALDIRTY;
}
void HandleResolMenu(int whichone)
{
	spheredepth = whichone;
	for (int i = 0; i < planetAmount; i++)
	{
		planetFaceList[i] = FormSphere(spheredepth,i+1);
	}
	geometrydirty = REALDIRTY;
}
void HandleHeightMenu(int whichone)
{
	switch (whichone) {
	case 1:
		deltaheight = 0.00001;
		break;
	case 2:
		deltaheight = 0.0001;
		break;
	case 3:
		deltaheight = 0.001;
		break;
	case 4:
		deltaheight = 0.01;
		break;
	}
	geometrydirty = REALDIRTY;
}
void HandleIterMenu(int whichone)
{
	switch (whichone) {
	case 1:
		iterationdepth--;
		if (iterationdepth < 0)
			iterationdepth = 0;
		geometrydirty = REALDIRTY;
		break;
	case 2:
		iterationdepth++;
		geometrydirty = ADDONE;
		break;
	case 3:
		iterationdepth += 100;
		geometrydirty = REALDIRTY;
		break;
	case 4:
		iterationdepth += 1000;
		geometrydirty = REALDIRTY;
		break;
	case 5:
		iterationdepth = 0;
		geometrydirty = REALDIRTY;
		break;
	}
}
#pragma endregion
#pragma region Glut Things
void HandleMouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		if (button == GLUT_LEFT_BUTTON) {
			currentbutton = GLUT_LEFT_BUTTON;
		}
		else if (button == GLUT_MIDDLE_BUTTON) {
			currentbutton = GLUT_MIDDLE_BUTTON;
		}
	}
}
void HandleVisibility(int visible)
{
	/*
	How to handle visibility
	Glut and stuff
	*/
	if (visible == GLUT_VISIBLE)
		HandleTimer(0);
}
void HandleTimer(int value)
{
	/*
	What to do on an idle event
	*/
	glutPostRedisplay();
	glutTimerFunc(30, HandleTimer, 0);
}
void HandleReshape(int w, int h)
{
	/*
	because resize is the life (especially on a fucking QHD screen ... thanks aorus)
	*/
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	camera.screenwidth = w;
	camera.screenheight = h;
}
#pragma endregion
#pragma region Sphere Generation
int FormSphere(int depth, int planetID)
{
	int i, n;

	n = 8;
	for (i = 0; i < depth; i++)
		n *= 4;

	if ((planetFaces[planetID - 1] = (TF*)realloc(planetFaces[planetID - 1], n * sizeof(TF))) == NULL) {
		fprintf(stderr, "Malloc of sphere failed\n");
		exit(-1);
	}

	n = MakeNSphere(planetFaces[planetID - 1], spheredepth);

	fprintf(stderr, "%d facets\n", n);

	return(n);
}

int FormSphereAsteroid(int depth, int asteroidId)
{
	int i, n;

	n = 8;
	for (i = 0; i < depth; i++)
		n *= 4;

	if ((AsteroidFaces[asteroidId - 1] = (TF*)realloc(AsteroidFaces[asteroidId - 1], n * sizeof(TF))) == NULL) {
		fprintf(stderr, "Malloc of sphere failed\n");
		exit(-1);
	}

	n = MakeNSphere(AsteroidFaces[asteroidId - 1], 0);

	return(n);
}

int MakeNSphere(TF *f, int iterations)
{
	/*
	Create a triangular facet approximation to a sphere
	Unit radius
	Return the number of facets created.
	The number of facets will be (4^iterations) * 8
	Because FUCK THIS FUCKING SPHERE ! (currently i'm almost out)
	*/
	int i, it;
	double a;
	XYZ p[6] = { XYZ(0,0,10),  XYZ(0,0,-10),  XYZ(-10,-10,0), XYZ(10,-10,0), XYZ(10,10,0), XYZ(-10,10,0) };
	XYZ pa, pb, pc;
	int nt = 0, ntold;

	/* Create the level 0 object */
	a = 1 / sqrt(2.0);
	for (i = 0; i < 6; i++) {
		p[i].x *= a;
		p[i].y *= a;
	}
	f[0].p[0] = p[0]; f[0].p[1] = p[3]; f[0].p[2] = p[4];
	f[1].p[0] = p[0]; f[1].p[1] = p[4]; f[1].p[2] = p[5];
	f[2].p[0] = p[0]; f[2].p[1] = p[5]; f[2].p[2] = p[2];
	f[3].p[0] = p[0]; f[3].p[1] = p[2]; f[3].p[2] = p[3];
	f[4].p[0] = p[1]; f[4].p[1] = p[4]; f[4].p[2] = p[3];
	f[5].p[0] = p[1]; f[5].p[1] = p[5]; f[5].p[2] = p[4];
	f[6].p[0] = p[1]; f[6].p[1] = p[2]; f[6].p[2] = p[5];
	f[7].p[0] = p[1]; f[7].p[1] = p[3]; f[7].p[2] = p[2];
	nt = 8;

	if (iterations < 1)
		return(nt);

	/* Bisect each edge and move to the surface of a unit sphere */
	for (it = 0; it < iterations; it++) {
		ntold = nt;
		for (i = 0; i < ntold; i++) {
			pa = MidPoint(f[i].p[0], f[i].p[1]);
			pb = MidPoint(f[i].p[1], f[i].p[2]);
			pc = MidPoint(f[i].p[2], f[i].p[0]);
			Normalise(&pa);
			Normalise(&pb);
			Normalise(&pc);
			f[nt].p[0] = f[i].p[0]; f[nt].p[1] = pa;        f[nt].p[2] = pc; nt++;
			f[nt].p[0] = pa;        f[nt].p[1] = f[i].p[1]; f[nt].p[2] = pb; nt++;
			f[nt].p[0] = pb;        f[nt].p[1] = f[i].p[2]; f[nt].p[2] = pc; nt++;
			f[i].p[0] = pa;
			f[i].p[1] = pb;
			f[i].p[2] = pc;
		}
	}

	return(nt);
}
void CreateSimpleSphere(XYZ c, double r, int n, int method)
{
	/*
	Create a simple sphere
	"method" is 0 for quads, 1 for triangles
	quads look nicer in wireframe mode (and tutorial was simplyer)
	*/
	int i, j;
	double theta1, theta2, theta3;
	XYZ e, p;

	if (r < 0)
		r = -r;
	if (n < 0)
		n = -n;
	if (n < 4 || r <= 0) {
		glBegin(GL_POINTS);
		glVertex3f(c.x, c.y, c.z);
		glEnd();
		return;
	}

	for (j = 0; j < n / 2; j++) {
		theta1 = j * TWOPI / n - PID2;
		theta2 = (j + 1) * TWOPI / n - PID2;

		if (method == 0)
			glBegin(GL_QUAD_STRIP);
		else
			glBegin(GL_TRIANGLE_STRIP);
		for (i = 0; i <= n; i++) {
			theta3 = i * TWOPI / n;

			e.x = cos(theta2) * cos(theta3);
			e.y = sin(theta2);
			e.z = cos(theta2) * sin(theta3);
			p.x = c.x + r * e.x;
			p.y = c.y + r * e.y;
			p.z = c.z + r * e.z;

			glNormal3f(e.x, e.y, e.z);
			glTexCoord2f(i / (double)n, 2 * (j + 1) / (double)n);
			glVertex3f(p.x, p.y, p.z);

			e.x = cos(theta1) * cos(theta3);
			e.y = sin(theta1);
			e.z = cos(theta1) * sin(theta3);
			p.x = c.x + r * e.x;
			p.y = c.y + r * e.y;
			p.z = c.z + r * e.z;

			glNormal3f(e.x, e.y, e.z);
			glTexCoord2f(i / (double)n, 2 * j / (double)n);
			glVertex3f(p.x, p.y, p.z);
		}
		glEnd();
	}
}
#pragma endregion
#pragma region Usefull 
double DotProduct(XYZ p1, XYZ p2)
{
	/*
	Dot product of two vectors in 3 space p1 dot p2
	you see dots ? that a dotprodut .... And night still go on
	*/
	return(p1.x*p2.x + p1.y*p2.y + p1.z*p2.z);
}
double Modulus(XYZ p)
{
	/*
	Calculate the length of a vector
	*/
	return(sqrt(p.x * p.x + p.y * p.y + p.z * p.z));
}
void Multiply(XYZ *p, float x)
{
	p->x *= x;
	p->y *= x;
	p->z *= x;
}
void Normalise(XYZ *p)
{
	/*
	Normalise a vector
	*/
	double length;

	length = sqrt(p->x * p->x + p->y * p->y + p->z * p->z);
	if (length != 0) {
		p->x /= length;
		p->y /= length;
		p->z /= length;
	}
	else {
		p->x = 0;
		p->y = 0;
		p->z = 0;
	}
}
XYZ VectorSub(XYZ p1, XYZ p2)
{
	/*
	Subtract two vectors p = p2 - p1
	*/
	XYZ p;

	p.x = p2.x - p1.x;
	p.y = p2.y - p1.y;
	p.z = p2.z - p1.z;

	return(p);
}
XYZ VectorAdd(XYZ p1, XYZ p2)
{
	/*
	Add two vectors p = p2 + p1
	*/
	XYZ p;

	p.x = p2.x + p1.x;
	p.y = p2.y + p1.y;
	p.z = p2.z + p1.z;

	return(p);
}
XYZ MidPoint(XYZ p1, XYZ p2)
{
	/*
	Return the midpoint between two vectors
	*/
	XYZ p;

	p.x = (p1.x + p2.x) / 2;
	p.y = (p1.y + p2.y) / 2;
	p.z = (p1.z + p2.z) / 2;

	return(p);
}
COLOUR GetColour(double v, double vmin, double vmax, int type)
{
	/*
	Return a colour from one of a number of colour ramps
	type == 1  blue -> cyan -> green -> magenta -> red
	2  blue -> red
	3  grey scale
	4  red -> yellow -> green -> cyan -> blue -> magenta -> red
	5  green -> yellow
	6  green -> magenta
	7  blue -> green -> red -> green -> blue
	8  white -> black -> white
	9  red -> blue -> cyan -> magenta
	10  blue -> cyan -> green -> yellow -> red -> white
	11  dark brown -> lighter brown (Mars colours, 2 colour transition)
	12  3 colour transition mars colours
	13  landscape colours, green -> brown -> yellow
	14  yellow -> red
	15  blue -> cyan -> green -> yellow -> brown -> white
	v should lie between vmin and vmax otherwise it is clipped
	The colour components range from 0 to 1

	Oh and, yes : 
	- too long
	- too complicated
	- you will hate this
	- (but) too indispensable
	and the result it : too,too,you,(but) too.... o//, \\o, o//, \\o, \o/
	(sorry)
	*/
	double dv, vmid;
	COLOUR c = { 1.0,1.0,1.0 };
	COLOUR c1, c2, c3;
	double ratio;

	if (v < vmin)
		v = vmin;
	if (v > vmax)
		v = vmax;
	dv = vmax - vmin;

	switch (type) {
	case 1:
		if (v < (vmin + 0.25 * dv)) {
			c.r = 0;
			c.g = 4 * (v - vmin) / dv;
			c.b = 1;
		}
		else if (v < (vmin + 0.5 * dv)) {
			c.r = 0;
			c.g = 1;
			c.b = 1 + 4 * (vmin + 0.25 * dv - v) / dv;
		}
		else if (v < (vmin + 0.75 * dv)) {
			c.r = 4 * (v - vmin - 0.5 * dv) / dv;
			c.g = 1;
			c.b = 0;
		}
		else {
			c.r = 1;
			c.g = 1 + 4 * (vmin + 0.75 * dv - v) / dv;
			c.b = 0;
		}
		break;
	case 2:
		c.r = (v - vmin) / dv;
		c.g = 0;
		c.b = (vmax - v) / dv;
		break;
	case 3:
		c.r = (v - vmin) / dv;
		c.b = c.r;
		c.g = c.r;
		break;
	case 4:
		if (v < (vmin + dv / 6.0)) {
			c.r = 1;
			c.g = 6 * (v - vmin) / dv;
			c.b = 0;
		}
		else if (v < (vmin + 2.0 * dv / 6.0)) {
			c.r = 1 + 6 * (vmin + dv / 6.0 - v) / dv;
			c.g = 1;
			c.b = 0;
		}
		else if (v < (vmin + 3.0 * dv / 6.0)) {
			c.r = 0;
			c.g = 1;
			c.b = 6 * (v - vmin - 2.0 * dv / 6.0) / dv;
		}
		else if (v < (vmin + 4.0 * dv / 6.0)) {
			c.r = 0;
			c.g = 1 + 6 * (vmin + 3.0 * dv / 6.0 - v) / dv;
			c.b = 1;
		}
		else if (v < (vmin + 5.0 * dv / 6.0)) {
			c.r = 6 * (v - vmin - 4.0 * dv / 6.0) / dv;
			c.g = 0;
			c.b = 1;
		}
		else {
			c.r = 1;
			c.g = 0;
			c.b = 1 + 6 * (vmin + 5.0 * dv / 6.0 - v) / dv;
		}
		break;
	case 5:
		c.r = (v - vmin) / (vmax - vmin);
		c.g = 1;
		c.b = 0;
		break;
	case 6:
		c.r = (v - vmin) / (vmax - vmin);
		c.g = (vmax - v) / (vmax - vmin);
		c.b = c.r;
		break;
	case 7:
		if (v < (vmin + 0.25 * dv)) {
			c.r = 0;
			c.g = 4 * (v - vmin) / dv;
			c.b = 1 - c.g;
		}
		else if (v < (vmin + 0.5 * dv)) {
			c.r = 4 * (v - vmin - 0.25 * dv) / dv;
			c.g = 1 - c.r;
			c.b = 0;
		}
		else if (v < (vmin + 0.75 * dv)) {
			c.g = 4 * (v - vmin - 0.5 * dv) / dv;
			c.r = 1 - c.g;
			c.b = 0;
		}
		else {
			c.r = 0;
			c.b = 4 * (v - vmin - 0.75 * dv) / dv;
			c.g = 1 - c.b;
		}
		break;
	case 8:
		if (v < (vmin + 0.5 * dv)) {
			c.r = 2 * (v - vmin) / dv;
			c.g = c.r;
			c.b = c.r;
		}
		else {
			c.r = 1 - 2 * (v - vmin - 0.5 * dv) / dv;
			c.g = c.r;
			c.b = c.r;
		}
		break;
	case 9:
		if (v < (vmin + dv / 3)) {
			c.b = 3 * (v - vmin) / dv;
			c.g = 0;
			c.r = 1 - c.b;
		}
		else if (v < (vmin + 2 * dv / 3)) {
			c.r = 0;
			c.g = 3 * (v - vmin - dv / 3) / dv;
			c.b = 1;
		}
		else {
			c.r = 3 * (v - vmin - 2 * dv / 3) / dv;
			c.g = 1 - c.r;
			c.b = 1;
		}
		break;
	case 10:
		if (v < (vmin + 0.2 * dv)) {
			c.r = 0;
			c.g = 5 * (v - vmin) / dv;
			c.b = 1;
		}
		else if (v < (vmin + 0.4 * dv)) {
			c.r = 0;
			c.g = 1;
			c.b = 1 + 5 * (vmin + 0.2 * dv - v) / dv;
		}
		else if (v < (vmin + 0.6 * dv)) {
			c.r = 5 * (v - vmin - 0.4 * dv) / dv;
			c.g = 1;
			c.b = 0;
		}
		else if (v < (vmin + 0.8 * dv)) {
			c.r = 1;
			c.g = 1 - 5 * (v - vmin - 0.6 * dv) / dv;
			c.b = 0;
		}
		else {
			c.r = 1;
			c.g = 5 * (v - vmin - 0.8 * dv) / dv;
			c.b = 5 * (v - vmin - 0.8 * dv) / dv;
		}
		break;
	case 11:
		c1.r = 200 / 255.0; c1.g = 60 / 255.0; c1.b = 0 / 255.0;
		c2.r = 250 / 255.0; c2.g = 160 / 255.0; c2.b = 110 / 255.0;
		c.r = (c2.r - c1.r) * (v - vmin) / dv + c1.r;
		c.g = (c2.g - c1.g) * (v - vmin) / dv + c1.g;
		c.b = (c2.b - c1.b) * (v - vmin) / dv + c1.b;
		break;
	case 12:
		c1.r = 55 / 255.0; c1.g = 55 / 255.0; c1.b = 45 / 255.0;
		/* c2.r = 200 / 255.0; c2.g =  60 / 255.0; c2.b =   0 / 255.0; */
		c2.r = 235 / 255.0; c2.g = 90 / 255.0; c2.b = 30 / 255.0;
		c3.r = 250 / 255.0; c3.g = 160 / 255.0; c3.b = 110 / 255.0;
		ratio = 0.4;
		vmid = vmin + ratio * dv;
		if (v < vmid) {
			c.r = (c2.r - c1.r) * (v - vmin) / (ratio*dv) + c1.r;
			c.g = (c2.g - c1.g) * (v - vmin) / (ratio*dv) + c1.g;
			c.b = (c2.b - c1.b) * (v - vmin) / (ratio*dv) + c1.b;
		}
		else {
			c.r = (c3.r - c2.r) * (v - vmid) / ((1 - ratio)*dv) + c2.r;
			c.g = (c3.g - c2.g) * (v - vmid) / ((1 - ratio)*dv) + c2.g;
			c.b = (c3.b - c2.b) * (v - vmid) / ((1 - ratio)*dv) + c2.b;
		}
		break;
	case 13:
		c1.r = 0 / 255.0; c1.g = 255 / 255.0; c1.b = 0 / 255.0;
		c2.r = 255 / 255.0; c2.g = 150 / 255.0; c2.b = 0 / 255.0;
		c3.r = 255 / 255.0; c3.g = 250 / 255.0; c3.b = 240 / 255.0;
		ratio = 0.3;
		vmid = vmin + ratio * dv;
		if (v < vmid) {
			c.r = (c2.r - c1.r) * (v - vmin) / (ratio*dv) + c1.r;
			c.g = (c2.g - c1.g) * (v - vmin) / (ratio*dv) + c1.g;
			c.b = (c2.b - c1.b) * (v - vmin) / (ratio*dv) + c1.b;
		}
		else {
			c.r = (c3.r - c2.r) * (v - vmid) / ((1 - ratio)*dv) + c2.r;
			c.g = (c3.g - c2.g) * (v - vmid) / ((1 - ratio)*dv) + c2.g;
			c.b = (c3.b - c2.b) * (v - vmid) / ((1 - ratio)*dv) + c2.b;
		}
		break;
	case 14:
		c.r = 1;
		c.g = (v - vmin) / dv;
		c.b = 0;
		break;
	case 15:
		if (v < (vmin + 0.25 * dv)) {
			c.r = 0;
			c.g = 4 * (v - vmin) / dv;
			c.b = 1;
		}
		else if (v < (vmin + 0.5 * dv)) {
			c.r = 0;
			c.g = 1;
			c.b = 1 - 4 * (v - vmin - 0.25 * dv) / dv;
		}
		else if (v < (vmin + 0.75 * dv)) {
			c.r = 4 * (v - vmin - 0.5 * dv) / dv;
			c.g = 1;
			c.b = 0;
		}
		else {
			c.r = 1;
			c.g = 1;
			c.b = 4 * (v - vmin - 0.75 * dv) / dv;
		}
		break;

	case 16:
		c.r = 255;
		c.g = 216;
		c.b = 0;
		break;
	}
	return(c);
}

#pragma endregion

void GiveUsage(char *cmd)
{
	/*
	Display the program usage information
	again if you are using command line, you are a king
	*/
	fprintf(stderr, "%s -h -f -s -d -D\n", cmd);
	fprintf(stderr, "   -h    this help message\n");
	fprintf(stderr, "   -f    full screen\n");
	fprintf(stderr, "   -s    stereo mode\n");
	fprintf(stderr, "   -d    debug mode\n");
	fprintf(stderr, "   -D    demo mode\n");
	exit(-1);
}




void skybox(void) {
	float x = 0;
	float y = 0;
	float z = 0;
	float width = 200;
	float height = 200;
	float length = 200;
	// Bind the BACK texture of the sky map to the BACK side of the cube
	glBindTexture(GL_TEXTURE_2D, textureSkyBox[0]);
	// Center the skybox
	x = x - width / 2;
	y = y - height / 2;
	z = z - length / 2;
	glBegin(GL_QUADS);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y, z);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height, z);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + height, z);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, textureSkyBox[1]);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z + length);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z + length);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z + length);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z + length);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, textureSkyBox[4]);
	glBegin(GL_QUADS);

	glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y, z + length);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y, z + length);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, textureSkyBox[5]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y + height, z);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y + height, z + length);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z + length);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + height, z);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, textureSkyBox[2]);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + height, z + length);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z + length);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z);

	glEnd();
	glBindTexture(GL_TEXTURE_2D, textureSkyBox[3]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y, z + length);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height, z + length);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z);
	glEnd();
}

GLuint LoadTexture(const char * filename, int width, int height) {
	GLuint texture;
	unsigned char * data;
	FILE* file;

	fopen_s(&file, filename, "rb");
	if (file == NULL) return 0;
	data = (unsigned char *)malloc(width * height * 3);
	fread(data, width * height * 3, 1, file);
	fclose(file);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
	free(data);
	return texture;
}

void FreeTexture(GLuint texture)
{
	glDeleteTextures(1, &texture);
}
