/* Windows specific includes */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL\GL.h>

#include "glut.h"
#include <math.h>
#include <stdio.h>

/*
** The following is a port of the FORTRAN code from the textbook to C.
** Also the graphical plot has been ported to take advantage of OpenGL
** and GLUT.
**
** This program simulates the free fall of a styrofoam ball with no drag
** using Euler, EulerCromer, and EulerRichardson.
*/

#define WIDTH 800
#define HEIGHT 600

double dt;
double t;
double y;
double v;
double a;
double g;
int nshow;
double xmid;
double r;


void Euler(double *y, double *v, double *a, double g, double *t, double dt)
{
	*y += (*v)*dt;
	*a = -g;
	*v += (*a)*dt;
	*t += dt;
}


void EulerCromer(double *y, double *v, double *a, double g, double *t, double dt)
{
	*a = -g;
	*v += (*a)*dt;
	*y += (*v)*dt;
	*t += dt;
}


void EulerRichardson(double *y, double *v, double *a, double g, double *t, double dt)
{
	double vmid;
	double amid;

	*a = -g;
	vmid = (*v) + (*a)*(0.5*dt);
	amid = -g;
	*v += vmid*dt;
	*y += (*a)*dt;
	*t += dt;
}


void Initial(double *y, double *v, double *a, double *g, double *t, double *dt)
{
	*t = 0;
	*y = 20;
	*v = 0;
	*g = 9.8;
	*a = -(*g);
	printf("time step dt = ");
	scanf("%lf", dt);
}


void PrintTable(double y, double v, double a, double t, int *nshow)
{
	if ( t == 0 ) {
		printf("number of steps between output = ");
		scanf("%d", nshow);
		printf("time (s) y (m) velocity (m/s) accel (m/s^2)\n");
	}
	printf("%lf %lf %lf %lf\n", t, y, v, a);
}


void ShowParticle(double y, double t, double *xmid, double *r, int *nshow)
{
	if ( t == 0 ) {
		printf("number of steps between output = ");
		scanf("%d", nshow);

		/* Setup OpenGL to render 2D */
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, 11, -1, y + 1, -1.0, 1.0);
		glMatrixMode(GL_MODELVIEW);

		*xmid = 5.5;
		*r = 0.01;

		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		/* draw the baseline for the ground */
		glBegin(GL_LINES);
		glColor3ub(0, 255, 0);
		glVertex2f(0, 0);
		glVertex2f(11, 0);
		glEnd();
	}

	/* draw the ball */
	glBegin(GL_QUADS);
	glColor3ub(255, 255, 255);
	glVertex2f(*xmid - *r, y - *r);
	glVertex2f(*xmid + *r, y - *r);
	glVertex2f(*xmid + *r, y + *r);
	glVertex2f(*xmid - *r, y + *r);
	glEnd();
	glutSwapBuffers();
}


int i = 0;
int Update(void)
{
	if ( y >= 0 ) {
		//Euler(&y, &v, &a, g, &t, dt);
		EulerRichardson(&y, &v, &a, g, &t, dt);
		++i;
		if ((i % nshow) == 0) {
			PrintTable(y, v, a, t, &nshow);
			ShowParticle(y, t, &xmid, &r, &nshow);
		}
		return 1;
	}
	return 0;
}


void WrapUpdate(void)
{
	if (!Update())
	{
		/*exit(0);*/
		ShowParticle(y, t, &xmid, &r, &nshow);
	}
}


void main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("FreeFall");

	/* Initialize the model */
	Initial(&y, &v, &a, &g, &t, &dt);
	ShowParticle(y,t, &xmid, &r, &nshow);

	/* start the simulation */
	glutIdleFunc(WrapUpdate);
	glutDisplayFunc(WrapUpdate);
	glutMainLoop();
}

