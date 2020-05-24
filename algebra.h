#ifndef __CASTRO_ALGEBRA_H__
#define __CASTRO_ALGEBRA_H__

typedef struct Point Point;
struct Point {
	int x;
	int y;
};

typedef struct RealPoint RealPoint;
struct RealPoint {
	double x;
	double y;
};

typedef struct Rect Rect;
struct Rect {
	Point min; /* first point included */
	Point max; /* first point excluded */
};

typedef struct RealRect RealRect;
struct RealRect {
	RealPoint min; /* first point included */
	RealPoint max; /* first point excluded */
};

/* utility functions for Rect and Point */
#define dPx(p1,p2) ((p2).x-(p1).x)
#define dPy(p1,p2) ((p2).y-(p1).y)
#define dRx(r) (dPx((r).min, (r).max))
#define dRy(r) (dPy((r).min, (r).max))
#define PinR(p, r) ( ((p).x >= (r).min.x) && ((p).y >= (r).min.y) && ((p).x < (r).max.x) && ((p).y < (r).max.y) )

typedef struct Rational Rational;
struct Rational {
	int num;
	int den;
};

#define absl(a) ( (a) > 0 ? (a) : -(a) )
#define roundi(a) ( (a) < 0 ? (int)(a-0.5) : (int)(a+0.5) )

int gcd(int p, int q);

Rational ratreduce(Rational a);

#define i2rat(i) = ( (Rational){ .num = i, .det = 1 } )

#define ratinv(a) = ( (Rational){ .num = (a).den, .den = (a).num } )
#define ratneg(a) = ( (Rational){ .num = -(a).num, .den = (a).den } )

#define ratmult(a, b) ( (Rational){ .num = (a).num * (b).num, .den = (a).den * (b).den } )
#define ratdiv(a, b) ( (Rational){ .num = (a).num * (b).den, .den = (a).den * (b).num } )

#define ratadd(a, b) ( (Rational){ .num = (a).num*(b).den + (a).den*(b).num, .den = (a).den*(b).den } )
#define ratsub(a, b) ( (Rational){ .num = (a).num*(b).den - (a).den*(b).num, .den = (a).den*(b).den } )

typedef struct Lattice Lattice;
struct Lattice {
	Point u; /* u, v are the rows of the lattice matrix, not the basis vectors */
	Point v;
	Point o; /* offset */
	Rational uix;
	Rational uiy;
	Rational vix;
	Rational viy;
};

Lattice lattice(Point u, Point v, Point offset);

int PinLc(Point pt, Lattice lc); /* does point belong to lattice? */
Point roundlc(Lattice lc, RealPoint pt); /* nearest lattice point */

/* get lattice coordinates from Z^2 integer and back (for integer points the inverse assumes valid lattice points) */
#define i2lcx(l, p) ( (l).u.x*(p).x + (l).u.y*(p).y + (l).o.x )
#define i2lcy(l, p) ( (l).v.x*(p).x + (l).v.y*(p).y + (l).o.y )

#define lc2ix(l, p) ( (l).uix.num*((p).x-(l).o.x)/(l).uix.den + (l).uiy.num*((p).y-(l).o.y)/(l).uiy.den )
#define lc2iy(l, p) ( (l).vix.num*((p).x-(l).o.x)/(l).vix.den + (l).viy.num*((p).y-(l).o.y)/(l).viy.den )

Lattice lcinverse(Lattice lc);

#endif

