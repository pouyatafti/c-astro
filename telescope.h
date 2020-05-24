/* requires: math.h, algebra.h */

#ifndef __CASTRO_TELESCOPE_H__
#define __CASTRO_TELESCOPE_H__

typedef struct GeodeticLocation GeodeticLocation;
struct GeodeticLocation {
	double lat;	/* in rad */
	double lng;	/* in rad */
	double alt_m;	/* in meters from mean sea level */
};

typedef struct EquatorialLocation EquatorialLocation;
struct EquatorialLocation {
	double ra;			/* right ascension in rad */
	double dec;		/* declination in rad */
};

typedef struct HorizontalLocation HorizontalLocation;
struct HorizontalLocation {
	double az;		/* azimuth in rad */
	double z;			/* zenith in rad (complement of elevation) */
};

typedef struct Observer Observer;
struct Observer {
	GeodeticLocation geod;
	RealPoint polarm;	/* polar motion coordinates */
	double P_kPa;		/* pressure in kPa */
	double T_K;		/* temperature in K */
	double rh;			/* relative humidity */
};

typedef struct  CameraAngle CameraAngle;
struct CameraAngle {
	double az;	/* azimuth in rad */
	double z;		/* zenith in rad (complement of elevation) */
	double roll;	/* roll in rad */
};

typedef struct Camera Camera;
struct Camera {
	double f_mm;			/* focal length in mm */
	RealPoint sdim_mm;		/* sensor dimensions in mm */
	Point sdim;			/* sensor dimensions in pixels */
//	RealPoint offset;		/* principal offset in pixels */
//	double skew;			/* axis skew */
//	double distcoeff[3];		/* radial distortion coefficients (k) */
	CameraAngle angle;
};

#define PI 3.14159265358979323846d
#define PIx2 6.28318530717958647692d
#define PIo2 1.57079632679489661923d
#define DEG 1.745329251994329576923e-2
#define DMIN 2.908882086657215961539e-4
#define DSEC 4.848136811095359935899e-6
#define HOUR 2.617993877991494365386e-1
#define MIN 4.363323129985823942309e-3
#define SEC 7.272205216643039903849e-5

#define JD00 2451545.0
#define JDxC 36525.0 /* JDs per century */
#define SECoJD (24*3600) /* seconds per JD */

#define deg2rad(d) ( (d) * DEG )
#define rad2deg(a) ( (a) / DEG )
#define h2rad(h) ( (h) * HOUR )
#define rad2h(a) ( (a) / HOUR )
#define hms2h(h, m, s) ( (h) + (m)/60.0d + (s)/3600.0d )
#define hms2rad(h, m, s) ( (h)*HOUR+(m)*MIN+(s)*SEC )
#define degms2rad(d, m, s) ( (d)*DEG+(m)*DMIN+(s)*DSEC )

/* atmospheric refraction */
#define NOMINAL_PRESSURE_kPa 101.0d
#define NOMINAL_TEMPERATURE_K 286.0d

/* add this to the vacuum zenith for atmospheric refraction (o is the observer) */
#define saemundsson(o, z) ( -h2rad(1.02d/60.0d) * (o.P_kPa/101.0d) * (283.0d/o.T_K) / tan( PI/2.0d - (z) + deg2rad(10.3d)/(PI/2.0d - (z) + deg2rad(5.11d)) ) )

/* jdutc is the julian UTC day; no atmospheric refraction */
HorizontalLocation eq2hz(EquatorialLocation eq, GeodeticLocation geod, double jdutc, double dut1, double dt);
EquatorialLocation hz2eq(HorizontalLocation hz, GeodeticLocation geod, double jdutc, double dut1, double dt);

RealPoint hz2pixel(Camera *c, HorizontalLocation hz);
HorizontalLocation pixel2hz(Camera *c, RealPoint px);
double gast(double jd, double dut1, double dt);

#define mm2pixel(c, pt) ( (RealPoint){ .x = (pt).x * ((c)->sdim).x / ((c)->sdim_mm).x  , .y = (pt).y * ((c)->sdim).y / ((c)->sdim_mm).y } )
#define pixel2mm(c, px) ( (RealPoint){ .x = (px).x * ((c)->sdim_mm).x / ((c)->sdim).x  , .y = (px).y * ((c)->sdim_mm).y / ((c)->sdim).y } )

#endif