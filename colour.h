/* requires: algebra.h */

#ifndef __CASTRO_COLOUR_H__
#define __CASTRO_COLOUR_H__

#ifndef COLOUR_EPS
#define COLOUR_EPS 1e-6
#endif

/* CIE xyY values */
typedef struct CIExyY CIExyY;
struct CIExyY {
	double x;
	double y;
	double Y;
};

/* RGB values */
typedef struct RGBc RGBc;
struct RGBc {
	double r;
	double g;
	double b;
};

/* RGB index */
typedef struct RGBi RGBi;
struct RGBi {
	int R;
	int G;
	int B;
};

typedef struct SCProfile SCProfile;
struct SCProfile {
	int depth;		/* depth per channel in bits */
	CIExyY red;
	CIExyY grn;
	CIExyY blu;
	CIExyY wht;
	double gR;
	double gG;
	double gB;
	double *gcR;
	double *gcG;
	double *gcB;
};

SCProfile *newscprofile(int depth, CIExyY red, CIExyY grn, CIExyY blu, CIExyY wht, double gR, double gG, double gB);
void freescprofile(SCProfile *cp);

SCProfile *newscprofile0(int depth);

RGBc xyY2rgbc(SCProfile *cp, CIExyY c);
CIExyY rgbc2xyY(SCProfile *cp, RGBc c);

#define rgbc2rgbc(cpin, cpout, c) ( xyY2rgbc(cpout, rgbc2xyY(cpin, c)) )

#define rgbc2i(cp, c) ( (RGBi){ .R = roundi(c.r * (double)(2<<cp->depth)), .G = roundi(c.g * (double)(2<<cp->depth)), .B = roundi(c.g * (double)(2<<cp->depth)) } )
#define i2rgbc(cp, c) ( (RGBc){ .r = cp->gcR[c.R], .g = cp->gcG[c.G], .b = cp->gcB[c.B] } )

#endif