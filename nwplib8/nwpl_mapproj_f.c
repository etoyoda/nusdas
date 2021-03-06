#include <math.h>
#include "nwpl_mapproj_df.h"
#include "nwpl_map_earth.h"

# ifndef M_PI
# define M_PI 3.14159265358979323846
# endif
static const double pid2 = M_PI / 2.0;
static const double pid4 = M_PI / 4.0;
static const double deg2rad = M_PI / 180.0;
static const double rad2deg = 180.0 / M_PI;

# define ITER 3

int NWP_ellipse2sphere_F(const float *elat, const float *elon, 
    const int size, const float slat, const float slon, 
    float *lat, float *lon)
{
    int i;
    double e_ecc = nwp_get_earth_ecc();
    double ecc2  = e_ecc * e_ecc;
    double c = sqrt(1.0 + ecc2 * pow(cos(slat * deg2rad), 4.0) / (1.0 - ecc2));
    /*poption parallel */
    for (i = 0; i < size; i++) {
	double esinelat = e_ecc * sin(elat[i] * deg2rad);
	lat[i] = (atan(
	    pow(tan(elat[i] * deg2rad / 2.0 + pid4), c)
	    * pow((1.0 - esinelat) / (1.0 + esinelat), e_ecc * c / 2.0)
	    ) * 2.0 - pid2) * rad2deg;
	lon[i] = (elon[i] - slon) * c + slon;
    }
    return 0;
}

int NWP_sphere2ellipse_F(const float *lat, const float *lon, 
    const int size, const float slat, const float slon, 
    float *elat, float *elon)
{
    double e_ecc = nwp_get_earth_ecc();
    double ecc2  = e_ecc * e_ecc;
    double cinv = 1.0 / sqrt(
	1.0 + ecc2 * pow(cos(slat * deg2rad), 4.0 )
	/ (1.0 - ecc2));
    int i;
    /*poption parallel */
    for (i = 0; i < size; i++) {
	int j;
        /*poption noparallel */
	for (elat[i] = lat[i], j = 0; j < ITER; j++) {
	    double esinelat = e_ecc * sin(elat[i] * deg2rad);
	    elat[i] = (atan(
		pow(tan(lat[i] * deg2rad / 2.0 + pid4), cinv)
		* pow((1.0 - esinelat) / (1.0 + esinelat), -e_ecc / 2.0)
		) * 2.0 - pid2) * rad2deg;
	}
	elon[i] = (lon[i] - slon) * cinv + slon;
    }
    return 0;
}

int NWP_sphere2oblique_F(const float *lat, const float *lon, 
    const int size, const float slat, const float slon, 
    float *olat, float *olon)
{
    int i;
    double sinslat = sin(slat * deg2rad);
    double cosslat = cos(slat * deg2rad);
    /*poption parallel */
    for (i = 0; i < size; i++) {
	double sinlat = sin(lat[i] * deg2rad);
	double coslat = cos(lat[i] * deg2rad);
	double sindlon = sin((lon[i] - slon) * deg2rad);
	double cosdlon = cos((lon[i] - slon) * deg2rad);
	olat[i] = asin(
	    sinslat * sinlat + cosslat * coslat * cosdlon
	    ) * rad2deg;
	olon[i] = atan2(coslat * sindlon , (
	    cosslat * sinlat - sinslat * coslat * cosdlon
	    )) * rad2deg;
    }
    return 0;
}

int NWP_oblique2sphere_F(const float *olat, const float *olon, 
    const int size, const float slat, const float slon, 
    float *lat, float *lon)
{
    int i;
    double sinslat = sin(slat * deg2rad);
    double cosslat = cos(slat * deg2rad);
    /*poption parallel */
    for (i = 0; i < size; i++) {
	double sinolat = sin(olat[i] * deg2rad);
	double cosolat = cos(olat[i] * deg2rad);
	double sinolon = sin(olon[i] * deg2rad);
	double cosolon = cos(olon[i] * deg2rad);
	lat[i] = asin(
	    sinslat * sinolat + cosslat * cosolat * cosolon
	    ) * rad2deg;
	lon[i] = atan2(sinolon * cosolat , (
	    cosslat * sinolat - sinslat * cosolat * cosolon
	    )) * rad2deg + slon;
    }
    return 0;
}

int NWP_sphere2lambert_F(const float *lat, const float *lon, 
    const int size, const float slat1, const float slat2, 
    const float slon, const float rlat, const float rlon, 
    const float rx, const float ry, const float d,
    float *x, float *y)
{
    int i;
    double e_re = nwp_get_earth_rad_m();
    double dinv = 1.0 / d;
    double sign = ( slat2 >= 0.0 ) ? 1.0 : -1.0;
    double m1 = cos(slat1 * deg2rad);
    double t1 = tan(pid4 - sign * slat1 * deg2rad / 2.0);
    double n, af, r0, x0, y0;
    if( slat1 == slat2 ) {
        n  = cos( pid2 - sign * slat1 );
        af = e_re * m1 / n / pow(t1, n);
    } else {
        double m2 = cos(slat2 * deg2rad);
        double t2 = tan(pid4 - sign * slat2 * deg2rad / 2.0);
        n  = log(m1 / m2) / log(t1 / t2);
        af = e_re * m1 / n / pow(t1, n);
    }
    r0 = af * pow(tan(pid4 - sign * rlat * deg2rad / 2.0), n);
    x0 = rx -        r0 * sin((rlon - slon) * deg2rad * n) * dinv;
    y0 = ry - sign * r0 * cos((rlon - slon) * deg2rad * n) * dinv;
    /*poption parallel */
    for (i = 0; i < size; i++) {
        double dlon = lon[i] - slon;
	double tanx = tan(fabs(90.0 - sign * lat[i]) * deg2rad * 0.5);
        double r;
        r = af * pow(tanx, n);
	if (dlon > 180.0){
	    dlon -= 360.0;
	} else if (dlon < -180.0){
	    dlon += 360.0;
	}
        x[i] = x0 +        r * sin(dlon * deg2rad * n) * dinv;
        y[i] = y0 + sign * r * cos(dlon * deg2rad * n) * dinv;
    }
    return 0;
}

int NWP_lambert2sphere_F(const float *x, const float *y,
    const int size, const float slat1, const float slat2, 
    const float slon, const float rlat, const float rlon, 
    const float rx, const float ry, const float d,
    float *lat, float *lon)
{
    int i;
    double e_re = nwp_get_earth_rad_m();
    double sign = ( slat2 >= 0.0 ) ? 1.0 : -1.0;
    double m1 = cos(slat1 * deg2rad);
    double t1 = tan(pid4 - sign * slat1 * deg2rad / 2.0);
    double n, af, r0, x0, y0;
    if( slat1 == slat2 ) {
        n  = cos( pid2 - sign * slat1 );
        af = e_re * m1 / n / pow(t1, n);
    } else {
        double m2 = cos(slat2 * deg2rad);
        double t2 = tan(pid4 - sign * slat2 * deg2rad / 2.0);
        n  = log(m1 / m2) / log(t1 / t2);
        af = e_re * m1 / n / pow(t1, n);
    }
    r0 = af * pow(tan(pid4 - sign * rlat * deg2rad / 2.0), n);
    x0 = rx -        r0 * sin((rlon - slon) * deg2rad * n) / d;
    y0 = ry - sign * r0 * cos((rlon - slon) * deg2rad * n) / d;
    /*poption parallel */
    for (i = 0; i < size; i++) {
        double dx = x[i] - x0;
        double dy = sign * ( y[i] - y0 );
        lat[i] = sign * (pid2 - atan(
            pow(sqrt(dx * dx + dy * dy) * d / af, 1.0 / n)
            ) * 2.0) * rad2deg;
        lon[i] = (atan2(dx, dy) / n) * rad2deg + slon;
        if (lon[i] > 180.0) {
            lon[i] -= 360.0;
        }
    }
    return 0;
}

int NWP_sphere2mercator_F(const float *lat, const float *lon, 
    const int size, const float slat, const float rlat, 
    const float rlon, const float rx, const float ry, 
    const float d, float *x, float *y)
{
    int i;
    double e_re = nwp_get_earth_rad_m();
    double dinv = 1.0 / d;
    double ak = e_re * cos(slat * deg2rad);
    double x0 = rx - ak * rlon * deg2rad * dinv;
    double y0 = ry + ak * log(tan(pid4 + rlat * deg2rad / 2.0)) * dinv;
    /*poption parallel */
    for (i = 0; i < size; i++) {
	x[i] = x0 + ak * (lon[i] < 0.0 ? 360.0 + lon[i] : lon[i]) * deg2rad
	    * dinv;
	y[i] = y0 - ak * log(tan(pid4 + lat[i] * deg2rad / 2.0)) * dinv;
    }
    return 0;
}

int NWP_sphere2mercator2_F(const float *lat, const float *lon, 
    const int size, const float slat, const float rlat, 
    const float rlon, const float rx, const float ry, 
    const float d, float *x, float *y)
{
    int i;
    double e_re = nwp_get_earth_rad_m();
    double dinv = 1.0 / d;
    double ak = e_re * cos(slat * deg2rad);
    double x0 = rx - ak * rlon * deg2rad * dinv;
    double y0 = ry + ak * log(tan(pid4 + rlat * deg2rad / 2.0)) * dinv;
    /*poption parallel */
    for (i = 0; i < size; i++) {
	x[i] = x0 + ak * lon[i]  * deg2rad * dinv;
	y[i] = y0 - ak * log(tan(pid4 + lat[i] * deg2rad / 2.0)) * dinv;
    }
    return 0;
}

int NWP_mercator2sphere_F(const float *x, const float *y, 
    const int size, const float slat, const float rlat, 
    const float rlon, const float rx, const float ry, 
    const float d, float *lat, float *lon)
{
    int i;
    double e_re = nwp_get_earth_rad_m();
    double ak = e_re * cos(slat * deg2rad);
    double x0 = rx - ak * rlon * deg2rad / d;
    double y0 = ry + ak * log(tan(pid4 + rlat * deg2rad / 2.0)) / d;
    /*poption parallel */
    for (i = 0; i < size; i++) {
	lat[i] = (atan(exp((y0 - y[i]) * d / ak)) * 2.0 - pid2) * rad2deg;
	lon[i] = ((x[i] - x0) * d / ak) * rad2deg;
	if (lon[i] > 180.0) {
	    lon[i] -= 360.0;
	}
    }
    return 0;
}

int NWP_sphere2polar_F(const float *lat, const float *lon,
    const int size, const float slat, const float slon,
    const float rlat, const float rlon,
    const float rx, const float ry, const float d,
    float *x, float *y)
{
    int i;
    double e_re = nwp_get_earth_rad_m();
    double dinv = 1.0 / d;
    double sign = ( slat >= 0.0 ) ? 1.0 : -1.0;
    double ak = e_re * (1.0 + sin(sign * slat * deg2rad));
    double r0 = ak * tan(pid4 - sign * rlat * deg2rad / 2.0);
    double x0 = rx -        r0 * sin((rlon - slon) * deg2rad) * dinv;
    double y0 = ry - sign * r0 * cos((rlon - slon) * deg2rad) * dinv;
    /*poption parallel */
    for (i = 0; i < size; i++) {
        double lon360 = lon[i] < 0.0 ? 360.0 + lon[i] : lon[i];
        double r = ak * tan(fabs(90.0 - sign * lat[i]) * deg2rad * 0.5);
        x[i] = x0 +        r * sin((lon360 - slon) * deg2rad) * dinv;
        y[i] = y0 + sign * r * cos((lon360 - slon) * deg2rad) * dinv;
    }
    return 0;
}

int NWP_polar2sphere_F(const float *x, const float *y, 
    const int size, const float slat, const float slon,
    const float rlat, const float rlon, 
    const float rx, const float ry, const float d,
    float *lat, float *lon)
{
    int i;
    double e_re = nwp_get_earth_rad_m();
    double sign = ( slat >= 0.0 ) ? 1.0 : -1.0;
    double ak = e_re * (1.0 + sin(sign * slat * deg2rad));
    double r0 = ak * tan(pid4 - sign * rlat * deg2rad / 2.0);
    double x0 = rx -        r0 * sin((rlon - slon) * deg2rad) / d;
    double y0 = ry - sign * r0 * cos((rlon - slon) * deg2rad) / d;
    /*poption parallel */
    for (i = 0; i < size; i++) {
        double dx = x[i] - x0;
        double dy = sign * ( y[i] - y0 );
        lat[i] = sign * (pid2 - atan(
            sqrt(dx * dx + dy * dy) * d / ak
            ) * 2.0) * rad2deg;
        lon[i] = atan2(dx, dy) * rad2deg + slon;
        if (lon[i] > 180.0) {
            lon[i] -= 360.0;
        }
    }
    return 0;
}

int NWP_mf_lambert_F(const float *lat, const int size, 
    const float slat1, const float slat2, float *mf)
{
    int i;
    double m1 = cos(slat1 * deg2rad);
    double m2 = cos(slat2 * deg2rad);
    double t1 = tan(pid4 - slat1 * deg2rad / 2.0);
    double t2 = tan(pid4 - slat2 * deg2rad / 2.0);
    double n = log(m1 / m2) / log(t1 / t2);
    double p1 = m1 / pow(t1, n);
    /*poption parallel */
    for(i = 0; i < size; i++) {
        mf[i] = p1 * pow(tan(fabs(90.0 - fabs(lat[i])) * deg2rad * 0.5), n)
            / cos(lat[i] * deg2rad);
    }
    return 0;
}

int NWP_mf_mercator_F(const float *lat, const int size, 
    const float slat, float *mf)
{
    int i;
    double cosslat = cos(slat * deg2rad);
    /*poption parallel */
    for (i = 0; i < size; i++) {
        mf[i] = cosslat / cos(lat[i] * deg2rad);
    }
    return 0;
}

int NWP_mf_polar_F(const float *lat, const int size, 
    const float slat, float *mf)
{
    int i;
    double fabssinslat = fabs(sin(slat * deg2rad));
    /*poption parallel */
    for (i = 0; i < size; i++) {
        mf[i] = (1.0 + fabssinslat)
            / (1.0 + fabs(sin(lat[i] * deg2rad)));
    }
    return 0;
}

float NWP_sphere_distance_F(const float alat, const float alon, 
    const float blat, const float blon)
{
    double e_re = nwp_get_earth_rad_m();
    return acos(
        sin(alat * deg2rad) * sin(blat * deg2rad)
        + cos(alat * deg2rad) * cos(blat * deg2rad)
        * cos((blon - alon) * deg2rad)
        ) * e_re;
}

