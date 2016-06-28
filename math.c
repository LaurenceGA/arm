#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// for built in math functions

double degtorad(double x) {
	return x * (M_PI/180);
}

double radtodeg(double x) {
	return x * (180/M_PI);
}

double Sqrt(double x) {
	if (x >= 0) {
		return sqrt(x);
	} else {
		fprintf(stderr, "Square root must be >= 0\n");
		abort();
	}
}

double nlog(double x) {
	if (x > 0) {
		return log(x);
	} else {
		fprintf(stderr, "ln must be > 0\n");
		abort();
	}
}

double Log10(double x) {
	if (x > 0) {
		return log10(x);
	} else {
		fprintf(stderr, "Log must be > 0\n");
		abort();
	}
}
