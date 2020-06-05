#include <stdio.h>
#include <stdint.h>

#include <math.h>

#include "../c.h"

#include "../sys.h"

#include "../algebra.h"
#include "../image.h"
#include "../telescope.h"

#include "../io/raw.h"
#include "../io/lpng.h"

#include "sample_data.h"

#define N_OBJECTS 6
#define EQUATORIAL_FIX 0
#define MAX_PATH_LEN 256

struct {
	EquatorialLocation *object;
	uint8_t colours[N_OBJECTS][3];
	Rect r;
	Observer o;
	Camera c;
	double jd_start;
	double jd_end;
	double jd_step;
	double DUT1;
	double DT;
} ctxt = {
	.colours = {
		{ UINT8_MAX, UINT8_MAX, UINT8_MAX },
		{ UINT8_MAX, 0, 0 },
		{ 0, UINT8_MAX, 0 },
		{ 0, 0, UINT8_MAX },
		{ UINT8_MAX, UINT8_MAX, 0 },
		{ UINT8_MAX, 0, UINT8_MAX }
	}
};

void
init()
{
	ctxt.object = sample_object;
	ctxt.o = sample_observer;
	ctxt.c = canon_EOS600D;
	ctxt.c.angle = samples_angle[0];
	ctxt.jd_start = sample_jd;
	ctxt.jd_end = sample_jd+0.25;
	ctxt.jd_step = 1. / 24. / 60.;
	ctxt.DUT1 = sample_DUT1;
	ctxt.DT = sample_DT;

	ctxt.c.f_mm = 14.0;
	ctxt.r = (Rect){
		/* these are not the actual camera image dimensions ! */
		.min = { .x = -ctxt.c.sdim.x/2 - ctxt.c.sdim.x%2, .y = -ctxt.c.sdim.y/2 - ctxt.c.sdim.y%2 },
		.max = { .x = ctxt.c.sdim.x/2, .y = ctxt.c.sdim.y/2 }
	};
}

int
main(int argc, char *argv[])
{
	char *prefix = (argc > 1) ? argv[1] : "", fn[MAX_PATH_LEN];

	Image *im;
	
	double t;
	HorizontalLocation hz;
	RealPoint pt;
	uint8_t *px;

	int i, j;

	init();

	EquatorialLocation cameq0 = hz2eq((HorizontalLocation){ .az = ctxt.c.angle.az, .z = ctxt.c.angle.z }, ctxt.o.geod, ctxt.jd_start, ctxt.DUT1, ctxt.DT);

	wtlog(0, "allocating image...\n");
	if ((im = newim(Irgb_uint8)) == nil) {
		wterror("cannot allocate memory\n");
		return -1;
	}

	wtlog(0, "allocating channel...\n");	
	if (initchan(im->chans[0], ctxt.r)) {
		wterror("cannot allocate memory\n");
		freeim(im);
		return -1;
	}
	if (initchan(im->chans[1], ctxt.r)) {
		wterror("cannot allocate memory\n");
		freeim(im);
		return -1;
	}
	if (initchan(im->chans[2], ctxt.r)) {
		wterror("cannot allocate memory\n");
		freeim(im);
		return -1;
	}

	wtlog(0, "calculating trajectory...\n");
	for (t = ctxt.jd_start, i = 0; t < ctxt.jd_end; t += ctxt.jd_step, i++) {
		if (EQUATORIAL_FIX) {
			/* move camera */
			hz = eq2hz(cameq0, ctxt.o.geod, t, ctxt.DUT1, ctxt.DT);
			ctxt.c.angle.az = hz.az;
			ctxt.c.angle.z = hz.z;
		}
		for (j = 0; j < N_OBJECTS; j++) {
			hz = eq2hz(ctxt.object[j], ctxt.o.geod, t, ctxt.DUT1, ctxt.DT);
			hz.z += saemundsson(ctxt.o, hz.z); /* atmospheric correction */
			pt = hz2pixel(&ctxt.c, hz);

			if ((i % 100) == 0) {
				wtlog(1, "object % 2d: [% 3.4f, % 3.4f]\t", j, rad2deg(hz.az), 90 - rad2deg(hz.z));
				wtlog(1, "[% 4.2f, % 4.2f] ", pt.x, pt.y);
			}

			if (isnan(pt.x) || isnan(pt.y))
				continue;

			if ((px = npixelptr(im, 0, pt)) != nil) {
				wtlog(1, "%d", j);
				wtlog(2, "r(% 4.0f, % 4.0f) = %d ", pt.x, pt.y, ctxt.colours[j][0]);
				*px = ctxt.colours[j][0];
			}
			if ((px = npixelptr(im, 1, pt)) != nil) {
				wtlog(1, ".");
				wtlog(2, "g(% 4.0f, % 4.0f) = %d ", pt.x, pt.y, ctxt.colours[j][1]);
				*px = ctxt.colours[j][1];
			}
			if ((px = npixelptr(im, 2, pt)) != nil) {
				wtlog(1, "o");
				wtlog(2, "b(% 4.0f, % 4.0f) = %d ", pt.x, pt.y, ctxt.colours[j][2]);
				*px = ctxt.colours[j][2];
			}
		}

	}

	wtlog(0, "writing image...\n");

	snprintf(fn, MAX_PATH_LEN, "%sRGB.raw", prefix);
	if (wtim(im, fn)) {
		wterror("cannot write to file\n");
		freeim(im);
		return -1;
	}

	snprintf(fn, MAX_PATH_LEN, "%sR.png", prefix);
	if (wtpng(im, 0, fn)) {
		wterror("cannot write to file\n");
		freeim(im);
		return -1;
	}
	snprintf(fn, MAX_PATH_LEN, "%sG.png", prefix);
	if (wtpng(im, 1, fn)) {
		wterror("cannot write to file\n");
		freeim(im);
		return -1;
	}
	snprintf(fn, MAX_PATH_LEN, "%sB.png", prefix);
	if (wtpng(im, 2, fn)) {
		wterror("cannot write to file\n");
		freeim(im);
		return -1;
	}

	freeim(im);
	return 0;		
}
