/* requires: stdint.h */

#ifndef __CASTRO_COLOUR_H__
#define __CASTRO_COLOUR_H__

#ifndef EPS
#define EPS 1e-6
#endif

#define sRGBred ( (CieXYZ) { .c.X = 0.412453, .c.Y = 0.212671 , .c.Z = 0.019334} )
#define sRGBgrn ( (CieXYZ) { .c.X = 0.357580, .c.Y = 0.715160, .c.Z = 0.119193} )
#define sRGBblu ( (CieXYZ) { .c.X = 0.180423, .c.Y = 0.072169, .c.Z = 0.950227} )
#define sRGBblk ( (CieXYZ) { .c.X = 0.001901, .c.Y = 0.002, .c.Z = 0.002178} )
#define sRGBwht ( (CieXYZ) { .c.X = 0.95047, .c.Y = 1.0, .c.Z = 1.08883} )

typedef union CieXYZ CieXYZ;
union CieXYZ {
	struct {
		double X;
		double Y;
		double Z;
	} c;
	double buf[3];
};

typedef union RGBc RGBc;
union RGBc {
	struct {
		double r;
		double g;
		double b;
	} c;
	double buf[3];
};


typedef struct Primcolour Primcolour;
struct Primcolour {
	CieXYZ xyz;
	double g;
	double (*gfun)(double u, double g);

	uint8_t depth;
	uint64_t minlevel;
	uint64_t maxlevel;

	uint64_t nlevels;
	double *u; 
};

void *newpcolour(uint8_t depth);
void freepcolour(Primcolour *pc);

void initpcolour(Primcolour *pc, CieXYZ xyz, uint64_t minlevel, uint64_t maxlevel, double g, double (*gfun)(double u, double g));

double gammasRGB(double u, double g);

#define pcolouri2u(pc, i) ( (pc)->u[(i)] )

uint64_t pcolouru2i(Primcolour *pc, double v);


typedef struct Colours Colours;
struct Colours {
	Primcolour *red;
	Primcolour *grn;
	Primcolour *blu;

	CieXYZ blk;
	CieXYZ wht;

	/* everything below here is set by the initialiser */
	RGBc X;
	RGBc Y;
	RGBc Z;

	double *Mt[3];
	double *invMt[3];
	double K[3]; /* white compensation to map RGBc {1,1,1} to white (wrong von Kries) */
};

void *newcolours(uint8_t depth);
void freecolours(Colours *cs);

int initcolours(Colours *cs, Primcolour *red, Primcolour *grn, Primcolour *blu, CieXYZ blk, CieXYZ wht);
int initsRGB(Colours *cs);

/* RGB{0,0,0} is XYZ{0,0,0}, RGB{1,0,0} is the red prim colour, and so forth */
void rgb2xyz_abs(Colours *cs, RGBc *rgb, CieXYZ *xyz);
void xyz2rgb_abs(Colours *cs, CieXYZ *xyz, RGBc *rgb);

/* RGB{0,0,0} is black, RGB{1,1,1} is white, RGB{1, 0, 0} is scaled relative to the red prim colour, and so forth */
void rgb2xyz_rel(Colours *cs, RGBc *rgb, CieXYZ *xyz);
void xyz2rgb_rel(Colours *cs, CieXYZ *xyz, RGBc *rgb);


#endif