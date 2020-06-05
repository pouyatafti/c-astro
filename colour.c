#include <stdlib.h>
#include <math.h>

#include "c.h"

#include "algebra.h"
#include "colour.h"


CProfile *
newcp(int depth)
{
	CProfile *cp;
	long n = 1 << (depth-1);

	if ((cp = calloc(1, sizeof(CProfile))) == nil)
		return nil;

	cp->depth = depth;

	if ((cp->gcR = malloc(n * sizeof(double))) == nil) {
		freecp(cp);
		return nil;
	}

	if ((cp->gcG = malloc(n * sizeof(double))) == nil) {
		freecp(cp);
		return nil;
	}

	if ((cp->gcB = malloc(n * sizeof(double))) == nil) {
		freecp(cp);
		return nil;
	}

	return cp;
}

void
freecp(CProfile *cp)
{
	if (cp->gcR) free(cp->gcR);
	if (cp->gcG) free(cp->gcG);
	if (cp->gcB) free(cp->gcB);

	free(cp);
}

void
initcp(
	CProfile *cp,
	CIExyY red, CIExyY grn, CIExyY blu, CIExyY wht,
	double gR, double gG, double gB,
	double (*gRfun)(double r, double gR),
	double (*gGfun)(double g, double gG),
	double (*gBfun)(double b, double gB)
)
{
	long i, n = 1 << (cp->depth-1);
	double *gcR = cp->gcR, *gcG = cp->gcG, *gcB = cp->gcB;
	double det;

	cp->red = red;
	cp->grn = grn;
	cp->blu = blu;
	cp->wht = wht;

	det = xyY2X(red) * (xyY2Z(blu) * xyY2Y(grn) - xyY2Z(grn) * xyY2Y(blu))
		- xyY2Y(red) * (xyY2Z(blu) * xyY2X(grn) - xyY2Z(grn) * xyY2X(blu))
		+ xyY2Z(red) * (xyY2Y(blu) * xyY2X(grn) - xyY2Y(grn) * xyY2X(blu));

	cp->X.r = (xyY2Z(blu) * xyY2Y(grn) - xyY2Z(grn) * xyY2Y(blu)) / det;
	cp->X.g = -(xyY2Z(blu) * xyY2X(grn) - xyY2Z(grn) * xyY2X(blu)) / det;
	cp->X.b = (xyY2Y(blu) * xyY2X(grn) - xyY2Y(grn) * xyY2X(blu)) / det;

	cp->Y.r =-(xyY2Z(blu) * xyY2Y(red) - xyY2Z(red) * xyY2Y(blu)) / det;
	cp->Y.g = (xyY2Z(blu) * xyY2X(red) - xyY2Z(red) * xyY2X(blu)) / det;
	cp->Y.b = -(xyY2Y(blu) * xyY2X(red) - xyY2Y(red) * xyY2X(blu)) / det;

	cp->Z.r = (xyY2Z(grn) * xyY2Y(red) - xyY2Z(red) * xyY2Y(grn)) / det;
	cp->Z.g =-(xyY2Z(grn) * xyY2X(red) - xyY2Z(red) * xyY2X(grn)) / det;
	cp->Z.b = (xyY2Y(grn) * xyY2X(red) - xyY2Y(red) * xyY2X(grn)) / det;

	cp->gR = gR;
	cp->gG = gG;
	cp->gB = gB;
	
	cp->gRfun = gRfun ? gRfun : pow;
	cp->gGfun = gGfun ? gGfun : pow;
	cp->gBfun = gBfun ? gBfun : pow;

	for (i = 0; i < n; i++) {
		*gcR++ = (*cp->gRfun)((double)i/(n-1), gR);
		*gcG++ = (*cp->gGfun)((double)i/(n-1), gG);
		*gcB++ = (*cp->gBfun)((double)i/(n-1), gB);
	}
}

void
initsRGB(CProfile *cp)
{
	CIExyY red = { 0.6400, 0.3300, 0.212671 };
	CIExyY grn = { 0.3000, 0.6000, 0.715160 };
	CIExyY blu = { 0.1500, 0.0600, 0.072169 };
	CIExyY wht = { 0.3127, 0.3291, 1.0 };

	initcp(cp, red, grn, blu, wht, 1.0, 1.0, 1.0, &sRGBgamma, &sRGBgamma, &sRGBgamma);
}

double
sRGBgamma(double u, double g /* not used */)
{
	return u <= 0.0031308 ? 323.0*u/25.0 : (211.0*pow(u, 5.0/12.0)-11.0) / 200.0;
}

CIExyY
RGBc2xyY(CProfile *cp, RGBc c)
{
	CIExyY xyY = {0,0,0};
	double X,Y,Z;

	X = c.r * xyY2X(cp->red) +
		c.g * xyY2X(cp->grn) +
		c.b * xyY2X(cp->blu);

	Y = c.r * xyY2Y(cp->red) +
		c.g * xyY2Y(cp->grn) +
		c.b * xyY2Y(cp->blu);

	Z = c.r * xyY2Z(cp->red) +
		c.g * xyY2Z(cp->grn) +
		c.b * xyY2Z(cp->blu);

	if (X+Y+Z>COLOUR_EPS) {
		xyY.x = X/(X+Y+Z);
		xyY.y = Y/(X+Y+Z);
		xyY.Y = Y;
	}

	return xyY;
}

RGBc
xyY2RGBc(CProfile *cp, CIExyY c)
{
	RGBc rgbc;

	rgbc.r = xyY2X(c) * cp->X.r +
		xyY2Y(c) * cp->Y.r +
		xyY2Z(c) * cp->Z.r;

	rgbc.g = xyY2X(c) * cp->X.g +
		xyY2Y(c) * cp->Y.g +
		xyY2Z(c) * cp->Z.g;

	rgbc.b = xyY2X(c) * cp->X.b +
		xyY2Y(c) * cp->Y.b +
		xyY2Z(c) * cp->Z.b;

	return rgbc;
}

RGBc
i2RGBc(CProfile *cp, RGBi c)
{
	return (RGBc){ .r = cp->gcR[c.R], .g = cp->gcG[c.G], .b = cp->gcB[c.B] };
}

RGBi
c2RGBi(CProfile *cp, RGBc c)
{
	RGBi rgbi;
	long n = 1 << (cp->depth-1);
	long l, m, h;
	double *u, v, d;

	u = cp->gcR;
	v = c.r;
	for (l = 0, m = n >> 1, h = n-1; l < h;) {
		if (v > *(u+m)) {
			l = m+1;
		} else {
			h = m;
		}
		m = (l+h)/2;
	}
	rgbi.R = m;

	u = cp->gcG;
	v = c.g;
	for (l = 0, m = n >> 1, h = n-1; l < h;) {
		if (v > *(u+m)) {
			l = m+1;
		} else {
			h = m;
		}
		m = (l+h)/2;
	}
	rgbi.G = m;

	u = cp->gcB;
	v = c.b;
	for (l = 0, m = n >> 1, h = n-1; l < h;) {
		if (v > *(u+m)) {
			l = m+1;
		} else {
			h = m;
		}
		m = (l+h)/2;
	}
	rgbi.B = m;

	return rgbi;
}