/*
** FreeFall
*/

#include <stdio.h>
#include <math.h>

/*
** The following is a port of the code from the textbook to C.
** This program simulates the free fall of an object with
** air resistance using the EulerRichardson method.
*/

struct {
	double x;
	double y;
	double y0;
	double vx;
	double vy;
	double t;
	double g;
	double vt2;
	double dt;
	double dt_2;
	double C;
	int nshow;
} data;


void Initialize(void)
{
	double vt;

	data.t = -0.132;
	data.y0 = 10;
	data.y = data.y0;
	data.vx = 00;
	data.vy = 0;
	data.x = 0;
	data.C = 0.1;

	printf("terminal velocity = ");
	scanf("%lf", &vt);

	data.vt2 = vt*vt;
	data.dt = 0.001;
	data.dt_2 = 0.5*data.dt;

	data.g = 9.8;
}


void EulerRichardson(void)
{
	double v2 = data.vx*data.vx + data.vy*data.vy;
	double v = sqrt(v2);
	double ax = -data.C*v*data.vx;
	double ay = -data.g - data.C*data.vy;
	double vxmid = data.vx + ax*data.dt_2;
	double vymid = data.vy + ay*data.dt_2;
	double vmid2 = vxmid*vxmid + vymid*vymid;
	double vmid = sqrt(vmid2);

	/* Fd = v^2 */
	/*vmid *= vmid;*/

	double axmid = -data.C*vmid*vxmid;

	/* Modified for problem 3.2.b */
	double Fd = -data.g*data.C*(1 - sqrt(data.vt2));
	/*double Fd = -data.g*data.C*(1 - v*v / data.vt2);*/
	double aymid = -data.g - Fd*vymid;

	data.vx += axmid*data.dt;
	data.vy += aymid*data.dt;
	data.x += vxmid*data.dt;
	data.y += vymid*data.dt;

	data.t += data.dt;
}


void PrintTable(void)
{
	if (data.t < 0) {
		double showtime = 0.1;
		data.nshow = (int)(showtime/data.dt);
		printf("time displacement\n");
	}
	printf("%lf %lf\n", data.t, data.y0 - data.y);
}


int main(int argc, char **argv) {
	int counter = 0;

	Initialize();
	PrintTable();

	while (data.t <= 0) {
		EulerRichardson();
	}
	PrintTable();

	while (data.t <= 0.8) {
		++counter;
		EulerRichardson();
		if ((counter % data.nshow) == 0) {
			PrintTable();
		}
	}
	PrintTable();

	return 0;
}

