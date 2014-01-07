/* Windows specific includes */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL\GL.h>

#include "glut.h"
#include <math.h>
#include <stdio.h>

#define PI 3.14159265359

float sunRadius = 0.1;

float dt = 0.0;
int nshow = 0;

double position[2];
double velocity[2];

float t = 0.0;
float GM = 0.0;
double r = 0.0;

void Euler(void)
{
	double accelerate[2];
	double r2 = position[0]*position[0] + position[1]*position[1];
	double r3 = r2*r2*r2;
	int i;

	for (i = 0; i < 2; ++i) {
		accelerate[i] = -GM*position[i]/r3;
		velocity[i] += accelerate[i]*dt;
		position[i] += velocity[i]*dt;
	}
	t += dt;
}

void Energy(void)
{
	double r2 = position[0]*position[0] + position[1]*position[1];
	double r3 = r2*r2*r2;

	double ex =
		0.5*velocity[0]*velocity[0] + GM/r3;
	double ey =
		0.5*velocity[1]*velocity[1] + GM/r3;
	printf("%f\n", ex+ey);
}

void Period(void)
{
	double T = 0.0;

	T = 2 * PI * r / sqrt( GM / r );

	printf("%f\n", T);
}

void EulerRichardson(void)
{
	double dt_2 = dt / 2.0;

	double accelerate[2];
	double v2 = velocity[0]*velocity[0] + velocity[1]*velocity[1];
	double v = v2*v2;
	double r2 = position[0]*position[0] + position[1]*position[1];
	double r3 = r2*r2*r2;

	double vxmid;
	double vymid;

	double xmid;
	double ymid;

	double vmid2;
	double vmid;

	double axmid;
	double aymid;

	accelerate[0] = -GM*velocity[0]/r3;
	accelerate[1] = -GM*velocity[1]/r3;

	vxmid = velocity[0] + accelerate[0] * dt_2;
	vymid = velocity[1] + accelerate[1] * dt_2;

	xmid = position[0] + velocity[0] * dt_2;
	ymid = position[1] + velocity[1] * dt_2;

	vmid2 = vxmid*vxmid + vymid*vymid;
	vmid = vmid2*vmid2;

	axmid = vmid*vxmid;
	aymid = -vmid*vymid;

	velocity[0] += axmid*dt;
	velocity[1] += aymid*dt;

	position[0] += vxmid*dt;
	position[1] += vymid*dt;

	t += dt;
}

int counter = 0;
void Redraw(void)
{
	Euler();
	++counter;

	if ((counter % nshow) == 0) {
		int i;
		const float radf = PI / 180.0;

		//Energy();
		Period();

		glBegin(GL_TRIANGLE_FAN);
		glColor3ub(255, 255, 0);

		for (i = 0; i < 360; ++i) {
			glVertex2d(-sunRadius * cos(i * radf) + 0, -sunRadius * sin(i * radf) + 0);
			glVertex2d( sunRadius * cos(i * radf) + 0, -sunRadius * sin(i * radf) + 0);
			glVertex2d( sunRadius * cos(i * radf) + 0,  sunRadius * sin(i * radf) + 0);
			glVertex2d(-sunRadius * cos(i * radf) + 0,  sunRadius * sin(i * radf) + 0);
		}
		glEnd();

		glBegin(GL_POINTS);
		glColor3ub(255, 255, 255);
		glVertex2d(position[0], position[1]);
		glEnd();

		glutSwapBuffers();
	}
}

void main(int argc, char** argv)
{
	const double WIDTH = 800;
	const double HEIGHT = 600;

	float ix;
	double x, y;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Orbits");

	glutIdleFunc(Redraw);
	glutDisplayFunc(Redraw);

	/* get state */
	GM = 4.0 * pow(PI, 2);
	t = 0;

	printf("time step = ");
	scanf("%f", &dt);

	printf("number of time steps between plots = ");
	scanf("%d", &nshow);

	printf("initial position x = ");
	scanf("%f", &ix);

	position[0] = ix;
	r = 2*position[0];
	x = (0.1*r + r)*800/600;
	y = (0.1*r + r);

	/* Setup OpenGL to render 2D */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-x, x, y, -y, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);

	position[1] = 0;
	velocity[0] = 0;

	printf("initial y velocity = ");
	scanf("%f", &velocity[1]);

	velocity[1] = sqrt(GM/r);

	glClear(GL_COLOR_BUFFER_BIT);
	glutMainLoop();
}
