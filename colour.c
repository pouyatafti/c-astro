#include <stdlib.h>
#include <math.h>

#include "c.h"

#include "algebra.h"
#include "colour.h"


SCProfile *
newscprofile(int depth, CIExyY red, CIExyY grn, CIExyY blu, CIExyY wht, double gR, double gG, double gB)
{
	SCProfile *cp;
	long i, n = 1 << (depth-1);
	double *gcR, *gcG, *gcB;

	if ((cp = calloc(1, sizeof(SCProfile))) == nil)
		return nil;

	cp->depth = depth;

	if ((gcR = cp->gcR = malloc(n * sizeof(double))) == nil) {
		freescprofile(cp);
		return nil;
	}

	if ((gcG = cp->gcG = malloc(n * sizeof(double))) == nil) {
		freescprofile(cp);
		return nil;
	}

	if ((gcB = cp->gcB = malloc(n * sizeof(double))) == nil) {
		freescprofile(cp);
		return nil;
	}

	cp->red = red;
	cp->grn = grn;
	cp->blu = blu;
	cp->wht = wht;

	cp->gR = gR;
	cp->gG = gG;
	cp->gB = gB;
	
	for (i = 0; i < n; i++) {
		*gcR++ = pow((double)i/(n-1),gR);
		*gcG++ = pow((double)i/(n-1),gG);
		*gcB++ = pow((double)i/(n-1),gB);
	}

	return cp;
}

SCProfile *
newscprofile0(int depth)
{
	double gR = 1.0, gG = 1.0, gB = 1.0;

	CIExyY red = { 0.6400, 0.3300, 0.212671 };
	CIExyY grn = { 0.3000, 0.6000, 0.715160 };
	CIExyY blu = { 0.1500, 0.0600, 0.072169 };
	CIExyY wht = { 0.3127, 0.3291, 1.0 };

	return newscprofile(depth, red, grn, blu, wht, gR, gG, gB);
}

void
freescprofile(SCProfile *cp)
{
	if (cp->gcR) free(cp->gcR);
	if (cp->gcG) free(cp->gcG);
	if (cp->gcB) free(cp->gcB);

	free(cp);
}

CIExyY
rgbc2xyY(SCProfile *cp, RGBc c)
{
	CIExyY xyY = {0,0,0};
	double X,Y,Z;

	X   =   c.r * ((cp->red).x/(cp->red).y)*(cp->red).Y +
		c.g * ((cp->grn).x/(cp->grn).y)*(cp->grn).Y +
		c.b * ((cp->blu).x/(cp->blu).y)*(cp->blu).Y;

	Y   =   c.r * (cp->red).Y +
		c.g * (cp->grn).Y + 
		c.b * (cp->blu).Y;

	Z   =   c.r * ((1-(cp->red).x-(cp->red).y)/(cp->red).y)*(cp->red).Y +
		c.g * ((1-(cp->grn).x-(cp->grn).y)/(cp->grn).y)*(cp->grn).Y +
		c.b * ((1-(cp->blu).x-(cp->blu).y)/(cp->blu).y)*(cp->blu).Y;

	if (X+Y+Z>COLOUR_EPS) {
		xyY.x = X/(X+Y+Z);
		xyY.y = Y/(X+Y+Z);
		xyY.Y = Y;
	}

	return xyY;
}

RGBc
xyY2rgbc(SCProfile *cp, CIExyY c)
{
	/* XXX not implemented */
	return (RGBc){ 0., 0., 0. };
}