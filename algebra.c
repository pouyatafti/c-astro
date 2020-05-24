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