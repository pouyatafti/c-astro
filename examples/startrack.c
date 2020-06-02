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

#define N_OBJECTS 6

#define EQUATORIAL_FIX 0

struct {
	EquatorialLocation object[N_OBJECTS];
	uint8_t colours[N_OBJECTS][3];
	Observer o;
	Camera c;
	double jd_start;
	double jd_end;
	double jd_step;
	double DUT1;
	double DT;
} ctxt = {
	.object = {
		{
			/* Polaris Aa */
			.ra = hms2rad(2, 31, 49.09),
			.dec = degms2rad(89, 15, 50.8)
		},
		{
			/* Andromeda Galaxy */
			.ra = hms2rad(0, 42, 44.33),
			.dec = degms2rad(41, 16, 7.5)
		},
		{
			/* Errai */
			.ra = hms2rad(23, 39, 21.69),
			.dec = degms2rad(77, 37, 58)
		},
		{
			/* Antares */
			.ra = hms2rad(16, 29, 24.45970),
			.dec = degms2rad(-26, 25, 55.2094)
		},
		{
			/* Kochab */
			.ra = hms2rad(14, 50, 42.32580),
			.dec = degms2rad(74, 9, 19.8142)
		},
		{
			/* Sirius */
			.ra = hms2rad(6, 45, 8.91728),
			.dec = degms2rad(-16, 42, 58.0171)
		}
	},

	.colours = {
		{ UINT8_MAX, UINT8_MAX, UINT8_MAX },
		{ UINT8_MAX, 0, 0 },
		{ 0, UINT8_MAX, 0 },
		{ 0, 0, UINT8_MAX },
		{ UINT8_MAX, UINT8_MAX, 0 },
		{ UINT8_MAX, 0, UINT8_MAX }
	},

	.o = {
		.geod = {
			/* Munich, Germany */
			.lat = degms2rad(48, 8, 14.74),
			.lng = degms2rad(11, 34, 31.76),
			.alt_m = 524
		},
		.polarm = { .x = 0, .y = 0 },
		.P_kPa = NOMINAL_PRESSURE_kPa,
		.T_K = NOMINAL_TEMPERATURE_K,
		.rh = 1.0
	},		
	
	.c = {
		/* Canon EOS 600D */
		.f_mm = 24,
		.sdim_mm = { .x = 22.3, .y = 14.9 },
		.sdim = { .x = 5344, .y = 3516 },
		.angle = {
/*
			.az = degms2rad(44, 31, 20),
			.z = PIo2 - degms2rad(15, 54, 12),
*/
			.az = degms2rad(0, 29, 0),
			.z = PIo2 - degms2rad(47, 36, 0),
			.roll = 0
		}
	},
	
	.jd_start = 2458994.5, /* 2020-05-25 UTC */
	.jd_end = 2458994.8,
	.jd_step = 0.00001,
	.DUT1 = -0.22305 / SECoJD,
	.DT = 68 / SECoJD	
};

int
main(int argc, char *argv[])
{
	char *fnR = (argc > 1) ? argv[1] : "/dev/stdout";
	char *fnG = (argc > 2) ? argv[2] : "/dev/stdout";
	char *fnB = (argc > 3) ? argv[3] : "/dev/stdout";

	Image *im;
	
	double t;
	HorizontalLocation hz;
	RealPoint pt;
	uint8_t *px;

	int i, j;

	EquatorialLocation cameq0 = hz2eq((HorizontalLocation){ .az = ctxt.c.angle.az, .z = ctxt.c.angle.z }, ctxt.o.geod, ctxt.jd_start, ctxt.DUT1, ctxt.DT);


	Rect r = {
		/* these are not the actual camera image dimensions ! */
		.min = { .x = -ctxt.c.sdim.x/2 - ctxt.c.sdim.x%2, .y = -ctxt.c.sdim.y/2 - ctxt.c.sdim.y%2 },
		.max = { .x = ctxt.c.sdim.x/2, .y = ctxt.c.sdim.y/2 }
	};

	wtlog(0, "allocating image...\n");
	if ((im = newim(Irgb_uint8)) == nil) {
		wterror("cannot allocate memory\n");
		return -1;
	}

	wtlog(0, "allocating channel...\n");	
	if (initchan(im->chans[0], r)) {
		wterror("cannot allocate memory\n");
		freeim(im);
		return -1;
	}
	if (initchan(im->chans[1], r)) {
		wterror("cannot allocate memory\n");
		freeim(im);
		return -1;
	}
	if (initchan(im->chans[2], r)) {
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
/*
	if (wtim(im, fn)) {
		wterror("cannot write to file\n");
		freeim(im);
		return -1;
	}
*/
	if (wtpng(im, 0, fnR)) {
		wterror("cannot write to file\n");
		freeim(im);
		return -1;
	}
	if (wtpng(im, 1, fnG)) {
		wterror("cannot write to file\n");
		freeim(im);
		return -1;
	}
	if (wtpng(im, 2, fnB)) {
		wterror("cannot write to file\n");
		freeim(im);
		return -1;
	}

	freeim(im);
	return 0;		
}
