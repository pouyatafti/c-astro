/* requires: telescope.h */

Observer sample_observer = {
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
};

Camera canon_EOS600D = {
	/* Canon EOS 600D */
	.f_mm = 24,
	.sdim_mm = { .x = 22.3, .y = 14.9 },
	.sdim = { .x = 5344, .y = 3516 },
	.angle = {
		.az = 0,
		.z = 0,
		.roll = 0
	}
};

CameraAngle samples_angle[] = {
	{
		/* watching Polaris from Munich around 20200525UTC0000 */
		.az = degms2rad(0, 29, 0),
		.z = PIo2 - degms2rad(47, 36, 0),
		.roll = 0
	},
	{
		/* watching Andromeda from Munich around 20200525UTC0000 */
		.az = degms2rad(44, 31, 20),
		.z = PIo2 - degms2rad(15, 54, 12),
		.roll = 0
	}
};

EquatorialLocation sample_object[] = {
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
};

double sample_jd = 2458994.5; /* 2020-05-25 UTC */
double sample_DUT1 = -0.22305 / SECoJD;
double sample_DT = 68 / SECoJD;

