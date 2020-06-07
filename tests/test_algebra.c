#include <math.h>
#include <stdio.h>

#include "../c.h"

#include "test.h"

#include "../algebra.h"

int
main()
{
	test_begin();

	int i, dim = 3, maxiter = dim * dim;
	double x[] = {1, 2, 3}, y[] = {0, 0, 0}, e[] = {10, 20, 30};
	double Amem[3][3] = { {2, 7, 6}, {9, 5, 1}, {4, 3, 8} }, *A[3];
	double workmem[7][3] = { {0} }, *work[7];
	double eps0 = 1e-6;

	for (i = 0; i < dim; ++i) A[i] = (double *)Amem + dim*i;
	for (i = 0; i < 7; ++i) work[i] = (double *)workmem + dim*i;	

	testrel(double, ==, dotprod(x, x, dim), 14.0);

	testrel(double, ==, matvecmult(e, A, 0, x, -1, 1, dim)[1], 2.0);

	testrel(double, <, bicgstab(y, A, x, work, &matvecmult, dim, maxiter, eps0)[0], INFINITY);
	testrel(double, <, matvecmult(e, A, 0, y, 0, 1, dim)[0], INFINITY);
	testrel(double, <, vecadd(e, e, x, 0, -1, 1, dim)[0], INFINITY);
	testrel(double, <, dotprod(e, e, dim), eps0*eps0);

	test_end();
}