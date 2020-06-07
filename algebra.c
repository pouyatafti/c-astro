#include <string.h>

#include "c.h"

#include "algebra.h"

int
gcd(int p, int q) {
	while (q != 0) {
		p %= q;
		p ^= q;
		q ^= p;
		p ^= q;
	}

	return p;
}

Rational
ratreduce(Rational a)
{
	int div, sgn = (a.num <0) ? (a.den < 0) ? 1 : -1 : (a.den < 0) ? -1 : 1;

	a.num = absl(a.num);
	a.den = absl(a.den);

	div = gcd(a.num, a.den);
	a.num /= div;
	a.den /= div;
	a.num *= sgn;

	return a;	
}

Lattice
lattice(Point u, Point v, Point o) {
	Lattice lc;

	int det;

	lc.u = u;
	lc.v = v;
	lc.o = o;

	det = u.x * v.y - u.y * v.x;

	lc.uix = ratreduce((Rational){ .num = v.y, .den = det });
	lc.uiy = ratreduce((Rational){ .num = -u.y, .den = det });
	lc.vix = ratreduce((Rational){ .num = -v.x, .den = det });
	lc.viy = ratreduce((Rational){ .num = u.x, .den = det });

	return lc;
}

int
PinLc(Point pt, Lattice lc)
{
	Point pti, ptii;

	pti.x = lc2ix(lc, pt);
	pti.y = lc2iy(lc,pt);
	ptii.x = i2lcx(lc, pti);
	ptii.y = i2lcy(lc, pti);

	return (pt.x == ptii.x) && (pt.y == ptii.y);
}

Point
roundlc(Lattice lc, RealPoint pt)
{
	Point pti, ptii;

	pti.x = roundi(lc2ix(lc, pt));
	pti.y = roundi(lc2iy(lc,pt));
	ptii.x = i2lcx(lc, pti);
	ptii.y = i2lcy(lc, pti);

	return ptii;	
}


double
dotprod(double *x, double *y, int dim)
{
	int i;
	double d;

	for (i = 0, d = 0; i < dim; ++i)
		d += *x++ * *y++;

	return d;
}

double *
vecadd(double *dest, double *x, double *y, double a, double b, double c, int dim)
{
	int i;
	double *d = dest;

	for (i = 0; i < dim; ++i, ++dest)
		*dest = a * *dest + b * *x++ + c * *y++;

	return d;
}

double *
matvecmult(double *dest, void *A, int transpose, double *x, double a, double b, int dim)
{
	int i, j;
	double *d = dest, **AA = (double **)A;

	if (transpose) {
		for (i = 0; i < dim; ++i) {
			dest[i] *= a;
			for (j = 0; j < dim; ++j)
				dest[i] += b * AA[j][i] * x[j];
		}
	} else {
		for (i = 0; i < dim; ++i) {
			dest[i] *= a;
			for (j = 0; j < dim; ++j)
				dest[i] += b * AA[i][j] * x[j];
		}
	}

	return d;
}

double *
bicgstab(
	double *x, void *A, double *b,
	double **work,
	double *(*matvec)(double *dest, void *A, int transpose, double *x, double a, double b, int dim),
	int dim, long maxiter, double eps0
)
{
	long i, ddim = dim*sizeof(double);
	double *r, *rnext, *rhat0, *p, *s, *Ap, *As, *swap, alpha, beta, om, rrhat0, rnextrhat0;

	r = work[0];
	rnext = work[1];
	rhat0 = work[2];
	p = work[3];
	s = work[4];
	Ap = work[5];
	As = work[6];

	memmove(r, b, ddim);
	matvec(r, A, 0, x, 1, -1, dim);
	memmove(rhat0, r, ddim);
	memmove(p, r, ddim);

	for (i = 0; i < maxiter; ++i) {
		rrhat0 = dotprod(r, rhat0, dim);
		matvec(Ap, A, 0, p, 0, 1, dim);
		alpha = rrhat0 / dotprod(Ap, rhat0, dim);
		vecadd(s, r, Ap, 0, 1, -alpha, dim);
		if (dotprod(s, s, dim) < eps0*eps0) {
			vecadd(x, x, p, 0, 1, alpha, dim);
			break;
		}
		matvec(As, A, 0, s, 0, 1, dim);
		om = dotprod(As, s, dim) / dotprod(As, As, dim);
		vecadd(x, p, s, 1, alpha, om, dim);
		vecadd(rnext, s, As, 0, 1, -om, dim);
		if (dotprod(rnext, rnext, dim) < eps0*eps0) {
			break;
		}
		rnextrhat0 = dotprod(rnext, rhat0, dim);
		beta = alpha/om*rnextrhat0/rrhat0;
		vecadd(p, rnext, Ap, beta, 1, -beta*om, dim);
		if (rnextrhat0 < 1e-6 && -rnextrhat0 < 1e-6) {
			memmove(rhat0, rnext, ddim);
			memmove(p, rnext, ddim);
		}
		swap = r;
		r = rnext;
		rnext = swap;		
	}

	return x;
}