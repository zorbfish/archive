/*
** This program is a md simulation that utilizes the Verlet integration method
*/

/* Windows specific includes */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL\GL.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* must be included AFTER stdlib.h to avoid exit conflict */
#include "glut.h"

#define _USE_MATH_DEFINES
#include <math.h>

#define RUN_PROB82C
#define WIDTH 800
#define HEIGHT 600

typedef struct Molecule
{
	double x, y; /* position */
	double vx, vy; /* velocity vector */
	double ax, ay; /* acceleration vector */
} Molecule;


/* global model state */
struct
{
	int N; /* number of particles in model */
	int steps; /* total steps run */

	double dt; /* time step */
	double dt2; /* squared time step */
	double t; /* runtime of the model */
	double runtime; /* length of time to run the model */
	double interval; /* time in between displays */

	double xCellSize; /* horizontal length of a screen cell */
	double yCellSize; /* vertical length of a screen cell */

	double pe; /* potential energy of the model */
	double ke; /* kinetic energy of the model */
	double keaccum; /* total kinetic energy over the run */

	double modVel; /* scales the velocities of the molecules in the model */
	double modCell; /* scales the size of the cell: >= 1 */

	int lookat; /* index to the current molecule focused on; -1 if not */
	double r; /* radius of the circle representing each molecule */
	Molecule mols[128]; /* state of each of the model's molecules */

	FILE *energy; /* records the total energy over the run */
	FILE *kinetic; /* records the kinetic energy over the run */
	FILE *potential; /* records the potential energy over the run */
	FILE *temperature; /* records the temperature over the run */
} data;


#ifdef RUN_PROB82C
FILE *nout = NULL;
void RecordMoleculeCountInLeft(void)
{
	int count = 0;
	int i;
	for (i = 0; i < data.N; ++i) {
		Molecule *m = data.mols + i;
		if (m->x <= (data.xCellSize * 0.5))
			++count;
	}

	fprintf(nout, "%lf %d\n", data.t, count);
}
void CloseNOUT(void)
{
	fclose(nout);
}
#endif


/* lookup tables to speed up draw */
double lsin[360];
double lcos[360];


/*
** Basic framework of the simulation is derived directly from
** the source code provided in the textbook on pages 220-226
*/


FILE *fmomentum = NULL;
void CloseMomentum(void)
{
	fclose(fmomentum);
}

void CheckMomentum(void)
{
	double vxsum = 0;
	double vysum = 0;
	double vxcm;
	double vycm;
	int i;

	if ( data.t == 0 ) {
		fmomentum=fopen("momentum.txt", "w");
		atexit(CloseMomentum);
	}

	for (i = 0; i < data.N; ++i) {
		Molecule *m = data.mols + i;
		vxsum += m->vx;
		vysum += m->vy;
	}

	vxcm = vxsum / data.N;
	vycm = vysum / data.N;

	for (i = 0; i < data.N; ++i) {
		Molecule *m = data.mols + i;

		m->vx -= vxcm;
		m->vy -= vycm;
	}

	fprintf(fmomentum, "%lf %lf %lf %lf %lf\n",
		data.t, vxcm, vycm, vxsum, vysum);
}


/* Determines the closest seperation between two molecules */
double Seperation(double ds, double L)
{
	if ( ds > 0.5 * L ) {
		return ds - L;
	} else if ( ds < -0.5 * L ) {
		return ds + L;
	}
	return ds;
}


/* Calculates the force upon two bodies (molecules) based on the distance */
void Force(double dx, double dy, double *fx, double *fy, double *pot)
{
	double r2 = dx*dx + dy*dy;
	double rm2 = 1/r2;
	double rm6 = rm2*rm2*rm2;

	/* compute the components of the f in the x and y directions */
	double f_over_r = 24*rm6*(2*rm6 - 1)*rm2;
	*fx = f_over_r*dx;
	*fy = f_over_r*dy;

	/* calculate the potential energy */
	*pot = 4*(rm6*rm6 - rm6);
}


/* Updates the acceleration of each molecule based on the forces acting upon each. */
void Accelerate(void)
{
	int i;
	for ( i = 0; i < data.N; ++i ) {
		Molecule *m = data.mols + i;
		m->ax = m->ay = 0;
	}
	data.pe = 0;

	for ( i = 0; i < data.N - 1; ++i ) {
		Molecule *mi = data.mols + (i);
		int j;
		for ( j = i + 1; j < data.N; ++j ) {
			Molecule *mj = data.mols + (j);
			double fx = 0;
			double fy = 0;
			double pot = 0;

			double dx = Seperation(mi->x - mj->x, data.xCellSize);
			double dy = Seperation(mi->y - mj->y, data.yCellSize);

			Force(dx, dy, &fx, &fy, &pot);
			mi->ax += fx;
			mi->ay += fy;
			mj->ax -= fx;
			mj->ay -= fy;
			data.pe += pot;
		}
	}
}


/* Wraps molecule positions around each cell */
double PBC(double pos, double L)
{
	if ( pos < 0 ) {
		return pos + L;
	} else if ( pos > L ) {
		return pos - L;
	}
	return pos;
}


/* Updates the state of each molecule */
void Verlet(void)
{
	int i;
	for ( i = 0; i < data.N; ++i ) {
		Molecule *m = data.mols + i;

		double xnew = m->x + m->vx*data.dt + 0.5*m->ax*data.dt2;
		double ynew = m->y + m->vy*data.dt + 0.5*m->ay*data.dt2;

		m->vx += 0.5*m->ax*data.dt;
		m->vy += 0.5*m->ay*data.dt;
		m->x = PBC(xnew, data.xCellSize);
		m->y = PBC(ynew, data.yCellSize);
	}

	Accelerate();
	data.ke = 0;
	for ( i = 0; i < data.N; ++i ) {
		Molecule *m = data.mols + i;
		
		m->vx += 0.5*m->ax*data.dt;
		m->vy += 0.5*m->ay*data.dt;
		data.ke += m->vx*m->vx + m->vy*m->vy;
	}

	data.ke *= 0.5;
	data.t += data.dt;
}


/* Draws the image of all of the molecules. Extracted from Redraw so
   that the code may be reused for the neighboring cells around the
	central cell */
void DrawImage(double xoffset, double yoffset)
{
	int i;
	for ( i = 0; i < data.N; ++i ) {
		Molecule *m = data.mols + i;
		int j;
			
		/* Translate in place the position of the molecule */
		double x = m->x - xoffset;
		double y = m->y - yoffset;

		/* First molecule is magenta */
		if ( i == 0 ) {
			glColor3ub(255, 0, 255);
		} else {
			glColor3ub(0, 0, 0);
		}

		/* Draw the molecule */
		glBegin(GL_TRIANGLE_FAN);
		for ( j = 0; j < 360; ++j ) {
			glVertex2d(-data.r * lcos[j] + x, -data.r * lsin[j] + y);
			glVertex2d( data.r * lcos[j] + x, -data.r * lsin[j] + y);
			glVertex2d( data.r * lcos[j] + x,  data.r * lsin[j] + y);
			glVertex2d(-data.r * lcos[j] + x,  data.r * lsin[j] + y);
		}
		glEnd();
	}
}


/* Performs a timestep of the simulation, outputs data about
   the state of the simulation at that timestep, and finally
	redraws the screen if the display interval has been reached. */
int elapsed = 0;
int momentum = 0;
void Redraw(void)
{
	double dt = 0.0;

#ifdef RUN_PROB82C
	RecordMoleculeCountInLeft();
#endif
	Verlet();
	++data.steps;
	++elapsed;
	++momentum;

	if (data.lookat != -1) {
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(-data.mols[data.lookat].x + 0.5 * data.xCellSize,
			-data.mols[data.lookat].y + 0.5 * data.yCellSize, 0.0);
	}

	dt = data.t - data.dt;
	if (data.energy)
		fprintf(data.energy, "%lf %lf\n", dt, data.ke + data.pe);
	if (data.kinetic)
		fprintf(data.kinetic, "%lf %lf\n", dt, data.ke);
	if (data.potential)
		fprintf(data.potential, "%lf %lf\n", dt, data.pe);

	data.keaccum += data.ke;
	if (data.temperature)
		fprintf(data.temperature, "%lf %lf\n", dt, (data.keaccum / data.steps) / data.N);

	if ( momentum > 100 ) {
		CheckMomentum();
		momentum = 0;
	}

	if ( elapsed >= data.interval ) {
		double lx = 0.5 * data.xCellSize;
		double ly = 0.5 * data.yCellSize;

		glClear(GL_COLOR_BUFFER_BIT);

		/* draw each of the cell images */
		DrawImage( 1.5 * data.xCellSize,  1.5 * data.yCellSize);
		DrawImage( 0.5 * data.xCellSize,  1.5 * data.yCellSize);
		DrawImage(-0.5 * data.xCellSize,  1.5 * data.yCellSize);
		DrawImage( 1.5 * data.xCellSize,  0.5 * data.yCellSize);
		DrawImage( 0.5 * data.xCellSize,  0.5 * data.yCellSize);
		DrawImage(-0.5 * data.xCellSize,  0.5 * data.yCellSize);
		DrawImage( 1.5 * data.xCellSize, -0.5 * data.yCellSize);
		DrawImage( 0.5 * data.xCellSize, -0.5 * data.yCellSize);
		DrawImage(-0.5 * data.xCellSize, -0.5 * data.yCellSize);

		/* Draw each of the cell borders */
		glBegin(GL_LINES);
		glColor3ub(64, 64, 64);

		/* First the horizontals */
		glVertex2d(-2*lx-lx, -ly);
		glVertex2d( 2*lx+lx, -ly);
		glVertex2d(-2*lx-lx,  ly);
		glVertex2d( 2*lx+lx,  ly);

		/* Then the verticals */
		glVertex2d(-lx, -2*ly-ly);
		glVertex2d(-lx,  2*ly+ly);
		glVertex2d( lx, -2*ly-ly);
		glVertex2d( lx,  2*ly+ly);
		glEnd();

		/* Now the border around all of that */
		glBegin(GL_LINE_STRIP);
		glColor3ub(64, 64, 64);
		glVertex2d(-2*lx-lx, -2*ly-ly);
		glVertex2d( 2*lx+lx, -2*ly-ly);
		glVertex2d( 2*lx+lx,  2*ly+ly);
		glVertex2d(-2*lx-lx,  2*ly+ly);
		glVertex2d(-2*lx-lx, -2*ly-ly);
		glEnd();

		glutSwapBuffers();
		elapsed = 0;
	}

	/* exit only if a defined runtime was specified */
	if ( (data.runtime >= 0) && (data.t >= data.runtime) ) exit(0); 
}


/* Sets up the orthogonal projection as well as adjusts that
	projection so that the rendered image is not distorted */
void SetProjection(double r)
{
	double x = 0.0;
	double y = 0.0;
	double aspect = WIDTH/HEIGHT;

	if ( WIDTH > HEIGHT ) {
		x = aspect * r;
		y = r;
	} else {
		x = r;
		y = aspect * r;
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-x, x, y, -y, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}


void OnKeyDown(unsigned char key, int x, int y)
{
	double lx = 0.5 * data.xCellSize;
	double ly = 0.5 * data.yCellSize;
	int i;

	(void)x;
	(void)y;

	switch (key) {
		case 'c':
			glClear(GL_COLOR_BUFFER_BIT);
			glutSwapBuffers();
			break;

		/* Remove Nth Molecule */
		case 'd':
			--data.N;
			break;

		/* Scale the velocities of all molecules */
		case 's':
			printf("please input a velocity scaling factor: ");
			scanf("%lf", &data.modVel);

			for ( i = 0; i < data.N; ++i ) {
				Molecule *m = data.mols + i;
				m->vx *= data.modVel;
				m->vy *= data.modVel;
			}
			break;

		/* look at a specific molecule */
		case 'l':
			printf("look at which molecule [1 .. %d]: ", data.N);
			scanf("%d", &data.lookat);
			if ( data.lookat != -1 && (data.lookat >= 1) && (data.lookat <= data.N) ) {
				/* make sure we're not zoomed out */
				if ( lx >= ly ) {
					SetProjection(lx);
				} else {
					SetProjection(ly);
				}
				data.modCell = 1.0;

				/* decrement so it acts as an array index */
				--data.lookat;
			} else {
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
			}
			break;

		/* zoom out the central cell */
		case 'o':
			data.modCell += 0.1;
			if ( data.modCell >= 3.0 ) {
				data.modCell = 3.0;
			}
			if ( lx >= ly ) {
				SetProjection(data.modCell * lx);
			} else {
				SetProjection(data.modCell * ly);
			}
			break;

		/* zoom in the central cell */
		case 'i':
			data.modCell -= 0.1;
			if ( data.modCell < 1.0 ) {
				data.modCell = 1.0;
			}
			if ( lx >= ly ) {
				SetProjection(data.modCell * lx);
			} else {
				SetProjection(data.modCell * ly);
			}
			break;

		/* Display Menu */
		case 'h':
			printf("Commands:\n");
			printf("[c] - clear the screen\n");
			printf("[d] - remove the nth molecule\n");
			printf("[s] - scale the velocities of all molecules\n");
			printf("[o] - zoom out central cell\n");
			printf("[i] - zoom in on central cell\n");
			printf("[l] - look at a particular molecule\n");
			printf("[q] - quit simulation\n");
			break;

		case 'q':
			data.runtime = 0;
			break;

		/* Null */
		default:
			;
			break;
	}
}


/* Straight forward 'prompt n response' input gatherer */
void GetKeyedData(void)
{
	int choice = 0;
	int i;

	printf("number of molecules: ");
	scanf("%d", &data.N);
	printf("width of cell: ");
	scanf("%lf", &data.xCellSize);
	printf("height of cell: ");
	scanf("%lf", &data.yCellSize);
	printf("time step: ");
	scanf("%lf", &data.dt);
	printf("runtime: ");
	scanf("%lf", &data.runtime);
	printf("display interval (in timesteps): ");
	scanf("%lf", &data.interval);

	printf("Select the following: \n");
	printf("[1] - Random positions\n");
	printf("[2] - Random positions and velocities\n");
	do {
		scanf("%d", &choice);
		if ((choice != 1) && (choice != 2)) {
			printf("please choose option 1 or 2\n");
		}
	} while ((choice != 1) && (choice != 2));

	/* generate positional data */
	srand(clock());
	for ( i = 0; i < data.N; ++i ) {
		Molecule *m = data.mols + i;

		/* generate the whole position */
		m->x = rand() % (int)(0.5*data.xCellSize);
		m->y = rand() % (int)(0.5*data.yCellSize);

		/* generate a fractional position */
		m->x += (rand() % 90) / 100.0;
		m->y += (rand() % 90) / 100.0;

		/* negate a few so we don't bunch up in one spot */
		if (rand() % 2) m->x *= -1;
		if (rand() % 2) m->y *= -1;

		m->vx = m->vy = 0.0;
		m->ax = m->ay = 0.0;
	}

	/* generate velocity data (if chosen) */
	if ( choice == 2 ) {
		for ( i = 0; i < data.N; ++i ) {
			Molecule *m = data.mols + i;

			/* keep the velocities small v < 2 */
			m->vx = rand() % 1;
			m->vy = rand() % 1;

			/* generate a fractional velocity */
			m->vx += (rand() % 90) / 100.0;
			m->vy += (rand() % 90) / 100.0;

			/* negate a few so they are not all going the same direction */
			if (rand() % 2) m->vx *= -1;
			if (rand() % 2) m->vy *= -1;
		}
	}
}


/* Initializes the molecule data from an external file. Returns 1 on success. */
int ReadDataFile(const char *file)
{
	FILE *fp = NULL;
	int i;
	
	/* Load default configuration */
	fp = fopen(file, "r");
	if (!fp) {
		return 0;
	}

	fscanf(fp, "%d %lf %lf %lf %lf %lf",
		&data.N, &data.dt, &data.runtime, &data.interval, &data.xCellSize, &data.yCellSize);
	for ( i = 0; i < data.N; ++i ) {
		Molecule *m = data.mols + i;
		fscanf(fp, "%lf %lf %lf %lf",
			&m->x, &m->y, &m->vx, &m->vy);
		m->ax = m->ay = 0.0;
	}
	fclose(fp);

	return 1;
}


/* Closes the output files the model opens at the beginning of a run */
void CloseFiles(void)
{
	fclose(data.energy);
	fclose(data.kinetic);
	fclose(data.potential);
	fclose(data.temperature);
}


void InitializeModel(void)
{
	double lx = 0.0;
	double ly = 0.0;
	int choice = 0;
	char plot[32] = {0};

	printf("Start model from which input method?\n");
	printf("[1] - From the keyboard\n");
	printf("[2] - From an external file\n");

	/* Wait for the input method to be chosen */
	do {
		scanf("%d", &choice);
		if ((choice != 1) && (choice != 2)) {
			printf("please choose option 1 or 2\n");
		}
	} while ((choice != 1) && (choice != 2));

	if (choice == 1) {
		GetKeyedData();
	} else {
		char file[512] = {0};
		int ok = 0;

		/* Prompt and attempt read until successful */
		while (!ok) {
			do {
				int read = 0;
				printf("which file? ");
				read = scanf("%s", file);
			} while (file[0] == '/0');

			ok = ReadDataFile(file);
			if (!ok) {
				printf("%s read failure\n", file);
			}
		}
	}

	/* Initialize model attributes not from input */
	data.t = data.steps = 0;
	data.pe = data.ke = data.keaccum = 0;
	data.dt2 = data.dt*data.dt;
	data.modVel = data.modCell = 1;
	data.lookat = -1;

	/* Setup OpenGL to render 2D */
	lx = 0.5 * data.xCellSize;
	ly = 0.5 * data.yCellSize;
	if ( lx >= ly ) {
		SetProjection(data.modCell * lx);
		data.r = 0.008 * data.xCellSize;
	} else {
		SetProjection(data.modCell * ly);
		data.r = 0.008 * data.yCellSize;
	}

	/* open the output files for plottable data if requested */
	printf("Record plot data from this run? [y/n]: ");
	do {
		scanf("%s", &plot);
		if ( plot[0] != 'y' && plot[0] != 'n' ) {
			printf("please select yes or no\n");
		}
	} while (plot[0] != 'y' && plot[0] != 'n');
	if ( plot[0] == 'y' ) {
		data.energy = fopen("energy.txt", "w");
		data.kinetic = fopen("kinetic.txt", "w");
		data.potential = fopen("potential.txt", "w");
		data.temperature = fopen("temperature.txt", "w");
		atexit(CloseFiles);
	} else {
		data.energy = NULL;
		data.kinetic = NULL;
		data.potential = NULL;
		data.temperature = NULL;
	}

#ifdef RUN_PROB82C
	nout = fopen("left.txt", "w");
	atexit(CloseNOUT);
#endif
}


void main(int argc, char** argv)
{
	int i;
	const double RADF = M_PI / 180.0;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Molecular Dynamics");

	/* Setup the lookup tables for sin/cos */
	for ( i = 0; i < 360; ++i ) {
		lsin[i] = sin(i * RADF);
		lcos[i] = cos(i * RADF);
	}

	InitializeModel();
	CheckMomentum();
	
	/* Make sure the client area of the window is empty */
	glClearColor(0.8, 0.8, 0.8, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glutSwapBuffers();

	glutIdleFunc(Redraw);
	glutDisplayFunc(Redraw);
	glutKeyboardFunc(OnKeyDown);
	glutMainLoop();
}

