#include <math.h>

#include "c.h"

#include "algebra.h"
#include "telescope.h"

double gast(double jd, double dut1, double dt);
double eqeqx(double jd, double dut1, double dt);

double gammab_fw(double jd, double dut1, double dt);
double phib_fw(double jd, double dut1, double dt);
double psib_fw(double jd, double dut1, double dt);
double epsA_fw(double jd, double dut1, double dt);


HorizontalLocation
eq2hz(EquatorialLocation eq, GeodeticLocation geod, double jdutc, double dut1, double dt)
{
	double gst = gast(jdutc, dut1, dt);
	double h = gst + geod.lng - eq.ra;
/*
	double tU = jd+dut1-JD00;
	double era = PIx2 * (tU -(long)tU + 0.7790572732640 + 0.00273781191135448 * tU);
	double h = era + geod.lng - eq.ra;
*/
	double sph = sin(geod.lat);
	double cph = cos(geod.lat);
	double sd = sin(eq.dec);
	double cd = cos(eq.dec);
	double sh = sin(h);
	double ch = cos(h);

	double u = sd*cph - cd*ch*sph;
	double v = -cd*sh;
	double w = sd*sph + cd*ch*cph;

	HorizontalLocation hz;

	hz.az = atan2(v, u);
	hz.az = hz.az < 0.0d ? hz.az+PIx2 : hz.az;

	hz.z = acos(w);
	//hz.z = PIo2 - atan2(w, sqrt(u*u+v*v));

	return hz;
}

EquatorialLocation
hz2eq(HorizontalLocation hz, GeodeticLocation geod, double jdutc, double dut1, double dt)
{
	double sph = sin(geod.lat);
	double cph = cos(geod.lat);
	double saz = sin(hz.az);
	double caz = cos(hz.az);
	double sz = sin(hz.z);
	double cz = cos(hz.z);

	double u = cz*cph - caz*sz*sph;
	double v = -saz*sz;
	double w = cz*sph + caz*sz*cph;

	double h = atan2(v, u);
	double gst = gast(jdutc, dut1, dt);
	
	EquatorialLocation eq;
	eq.ra = gst + geod.lng - h;
	//eq.dec = asin(w);
	eq.dec = atan2(w, sqrt(u*u+v*v));

	return eq;
}

RealPoint
hz2pixel(Camera *c, HorizontalLocation hz)
{
	double dz = hz.z - (c->angle).z;
	double daz = hz.az - (c->angle).az;

	double tz = tan(dz);
	double taz = tan(daz);

	double sr = sin((c->angle).roll);
	double cr = cos((c->angle).roll);

	RealPoint pt = { .x = -(c->f_mm)*tz, .y = (c->f_mm)*taz };
	RealPoint ptr = { .x = pt.x*cr-pt.y*sr, .y = pt.x*sr+pt.y*cr };

	while (dz < -PI) dz += PIx2;
	while (dz > PI) dz -= PIx2;
	while (daz < -PI) daz += PIx2;
	while (daz > PI) daz -= PIx2;

	// behind the camera
	if (dz < -PIo2 || dz > PIo2 || daz < -PIo2 || daz > PIo2)
		return (RealPoint){ .x = NAN, .y = NAN };

	return mm2pixel(c, ptr);	
}

HorizontalLocation
pixel2hz(Camera *c, RealPoint px)
{
	HorizontalLocation hz;

	double sr = sin((c->angle).roll);
	double cr = cos((c->angle).roll);

	RealPoint ptr = pixel2mm(c, px);
	RealPoint pt = { .x = ptr.x*cr+ptr.y*sr, .y = -ptr.x*sr+ptr.y*cr };

	double tz = -pt.x / (c->f_mm);
	double taz = -pt.y / (c->f_mm);

	hz.z = (c->angle).z + atan(tz);
	hz.az = (c->angle).az + atan(taz);

	return hz;
}

double
gast(double jd, double dut1, double dt)
{
	double tU = jd+dut1-JD00;
	double t = (tU+dt)/JDxC;

	/* earth rotation angle is used for the CIO-based hour angle */
	double era = PIx2 * (tU -(long)tU + 0.7790572732640 + 0.00273781191135448 * tU);
	
	/* from Fukushima 2003 */
	double gmst = era +
		(0.012911569 + t*4612.160517397) * DSEC +
		t*t*(1.391542507 + t*(-0.000124849 + t*(-0.000004991 + t*(-0.000000479)))) * DSEC;

#if 0
	/* GMST approx from Capitaine et al. 2005 */
	double gmst = era + DSEC * (((((
		-0.0000000368)*t +
		-0.000029956)*t +
		-0.00000044)*t +
		1.3915817)*t +
		4612.156534)*t +
		0.014506;
#endif

	return gmst+eqeqx(jd, dut1, dt);
}

double
eqeqx(double jd, double dut1, double dt)
{
	double tU = jd+dut1-JD00;
	double t = (tU+dt)/JDxC;

	/* longitude of the scending node of moon's orbit */
	double om = (125.04452 + t*(-134.136261 + t*(0.0020708 + t/450000.0))) * DEG;

	/* mean longitude of the moon */
	double lm = (218.31654591 + t*(481267.88134236 + t*(-0.00163 + t*(1./538841.0 - t/65194000.0)))) * DEG;

	/* mean longitude of the sun */
	double ls = (280.46645 + t*(36000.76983 + t*0.0003032)) * DEG;

	/* mean anomaly of the sun */
	double gs = (357.52910 + t*(35999.05030 + t*( -0.0001559 + t*(-0.00000048)))) * DEG;

	/* geometric longitude of the sun */
	double gls = ls + ( (1.9146000 + t*(-0.004817 + t*(- 0.000014)))*sin(gs) + (0.019993 - 0.000101*t)*sin(2.0*gs) + 0.000290*sin(3.0*gs) ) * DEG;
	
	double dpsi =  ( -17.20*sin(om) - 1.32*sin(gls+gls) - 0.23*sin(lm+lm) + 0.21*sin(om+om) ) * DSEC;
	double epsbar = (84381.448 + t*(-46.8150 + t*(-0.00059 + t*0.001813))) * DSEC;

	return dpsi*cos(epsbar) + (0.00264*sin(om) + 0.000063*sin(om+om)) * DSEC;	
}

double gammab_fw(double jd, double dut1, double dt)
{
	double t = (jd+dut1+dt-JD00) / JDxC;

	/* from Wallace & Capitaine, 2006 (Fukushima-Williams equations) */
	return ((((((
		0.0000000260)*t - 
		0.000002788)*t -
		0.00031238)*t +
		0.4932044)*t +
		10.556378)*t -
		0.052928) * DSEC;
}

double phib_fw(double jd, double dut1, double dt)
{
	double t = (jd+dut1+dt-JD00) / JDxC;

	/* from Wallace & Capitaine, 2006 (Fukushima-Williams equations) */
	return ((((((
		-0.0000000176)*t -
		0.000000440)*t +
		0.00053289)*t +
		0.0511268)*t -
		46.811016)*t +
		84381.412819) * DSEC;
}

double psib_fw(double jd, double dut1, double dt)
{
	double t = (jd+dut1+dt-JD00) / JDxC;

	/* from Wallace & Capitaine, 2006 (Fukushima-Williams equations) */
	return ((((((
		-0.0000000148)*t +
		0.000026452)*t -
		0.00018522)*t -
		1.5584175)*t +
		5038.481484)*t -
		0.041775) * DSEC;
}

double epsA_fw(double jd, double dut1, double dt)
{
	double t = (jd+dut1+dt-JD00) / JDxC;

	/* from Wallace & Capitaine, 2006 (Fukushima-Williams equations) */
	return ((((((
		-0.0000000434)*t -
		0.000000576)*t +
		0.00200340)*t - 
		0.0001831)*t -
		46.836769)*t +
		84381.406) * DSEC;
}

void
sXY(double jd, double dut1, double dt, double *s, double *X, double *Y)
{
	double tau = (jd+dut1+dt-JD00);
	double t = tau / JDxC;

	double sXY2 = (((((15.62*t + 27.98)*t - 72574.11)*t - 122.68)*t + 3808.65)*t + 94.0);

	double Omega = 2.182 - 9.242e-4*t;

	*X = 2.6603e-7*t - 33.2e-6*sin(Omega);
	*Y = -8.14e-14*t*t+44.6e-6*cos(Omega);
	
	*s = sXY2 - (*X)*(*Y)/2;

	return;	
}
