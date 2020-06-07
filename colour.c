#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "c.h"

#include "algebra.h"
#include "colour.h"


int updatecolours(Colours *cs);

void *
newpcolour(uint8_t depth)
{
	Primcolour *pc;
	
	if (!(pc = calloc(1, sizeof(Primcolour)))) return nil;

	pc->depth = depth;
	pc->nlevels = 1<<(depth-1);

	if (!(pc->u = malloc(pc->nlevels*sizeof(double)))) {
		free(pc);
		return nil;
	}

	return pc;
}

void
freepcolour(Primcolour *pc)
{
	if (pc->u) free(pc->u);
	free(pc);
}

void
initpcolour(Primcolour *pc, CieXYZ xyz, uint64_t minlevel, uint64_t maxlevel, double g, double (*gfun)(double u, double g))
{
	uint64_t i;
	double *u = pc->u;

	pc->xyz = xyz;

	pc->minlevel = minlevel;
	pc->maxlevel = maxlevel <= 0 ? pc->nlevels -1 : maxlevel;

	pc->g = g;
	pc->gfun = gfun ? gfun : pow;

	for (i = 0; i < pc->nlevels; ++i)
		*u++ = (*pc->gfun)((double)(i < pc->minlevel ? 0 : i > pc->maxlevel ? pc->maxlevel : i-pc->minlevel)/(pc->maxlevel), g);
}

double
gammasRGB(double u, double g /* not used */)
{
	return u <= 0.0031308 ? 323.0*u/25.0 : (211.0*pow(u, 5.0/12.0)-11.0) / 200.0;
}

uint64_t
pcolouru2i(Primcolour *pc, double v)
{
	uint64_t l, m, h, n = pc->nlevels;
	double *u = pc->u;

	/* binary search */
	for (l = 0, m = n >> 1, h = n-1; l < h;) {
		if (v > *(u+m)) {
			l = m+1;
		} else {
			h = m;
		}
		m = (l+h)/2;
	}

	return m;
}



void *
newcolours(uint8_t depth)
{
	Colours *cs;

	if (!(cs = calloc(1, sizeof(Colours)))) return nil;

	if (!(cs->red = newpcolour(depth))) {
		freecolours(cs);
		return nil;
	}

	if (!(cs->grn = newpcolour(depth))) {
		freecolours(cs);
		return nil;
	}

	if (!(cs->blu = newpcolour(depth))) {
		freecolours(cs);
		return nil;
	}

	return cs;
}

void
freecolours(Colours *cs)
{
	if (cs->red) freepcolour(cs->red);
	if (cs->grn) freepcolour(cs->grn);
	if (cs->blu) freepcolour(cs->blu);

	free(cs);
}

int
initcolours(Colours *cs, Primcolour *red, Primcolour *grn, Primcolour *blu, CieXYZ blk, CieXYZ wht)
{
	initpcolour(cs->red, red->xyz, red->minlevel, red->maxlevel, red->g, red->gfun);
	initpcolour(cs->grn, grn->xyz, grn->minlevel, grn->maxlevel, grn->g, grn->gfun);
	initpcolour(cs->blu, blu->xyz, blu->minlevel, blu->maxlevel, blu->g, blu->gfun);

	cs->blk = blk;
	cs->wht = wht;

	return updatecolours(cs);
}

int
initsRGB(Colours *cs)
{
	initpcolour(cs->red, sRGBred, 0, cs->red->nlevels-1, 1.0, &gammasRGB);
	initpcolour(cs->grn, sRGBgrn, 0, cs->grn->nlevels-1, 1.0, &gammasRGB);
	initpcolour(cs->blu, sRGBblu, 0, cs->blu->nlevels-1, 1.0, &gammasRGB);

	cs->blk = sRGBblk;
	cs->wht = sRGBwht;

	return updatecolours(cs);	
}

int
updatecolours(Colours *cs)
{
	double workmem[7][3] = {{0}}, buf[3], *work[7];
	int i;

	cs->Mt[0] = cs->red->xyz.buf;
	cs->Mt[1] = cs->grn->xyz.buf;
	cs->Mt[2] = cs->blu->xyz.buf;

	for (i = 0; i < 7; ++i) work[i] = (double *)workmem + 3*i;
	buf[0] = 1; buf[1] = buf[2] = 0;
	bicgstab(cs->X.buf, cs->Mt, buf, work, &matvecmult, 3, 9, EPS);

	for (i = 0; i < 7; ++i) work[i] = (double *)workmem + 3*i;
	buf[1] = 1; buf[0] = buf[2] = 0;
	bicgstab(cs->Y.buf, cs->Mt, buf, work, &matvecmult, 3, 9, EPS);

	for (i = 0; i < 7; ++i) work[i] = (double *)workmem + 3*i;
	buf[2] = 1; buf[0] = buf[1] = 0;
	bicgstab(cs->Z.buf, cs->Mt, buf, work, &matvecmult, 3, 9, EPS);

	cs->invMt[0] = cs->X.buf;
	cs->invMt[1] = cs->Y.buf;
	cs->invMt[2] = cs->Z.buf;

	vecadd(buf, cs->wht.buf, cs->blk.buf, 0, 1, -1, 3);
	matvecmult(cs->K, cs->invMt, 1, buf, 0, 1, 3);

	return 0;
}

void
rgb2xyz_abs(Colours *cs, RGBc *rgb, CieXYZ *xyz)
{
	xyz->c.X = cs->X.c.r * rgb->c.r + cs->X.c.g * rgb->c.g + cs->X.c.b * rgb->c.b;
	xyz->c.Y = cs->Y.c.r * rgb->c.r + cs->Y.c.g * rgb->c.g + cs->Y.c.b * rgb->c.b;
	xyz->c.Z = cs->Z.c.r * rgb->c.r + cs->Z.c.g * rgb->c.g + cs->Z.c.b * rgb->c.b;
}

void
xyz2rgb_abs(Colours *cs, CieXYZ *xyz, RGBc *rgb)
{
	rgb->c.r = cs->red->xyz.c.X * xyz->c.X + cs->red->xyz.c.Y * xyz->c.Y + cs->red->xyz.c.Z * xyz->c.Z;
	rgb->c.g = cs->grn->xyz.c.X * xyz->c.X + cs->grn->xyz.c.Y * xyz->c.Y + cs->grn->xyz.c.Z * xyz->c.Z;
	rgb->c.b = cs->blu->xyz.c.X * xyz->c.X + cs->blu->xyz.c.Y * xyz->c.Y + cs->blu->xyz.c.Z * xyz->c.Z;
}

void
rgb2xyz_rel(Colours *cs, RGBc *rgb, CieXYZ *xyz)
{
	xyz->c.X = cs->K[0]*(cs->X.c.r * rgb->c.r + cs->X.c.g * rgb->c.g + cs->X.c.b * rgb->c.b) + cs->blk.c.X;
	xyz->c.Y = cs->K[1]*(cs->Y.c.r * rgb->c.r + cs->Y.c.g * rgb->c.g + cs->Y.c.b * rgb->c.b) + cs->blk.c.Y;
	xyz->c.Z = cs->K[2]*(cs->Z.c.r * rgb->c.r + cs->Z.c.g * rgb->c.g + cs->Z.c.b * rgb->c.b) + cs->blk.c.Z;
}

void
xyz2rgb_rel(Colours *cs, CieXYZ *xyz, RGBc *rgb)
{
	rgb->c.r = ( cs->red->xyz.c.X * (xyz->c.X-cs->blk.c.X) + cs->red->xyz.c.Y * (xyz->c.Y-cs->blk.c.Y) + cs->red->xyz.c.Z * (xyz->c.Z-cs->blk.c.Z) ) / cs->K[0];
	rgb->c.g = ( cs->grn->xyz.c.X * (xyz->c.X-cs->blk.c.X) + cs->grn->xyz.c.Y * (xyz->c.Y-cs->blk.c.Y) + cs->grn->xyz.c.Z * (xyz->c.Z-cs->blk.c.Z) ) / cs->K[1];
	rgb->c.b = ( cs->blu->xyz.c.X * (xyz->c.X-cs->blk.c.X) + cs->blu->xyz.c.Y * (xyz->c.Y-cs->blk.c.Y) + cs->blu->xyz.c.Z * (xyz->c.Z-cs->blk.c.Z) ) / cs->K[2];
}
