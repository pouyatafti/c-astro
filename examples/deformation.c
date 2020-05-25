#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <math.h>

#include "../c.h"

#include "../sys.h"

#include "../algebra.h"
#include "../image.h"
#include "../telescope.h"

#include "../io/lpng.h"

#include "sample_data.h"

#define MAX_PATH_LEN 256

struct {
	Observer o;
	Camera c;
	Rect r;
	double jd_start;
	double jd_end;
	double jd_step;
} ctxt;

void init();

Image *newmono16(Rect r);
void fillnoise16(Image *im);
void deform16(Observer *o, Camera *c, double jd_src, double jd_dest, Image *src, Image *dest); 

Image *
newmono16(Rect r)
{
	Image *im;

	if ((im = newim(Imono_uint16)) == nil) {
		return nil;
	}

	if (initchan(*im->chans, r)) {
		freeim(im);
		return nil;
	}

	return im;		
}

void
fillnoise16(Image *im)
{
	uint16_t *px;
	Point i;
	Rect r;
	int c;

	for (c = 0; c < im->nchans; c++) {
		r = im->chans[c]->r;
		for (i.x = r.min.x; i.x < r.max.x; i.x++) {
			for (i.y = r.min.y; i.y < r.max.y; i.y++) {
				if((px = pixelptr(im, c, i)))
					*px = rand();
			}
		}
	}	
}

void
deform16(Observer *o, Camera *c, double jd_src, double jd_dest, Image *src, Image *dest)
{
	uint16_t *pxs, *pxd;
	HorizontalLocation hz;
	EquatorialLocation eq;
	RealPoint i, pt;
	Rect r;
	int chan;

	if (src->nchans > dest->nchans)
		return;

	for (chan = 0; chan < src->nchans; chan++) {
		r = dest->chans[chan]->r;
		for (i.x = r.min.x; i.x < r.max.x; i.x++) {
			for (i.y = r.min.y; i.y < r.max.y; i.y++) {
				hz = pixel2hz(c, i);
				eq = hz2eq(hz, o->geod, jd_dest, 0, 0);
				hz = eq2hz(eq, o->geod, jd_src, 0, 0);
				pt = hz2pixel(c, hz);

				if (isnan(pt.x) || isnan(pt.y))
					continue;

				if ((pxs = npixelptr(src, chan, pt)))
					if ((pxd = npixelptr(dest, chan, i)))
						*pxd = *pxs;		
			}
		}
	}
}

void
init()
{
	ctxt.o = sample_observer;
	ctxt.c = canon_EOS600D;
	ctxt.r = (Rect){
		/* these are not the actual camera image dimensions ! */
		.min = { .x = -(ctxt.c.sdim.x/10)/2 - (ctxt.c.sdim.x/10)%2, .y = -(ctxt.c.sdim.y/10)/2 - (ctxt.c.sdim.y/10)%2 },
		.max = { .x = (ctxt.c.sdim.x/10)/2, .y = (ctxt.c.sdim.y/10)/2 }
	};
	ctxt.c.angle = samples_angle[0];
	ctxt.jd_start = sample_jd;
	ctxt.jd_end = sample_jd+0.25;
	ctxt.jd_step = 1. / 24. / 60.;
}

int
main(int argc, char *argv[])
{
	int i, ret = 0;
	double jd;

	char *errmsg, *prefix, fn[MAX_PATH_LEN];

	Image *src = nil, *dest = nil;

	init();

	prefix = (argc > 1) ? argv[1] : "";

	if ((src = newmono16(ctxt.r)) == nil) {
		errmsg = "deformation: cannot allocate memory\n";
		ret = -1;
		goto main_error;
	}

	if ((dest = newmono16(ctxt.r)) == nil) {
		errmsg = "deformation: cannot allocate memory\n";
		ret = -1;
		goto main_error;
	}

	fillnoise16(src);
	for (jd = ctxt.jd_start, i = 0; jd <= ctxt.jd_end; jd += ctxt.jd_step, i++) {
		deform16(&ctxt.o, &ctxt.c, ctxt.jd_start, jd, src, dest);

		snprintf(fn, MAX_PATH_LEN, "%s%05d.png", prefix, i);
		if (wtpng(dest, 0, fn)) {
			wterror("deformation: cannot write to file\n");
			ret = -2;
			goto main_error;
		}
	}

	goto main_cleanup;

main_error:
	wterror(errmsg);

main_cleanup:
	if (src) freeim(src);
	if(dest) freeim(dest);

	return ret;
}