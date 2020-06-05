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

/* RGB values (linear) */
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

typedef struct CProfile CProfile;
struct CProfile {
	int depth;		/* depth per channel in bits */
	CIExyY red;
	CIExyY grn;
	CIExyY blu;
	CIExyY wht;
	RGBc X;
	RGBc Y;
	RGBc Z;
	double gR;
	double gG;
	double gB;
	double *gcR;
	double *gcG;
	double *gcB;
	double (*gRfun)(double r, double gR);
	double (*gGfun)(double g, double gG);
	double (*gBfun)(double b, double gB);
};

CProfile *newcp(int depth);
void freecp(CProfile *cp);

void
initcp(
	CProfile *cp,
	CIExyY red, CIExyY grn, CIExyY blu, CIExyY wht,
	double gR, double gG, double gB,
	double (*gRfun)(double r, double gR),
	double (*gGfun)(double g, double gG),
	double (*gBfun)(double b, double gB)
);

void initsRGB(CProfile *cp);

double sRGBgamma(double u, double g);

RGBc xyY2RGBc(CProfile *cp, CIExyY c);
CIExyY RGBc2xyY(CProfile *cp, RGBc c);

RGBc i2RGBc(CProfile *cp, RGBi c);
RGBi c2RGBi(CProfile *cp, RGBc c);

#define xyY2X(xyY) ( (xyY).x/(xyY).y*(xyY).Y )
#define xyY2Y(xyY) ( (xyY).Y )
#define xyY2Z(xyY) ( (1 - (xyY).x - (xyY).y)/(xyY).y*(xyY).Y )

#define rgbc2rgbc(cpin, cpout, c) ( xyY2rgbc(cpout, rgbc2xyY(cpin, c)) )

#endif