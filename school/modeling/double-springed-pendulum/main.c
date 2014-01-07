/*
** Double Pendulum with Springs Model
** 
** This program simulates a spring loaded double pendulum. It
** uses the Runge Kutta 4 method and OpenGL for visualization.
*/

/* Windows specific includes */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL\GL.h>
#include <mmsystem.h>

#include "glut.h"
#include <math.h>
#include <stdio.h>

#define N	8
float gravity = 9.8;

/* Control the simulation manually */
int pause = 0;
int mstep = 0;
float timestep = 0.03;

/* State of the model */
double state[N] = {0};

struct {
	/* Fully extended length of spring */
	int length;

	/* Spring stiffness constant*/
	float k;

	/* Endpoints of the line representing the spring */
	double X1, Y1;
	double X2, Y2;

	/* Color of the spring */
	GLubyte color[3];
}
spring1, spring2;

struct {
	float mass;

	/* Top left corner of the mass */
	double X, Y;

	/* Mass is square */
	double side;

	/* Color of the mass */
	GLubyte color[3];
}
mass1, mass2;


void UpdateModel(void);

/* Fills the state array used in rk4 */
void SetState(void)
{
	/* Mass1's position */
	state[0] = 250 - mass1.side * 0.5;
	state[1] = gravity * (mass1.mass + mass2.mass) / spring1.k
		+ spring1.length + spring1.Y1;

	/* Mass2's position */
	state[2] = state[0];
	state[3] = gravity * (mass2.mass / spring2.k + (mass1.mass + mass2.mass) / spring1.k)
		+ spring1.length + spring2.length + spring1.Y1;

	/* Both masses are at rest */
	state[4] = 0;
	state[5] = 0;
	state[6] = 0;
	state[7] = 0;
}

/* Sets the default initial state of the model */
void InitializeModel(void)
{
	mass1.mass = 2.5;
	mass2.mass = 5;

	mass1.side = 10;
	mass2.side = 10;

	/* Mass1 is red */
	mass1.color[0] = 255;
	mass1.color[1] = 0;
	mass1.color[2] = 0;

	/* Mass2 is blue */
	mass2.color[0] = 0;
	mass2.color[1] = 0;
	mass2.color[2] = 255;

	spring1.k = 1.0;
	spring2.k = 1.0;

	/* Both springs are white */
	spring1.color[0] = 255;
	spring1.color[1] = 255;
	spring1.color[2] = 255;

	spring2.color[0] = 255;
	spring2.color[1] = 255;
	spring2.color[2] = 255;

	spring1.length = 50;
	spring2.length = 50;

	/* Set the fixed point that spring1 is attached onto */
	spring1.X1 = 250 - mass1.side * 0.5;
	spring1.Y1 = 100;

	/* Set the initial state by positioning the masses and placing them at rest */
	SetState();
	UpdateModel();
}

/* Used in the rungekutta4 function to solve the system of equations */
void Solve(double *input, double *output)
{
	double temp = 0.0;

  double x = input[0] - spring1.X1;
  double y = input[1] - spring1.Y1;
  double L1 = sqrt(x * x + y * y);

  double x2 = input[2] - input[0];
  double y2 = input[3] - input[1];
  double L2 = sqrt(x2 * x2 + y2 * y2);

	/* U1x' = V1x */
  output[0] = input[4];

	/* U1y' = V1y */
  output[1] = input[5];

	/* U2x' = V2x */
  output[2] = input[6];

	/* U2y' = V2y */
  output[3] = input[7];

	/* V1x = -(k1 / m1) * (L1 - R1) * (x / L1) + (k2 / m1) * (L2 - R2) * (x2 / L2) */
	temp = -(spring1.k / mass1.mass)* (L1 - spring1.length) * (x / L1);
	temp = temp + (spring2.k / mass1.mass) * (L2 - spring2.length) * (x2 / L2);
  output[4] = temp;

	/* V1y = g - (k1 / m1) * (L1 - R1) * (y / L1) + (k2 / m1) * (L2 - R2) * (y2 / L2) */
	temp = gravity - (spring1.k / mass1.mass) * (L1 - spring1.length) * (y / L1);
	temp = temp + (spring2.k / mass1.mass) * (L2 - spring2.length) * (y2 / L2);
  output[5] = temp;

	/* V2x = -(k2 / m2) * (L2 - R2) * (x2 / L2) */
  output[6] = -(spring2.k / mass2.mass)* (L2 - spring2.length) * (x2 / L2);

  /* V2y = g - (k2 / m2) * (L2 - R2) * (y2 / L2) */
	output[7] = gravity - (spring2.k / mass2.mass)* (L2 - spring2.length) * (y2 / L2);
}

/* A converted version of the rk4sys fortran subroutine. it
   takes the timestep and a function to solve the system with */
void rungekutta4(double t, double *X, void (*solve)(double*, double*))
{
	int i;
	double k1[N] = {0};
	double k2[N] = {0};
	double k3[N] = {0};
	double k4[N] = {0};
	double temp[N] = {0};

	solve(X, k1);
	for ( i = 0; i < 8; ++i ) {
		temp[i] = X[i] + k1[i] * t / 2;
	}
	solve(temp, k2);
	for ( i = 0; i < 8; ++i ) {
		temp[i] = X[i] + k2[i] * t / 2;
	}
	solve(temp, k3);
	for ( i = 0; i < 8; ++i ) {
		temp[i] = X[i] + k3[i] * t;
	}
	solve(temp, k4);
	for (i = 0; i < 8; i++) {
			X[i] = X[i] + (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]) * t / 6;
	}
}

/* Moves the values from the state array back into the mass and spring structs */
void UpdateModel(void) {
	const double HALF_SIDE = mass1.side * 0.5;

	/* Copy the mass positions; subtract a half side to center it */
	mass1.X = state[0] - HALF_SIDE;
	mass1.Y = state[1] - HALF_SIDE;
	mass2.X = state[2] - HALF_SIDE;
	mass2.Y = state[3] - HALF_SIDE;

	/* Copy the spring positions. we do this by taking the center of the connecting
	   masses as the endpoints of the springs. spring1's upper endpoint is fixed */
	spring1.X2 = mass1.X + HALF_SIDE;
	spring1.Y2 = mass1.Y + HALF_SIDE;

	/* Spring2's upper endpoint is connected to mass1 and the other to mass2 */
	spring2.X1 = mass1.X + HALF_SIDE;
	spring2.Y1 = mass1.Y + HALF_SIDE;
	spring2.X2 = mass2.X + HALF_SIDE;
	spring2.Y2 = mass2.Y + HALF_SIDE;
}

void OnKeyDown(unsigned char key, int x, int y)
{
	switch (key) {
		case 'c':
			glClear(GL_COLOR_BUFFER_BIT);
			glutSwapBuffers();
			break;

		/* Reset to predefined initial state */
		case 'r':
			InitializeModel();
			break;

		/* Change Mass 1 */
		case '1':
			printf("Input a new value for mass 1 [%f]: ", mass1.mass);
			scanf("%f", &mass1.mass);
			break;

		/* Change Mass 2 */
		case '2':
			printf("Input a new value for mass 2 [%f]: ", mass2.mass);
			scanf("%f", &mass2.mass);
			break;

		/* Change Gravity */
		case 'g':
			printf("Input a new value for gravity [%f]: ", gravity);
			scanf("%f", &gravity);
			break;

		/* Display Menu */
		case 'h':
			printf("Commands:\n");
			printf("[c] - clears the screen\n");
			printf("[r] - reset the simulation\n");
			printf("[g] - change gravity\n");
			printf("[1] - change mass 1\n");
			printf("[2] - change mass 2\n");
			printf("[p] - pause simulation\n");
			printf("[ ] - advance paused simulation one step\n");
			printf("[s] - set model variables\n");
			printf("[t] - set timestep\n");
			printf("[q] - quit simulation\n");
			break;

		/* Set the initial state */
		case 's':
			printf("Input -1 to use default value\n");
			printf("Input a new length for spring 1 [50]: ");
			scanf("%d", &spring1.length);
			if ( spring1.length < 0 ) {
				spring1.length = 50;
			}
			printf("Input a new value for spring constant 1 [1.0]: ");
			scanf("%f", &spring1.k);
			if ( spring1.k < 0 ) {
				spring1.k = 1.0;
			}
			printf("Input a new length for spring 2 [50]: ");
			scanf("%d", &spring2.length);
			if ( spring2.length < 0 ) {
				spring2.length = 50;
			}
			printf("Input a new value for spring constant 2 [1.0]: ");
			scanf("%f", &spring2.k);
			if ( spring2.k < 0 ) {
				spring2.k = 1.0;
			}
			printf("Input a new value for mass 1 [2.5]: ");
			scanf("%f", &mass1.mass);
			if ( mass1.mass < 0 ) {
				mass1.mass = 2.5;
			}
			printf("Input a new value for mass 2 [5.0]: ");
			scanf("%f", &mass2.mass);
			if ( mass2.mass < 0 ) {
				mass2.mass = 5.0;
			}
			SetState();
			UpdateModel();
			break;

		/* Toggle Pause Sim */
		case 'p':
			pause = !pause;
			break;

		/* Step once */
		case ' ':
			mstep = 1;
			break;

		/* Set timestep */
		case 't':
			printf("Input a new value for timestep [0.03]: ");
			scanf("%f", &timestep);
			if ( timestep < 0 ) {
				timestep = 0.03;
			}
			break;

		/* Quit */
		case 'q':
			glutDestroyWindow(glutGetWindow());
			exit(0);

		/* Null */
		default:
			;
			break;
	}
}

/* Determines whether or not a mass is being dragged */
int drag = 0;

/* Pauses while dragging a mass */
int dragpause = 0;

/* Offsets into the state array the position to the mass being dragged */
int offset = 0;

/* Catches mouse clicks and does some simple collision detection
   against each mass. It then sets up the necessary flags for the
	 motion callback to drag that mass */
void HitTest(int button, int buttonState, int x, int y)
{
	/* Only check for left clicks */
	if ( button != GLUT_LEFT_BUTTON ) {
		return;
	}
	if ( buttonState == GLUT_DOWN ) {
		/* Hit test mass1 */
		if ( (x >= mass1.X) && (x <= mass1.X + mass1.side) ) {
			drag = 1;
			offset = 0;
			dragpause = 1;
		}
		/* Hit test mass2 */
		if ( (x >= mass2.X) && (x <= mass2.X + mass2.side) ) {
			drag = 1;
			offset = 2;
			dragpause = 1;
		}
	/* Unpause when we stop dragging */
	} else if ( drag ) {
		drag = 0;
		dragpause = 0;
	}
}

/* Catches when the mouse moves and update the position of the
   mass that is being dragged and then updates the model */
void DragMass(int x, int y)
{
	if ( drag ) {
		state[offset + 0] = x;
		state[offset + 1] = y;
		UpdateModel();
	}
}

/* Used to limit the renderer from running too fast */
DWORD lastTime = 0;
DWORD thisTime = 0;

void Redraw(void)
{
	thisTime = timeGetTime();
	if ( lastTime - thisTime > 300 ) {
		/* Step the model and then update the spring and mass structs */
		if ( !dragpause && (!pause || mstep) ) {
			rungekutta4(timestep, state, Solve);
			UpdateModel();
			if ( mstep > 0 ) {
				mstep = 0;
			}
		}

		/* erase the screen */
		glClear(GL_COLOR_BUFFER_BIT);

		/* draw the masses */
		glBegin(GL_QUADS);
			glColor3ubv(mass1.color);
			glVertex2f(mass1.X, mass1.Y);
			glVertex2f(mass1.X + mass1.side, mass1.Y);
			glVertex2f(mass1.X + mass1.side, mass1.Y + mass1.side);
			glVertex2f(mass1.X, mass1.Y + mass1.side);

			glColor3ubv(mass2.color);
			glVertex2f(mass2.X, mass2.Y);
			glVertex2f(mass2.X + mass2.side, mass2.Y);
			glVertex2f(mass2.X + mass2.side, mass2.Y + mass2.side);
			glVertex2f(mass2.X, mass2.Y + mass2.side);
		glEnd();

		/* draw the springs */
		glBegin(GL_LINES);
			glColor3ubv(spring1.color);
			glVertex2f(spring1.X1, spring1.Y1);
			glVertex2f(spring1.X2, spring1.Y2);

			glColor3ubv(spring2.color);
			glVertex2f(spring2.X1, spring2.Y1);
			glVertex2f(spring2.X2, spring2.Y2);
		glEnd();

		glutSwapBuffers();
		lastTime = thisTime;
	}
}

void main(int argc, char** argv)
{
	const int WIDTH = 500;
	const int HEIGHT = 500;

	/* Prime the timer */
	lastTime = timeGetTime();
	thisTime = lastTime;

	InitializeModel();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Double Pendulum with Springs");

	glutIdleFunc(Redraw);
	glutDisplayFunc(Redraw);
	glutKeyboardFunc(OnKeyDown);
	glutMouseFunc(HitTest);
	glutMotionFunc(DragMass);

	/* Setup OpenGL to render 2D */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, WIDTH, HEIGHT, 0.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);

	glutMainLoop();
}
