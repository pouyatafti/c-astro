/* requires: algebra.h */

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
	int ri;
	int gi;
	int bi;
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

SCProfile *newscprofile(int depth, CIExyY red, CIExyY grn, CIExyY blu, double gR, double gG, double gB);
void freescprofile(SCProfile *cp);

SCProfile *newsRGB(int depth);

RGBc xyY2rgb(SCProfile *cp, CIExyY c);
CIExyY rgb2xyY(SCProfile *cp, RGBc c);

#define rgb2rgb(cpin, cpout, c) ( RGBc(cpout, CIExyY(cpin, c)) )

#define rgb2i(cp, c) ( (RGBi){ .ri = roundi(c.r * (double)(2<<cp->depth)), .gi = roundi(c.g * (double)(2<<cp->depth)), .bi = roundi(c.g * (double)(2<<cp->depth)) } )
#define i2rgb(cp, c) ( (RGBc){ .r = cp->gcR[c.ri], .g = cp->gcG[c.gi], .b = cp->gcB[c.bi] } )