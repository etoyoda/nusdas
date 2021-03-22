dnl to avoid error caused by single quotation marks in m4 statement
changequote(@<:@, @:>@)dnl
dnl

#include "nwpl_mapproj_df.h"


dnl
define(@<:@ELLIPSE2SPHERE@:>@, dnl
@<:@dnl
void $1(const $2 *elat, const $2 *elon, 
    const int *size, const $2 *slat, const $2 *slon,
    $2 *lat, $2 *lon, int *iostat)
{
    *iostat = NWP_ellipse2sphere_$3(elat, elon, *size, *slat, *slon, lat, lon);
}
@:>@)dnl
ELLIPSE2SPHERE(NWPC8_ELLIPSE2SPHERE_F, float, F)
ELLIPSE2SPHERE(NWPC8_ELLIPSE2SPHERE_F_, float, F)
ELLIPSE2SPHERE(nwpc8_ellipse2sphere_f, float, F)
ELLIPSE2SPHERE(nwpc8_ellipse2sphere_f_, float, F)
ELLIPSE2SPHERE(nwpc8_ellipse2sphere_f__, float, F)
ELLIPSE2SPHERE(NWPC8_ELLIPSE2SPHERE_D, double, D)
ELLIPSE2SPHERE(NWPC8_ELLIPSE2SPHERE_D_, double, D)
ELLIPSE2SPHERE(nwpc8_ellipse2sphere_d, double, D)
ELLIPSE2SPHERE(nwpc8_ellipse2sphere_d_, double, D)
ELLIPSE2SPHERE(nwpc8_ellipse2sphere_d__, double, D)dnl



dnl
define(@<:@SPHERE2ELLIPSE@:>@, dnl
@<:@dnl
void $1(const $2 *lat, const $2 *lon,
    const int *size, const $2 *slat, const $2 *slon,
    $2 *elat, $2 *elon, int *iostat)
{
    *iostat = NWP_sphere2ellipse_$3(lat, lon, *size, *slat, *slon, elat, elon);
}
@:>@)dnl
SPHERE2ELLIPSE(NWPC8_SPHERE2ELLIPSE_F, float, F)
SPHERE2ELLIPSE(NWPC8_SPHERE2ELLIPSE_F_, float, F)
SPHERE2ELLIPSE(nwpc8_sphere2ellipse_f, float, F)
SPHERE2ELLIPSE(nwpc8_sphere2ellipse_f_, float, F)
SPHERE2ELLIPSE(nwpc8_sphere2ellipse_f__, float, F)
SPHERE2ELLIPSE(NWPC8_SPHERE2ELLIPSE_D, double, D)
SPHERE2ELLIPSE(NWPC8_SPHERE2ELLIPSE_D_, double, D)
SPHERE2ELLIPSE(nwpc8_sphere2ellipse_d, double, D)
SPHERE2ELLIPSE(nwpc8_sphere2ellipse_d_, double, D)
SPHERE2ELLIPSE(nwpc8_sphere2ellipse_d__, double, D)dnl



dnl
define(@<:@SPHERE2OBLIQUE@:>@, dnl
@<:@dnl
void $1(const $2 *lat, const $2 *lon,
    const int *size, const $2 *slat, const $2 *slon,
    $2 *olat, $2 *olon, int *iostat)
{
    *iostat = NWP_sphere2oblique_$3(lat, lon, *size, *slat, *slon, olat, olon);
}
@:>@)dnl
SPHERE2OBLIQUE(NWPC8_SPHERE2OBLIQUE_F, float, F)
SPHERE2OBLIQUE(NWPC8_SPHERE2OBLIQUE_F_, float, F)
SPHERE2OBLIQUE(nwpc8_sphere2oblique_f, float, F)
SPHERE2OBLIQUE(nwpc8_sphere2oblique_f_, float, F)
SPHERE2OBLIQUE(nwpc8_sphere2oblique_f__, float, F)
SPHERE2OBLIQUE(NWPC8_SPHERE2OBLIQUE_D, double, D)
SPHERE2OBLIQUE(NWPC8_SPHERE2OBLIQUE_D_, double, D)
SPHERE2OBLIQUE(nwpc8_sphere2oblique_d, double, D)
SPHERE2OBLIQUE(nwpc8_sphere2oblique_d_, double, D)
SPHERE2OBLIQUE(nwpc8_sphere2oblique_d__, double, D)dnl



dnl
define(@<:@OBLIQUE2SPHERE@:>@, dnl
@<:@dnl
void $1(const $2 *olat, const $2 *olon,
    const int *size, const $2 *slat, const $2 *slon,
    $2 *lat, $2 *lon, int *iostat)
{
    *iostat = NWP_oblique2sphere_$3(olat, olon, *size, *slat, *slon, lat, lon);
}
@:>@)dnl
OBLIQUE2SPHERE(NWPC8_OBLIQUE2SPHERE_F, float, F)
OBLIQUE2SPHERE(NWPC8_OBLIQUE2SPHERE_F_, float, F)
OBLIQUE2SPHERE(nwpc8_oblique2sphere_f, float, F)
OBLIQUE2SPHERE(nwpc8_oblique2sphere_f_, float, F)
OBLIQUE2SPHERE(nwpc8_oblique2sphere_f__, float, F)
OBLIQUE2SPHERE(NWPC8_OBLIQUE2SPHERE_D, double, D)
OBLIQUE2SPHERE(NWPC8_OBLIQUE2SPHERE_D_, double, D)
OBLIQUE2SPHERE(nwpc8_oblique2sphere_d, double, D)
OBLIQUE2SPHERE(nwpc8_oblique2sphere_d_, double, D)
OBLIQUE2SPHERE(nwpc8_oblique2sphere_d__, double, D)dnl



dnl
define(@<:@SPHERE2LAMBERT@:>@, dnl
@<:@dnl
void $1(const $2 *lat, const $2 *lon, 
    const int *size, const $2 *slat1, const $2 *slat2, 
    const $2 *slon, const $2 *rlat, const $2 *rlon, 
    const $2 *rx, const $2 *ry, const $2 *d, 
    $2 *x, $2 *y, int *iostat)
{
    *iostat = NWP_sphere2lambert_$3(lat, lon, *size, *slat1, *slat2, *slon,
    *rlat, *rlon, *rx, *ry, *d, x, y);
}
@:>@)dnl
SPHERE2LAMBERT(NWPC8_SPHERE2LAMBERT_F, float, F)
SPHERE2LAMBERT(NWPC8_SPHERE2LAMBERT_F_, float, F)
SPHERE2LAMBERT(nwpc8_sphere2lambert_f, float, F)
SPHERE2LAMBERT(nwpc8_sphere2lambert_f_, float, F)
SPHERE2LAMBERT(nwpc8_sphere2lambert_f__, float, F)
SPHERE2LAMBERT(NWPC8_SPHERE2LAMBERT_D, double, D)
SPHERE2LAMBERT(NWPC8_SPHERE2LAMBERT_D_, double, D)
SPHERE2LAMBERT(nwpc8_sphere2lambert_d, double, D)
SPHERE2LAMBERT(nwpc8_sphere2lambert_d_, double, D)
SPHERE2LAMBERT(nwpc8_sphere2lambert_d__, double, D)dnl



dnl
define(@<:@LAMBERT2SPHERE@:>@, dnl
@<:@dnl
void $1(const $2 *x, const $2 *y, 
    const int *size, const $2 *slat1, const $2 *slat2,
    const $2 *slon, const $2 *rlat, const $2 *rlon,
    const $2 *rx, const $2 *ry, const $2 *d, 
    $2 *lat, $2 *lon, int *iostat)
{
    *iostat = NWP_lambert2sphere_$3(x, y, *size, *slat1, *slat2, *slon,
    *rlat, *rlon, *rx, *ry, *d, lat, lon);
}
@:>@)dnl
LAMBERT2SPHERE(NWPC8_LAMBERT2SPHERE_F, float, F)
LAMBERT2SPHERE(NWPC8_LAMBERT2SPHERE_F_, float, F)
LAMBERT2SPHERE(nwpc8_lambert2sphere_f, float, F)
LAMBERT2SPHERE(nwpc8_lambert2sphere_f_, float, F)
LAMBERT2SPHERE(nwpc8_lambert2sphere_f__, float, F)
LAMBERT2SPHERE(NWPC8_LAMBERT2SPHERE_D, double, D)
LAMBERT2SPHERE(NWPC8_LAMBERT2SPHERE_D_, double, D)
LAMBERT2SPHERE(nwpc8_lambert2sphere_d, double, D)
LAMBERT2SPHERE(nwpc8_lambert2sphere_d_, double, D)
LAMBERT2SPHERE(nwpc8_lambert2sphere_d__, double, D)dnl



dnl
define(@<:@SPHERE2MERCATOR@:>@, dnl
@<:@dnl
void $1(const $2 *lat, const $2 *lon,
    const int *size, const $2 *slat, const $2 *rlat, 
    const $2 *rlon, const $2 *rx, const $2 *ry, 
    const $2 *d, $2 *x, $2 *y, int *iostat)
{
    *iostat = NWP_sphere2mercator_$3(lat, lon, *size, *slat, *rlat, *rlon,
    *rx, *ry, *d, x, y);
}
@:>@)dnl
SPHERE2MERCATOR(NWPC8_SPHERE2MERCATOR_F, float, F)
SPHERE2MERCATOR(NWPC8_SPHERE2MERCATOR_F_, float, F)
SPHERE2MERCATOR(nwpc8_sphere2mercator_f, float, F)
SPHERE2MERCATOR(nwpc8_sphere2mercator_f_, float, F)
SPHERE2MERCATOR(nwpc8_sphere2mercator_f__, float, F)
SPHERE2MERCATOR(NWPC8_SPHERE2MERCATOR_D, double, D)
SPHERE2MERCATOR(NWPC8_SPHERE2MERCATOR_D_, double, D)
SPHERE2MERCATOR(nwpc8_sphere2mercator_d, double, D)
SPHERE2MERCATOR(nwpc8_sphere2mercator_d_, double, D)
SPHERE2MERCATOR(nwpc8_sphere2mercator_d__, double, D)dnl
dnl

define(@<:@SPHERE2MERCATOR2@:>@, dnl
@<:@dnl
void $1(const $2 *lat, const $2 *lon,
    const int *size, const $2 *slat, const $2 *rlat, 
    const $2 *rlon, const $2 *rx, const $2 *ry, 
    const $2 *d, $2 *x, $2 *y, int *iostat)
{
    *iostat = NWP_sphere2mercator2_$3(lat, lon, *size, *slat, *rlat, *rlon,
    *rx, *ry, *d, x, y);
}
@:>@)dnl
SPHERE2MERCATOR2(NWPC8_SPHERE2MERCATOR2_F, float, F)
SPHERE2MERCATOR2(NWPC8_SPHERE2MERCATOR2_F_, float, F)
SPHERE2MERCATOR2(nwpc8_sphere2mercator2_f, float, F)
SPHERE2MERCATOR2(nwpc8_sphere2mercator2_f_, float, F)
SPHERE2MERCATOR2(nwpc8_sphere2mercator2_f__, float, F)
SPHERE2MERCATOR2(NWPC8_SPHERE2MERCATOR2_D, double, D)
SPHERE2MERCATOR2(NWPC8_SPHERE2MERCATOR2_D_, double, D)
SPHERE2MERCATOR2(nwpc8_sphere2mercator2_d, double, D)
SPHERE2MERCATOR2(nwpc8_sphere2mercator2_d_, double, D)
SPHERE2MERCATOR2(nwpc8_sphere2mercator2_d__, double, D)dnl

dnl
define(@<:@MERCATOR2SPHERE@:>@, dnl
@<:@dnl
void $1(const $2 *x, const $2 *y, 
    const int *size, const $2 *slat, const $2 *rlat, 
    const $2 *rlon, const $2 *rx, const $2 *ry, 
    const $2 *d, $2 *lat, $2 *lon, int *iostat)
{
    *iostat = NWP_mercator2sphere_$3(x, y, *size, *slat, *rlat, *rlon,
    *rx, *ry, *d, lat, lon);
}
@:>@)dnl
MERCATOR2SPHERE(NWPC8_MERCATOR2SPHERE_F, float, F)
MERCATOR2SPHERE(NWPC8_MERCATOR2SPHERE_F_, float, F)
MERCATOR2SPHERE(nwpc8_mercator2sphere_f, float, F)
MERCATOR2SPHERE(nwpc8_mercator2sphere_f_, float, F)
MERCATOR2SPHERE(nwpc8_mercator2sphere_f__, float, F)
MERCATOR2SPHERE(NWPC8_MERCATOR2SPHERE_D, double, D)
MERCATOR2SPHERE(NWPC8_MERCATOR2SPHERE_D_, double, D)
MERCATOR2SPHERE(nwpc8_mercator2sphere_d, double, D)
MERCATOR2SPHERE(nwpc8_mercator2sphere_d_, double, D)
MERCATOR2SPHERE(nwpc8_mercator2sphere_d__, double, D)dnl



dnl
define(@<:@SPHERE2POLAR@:>@, dnl
@<:@dnl
void $1(const $2 *lat, const $2 *lon, 
    const int *size, const $2 *slat, const $2 *slon, 
    const $2 *rlat, const $2 *rlon,
    const $2 *rx, const $2 *ry, const $2 *d,
    $2 *x, $2 *y, int *iostat)
{
    *iostat = NWP_sphere2polar_$3(lat, lon, *size, *slat, *slon,
    *rlat, *rlon, *rx, *ry, *d, x, y);
}
@:>@)dnl
SPHERE2POLAR(NWPC8_SPHERE2POLAR_F, float, F)
SPHERE2POLAR(NWPC8_SPHERE2POLAR_F_, float, F)
SPHERE2POLAR(nwpc8_sphere2polar_f, float, F)
SPHERE2POLAR(nwpc8_sphere2polar_f_, float, F)
SPHERE2POLAR(nwpc8_sphere2polar_f__, float, F)
SPHERE2POLAR(NWPC8_SPHERE2POLAR_D, double, D)
SPHERE2POLAR(NWPC8_SPHERE2POLAR_D_, double, D)
SPHERE2POLAR(nwpc8_sphere2polar_d, double, D)
SPHERE2POLAR(nwpc8_sphere2polar_d_, double, D)
SPHERE2POLAR(nwpc8_sphere2polar_d__, double, D)dnl



dnl
define(@<:@POLAR2SPHERE@:>@, dnl
@<:@dnl
void $1(const $2 *x, const $2 *y, 
    const int *size, const $2 *slat, const $2 *slon, 
    const $2 *rlat, const $2 *rlon, 
    const $2 *rx, const $2 *ry, const $2 *d, 
    $2 *lat, $2 *lon, int *iostat)
{
    *iostat = NWP_polar2sphere_$3(x, y, *size, *slat, *slon, *rlat, *rlon,
    *rx, *ry, *d, lat, lon);
}
@:>@)dnl
POLAR2SPHERE(NWPC8_POLAR2SPHERE_F, float, F)
POLAR2SPHERE(NWPC8_POLAR2SPHERE_F_, float, F)
POLAR2SPHERE(nwpc8_polar2sphere_f, float, F)
POLAR2SPHERE(nwpc8_polar2sphere_f_, float, F)
POLAR2SPHERE(nwpc8_polar2sphere_f__, float, F)
POLAR2SPHERE(NWPC8_POLAR2SPHERE_D, double, D)
POLAR2SPHERE(NWPC8_POLAR2SPHERE_D_, double, D)
POLAR2SPHERE(nwpc8_polar2sphere_d, double, D)
POLAR2SPHERE(nwpc8_polar2sphere_d_, double, D)
POLAR2SPHERE(nwpc8_polar2sphere_d__, double, D)dnl



dnl
define(@<:@MF_LAMBERT@:>@, dnl
@<:@dnl
void $1(const $2 *lat, const int *size,
    const $2 *slat1, const $2 *slat2, 
    $2 *mf, int *iostat)
{
    *iostat = NWP_mf_lambert_$3(lat, *size, *slat1, *slat2, mf);
}
@:>@)dnl
MF_LAMBERT(NWPC8_MF_LAMBERT_F, float, F)
MF_LAMBERT(NWPC8_MF_LAMBERT_F_, float, F)
MF_LAMBERT(nwpc8_mf_lambert_f, float, F)
MF_LAMBERT(nwpc8_mf_lambert_f_, float, F)
MF_LAMBERT(nwpc8_mf_lambert_f__, float, F)
MF_LAMBERT(NWPC8_MF_LAMBERT_D, double, D)
MF_LAMBERT(NWPC8_MF_LAMBERT_D_, double, D)
MF_LAMBERT(nwpc8_mf_lambert_d, double, D)
MF_LAMBERT(nwpc8_mf_lambert_d_, double, D)
MF_LAMBERT(nwpc8_mf_lambert_d__, double, D)dnl



dnl
define(@<:@MF_MERCATOR@:>@, dnl
@<:@dnl
void $1(const $2 *lat, const int *size, 
    const $2 *slat, $2 *mf, int *iostat)
{
    *iostat = NWP_mf_mercator_$3(lat, *size, *slat, mf);
}
@:>@)dnl
MF_MERCATOR(NWPC8_MF_MERCATOR_F, float, F)
MF_MERCATOR(NWPC8_MF_MERCATOR_F_, float, F)
MF_MERCATOR(nwpc8_mf_mercator_f, float, F)
MF_MERCATOR(nwpc8_mf_mercator_f_, float, F)
MF_MERCATOR(nwpc8_mf_mercator_f__, float, F)
MF_MERCATOR(NWPC8_MF_MERCATOR_D, double, D)
MF_MERCATOR(NWPC8_MF_MERCATOR_D_, double, D)
MF_MERCATOR(nwpc8_mf_mercator_d, double, D)
MF_MERCATOR(nwpc8_mf_mercator_d_, double, D)
MF_MERCATOR(nwpc8_mf_mercator_d__, double, D)dnl



dnl
define(@<:@MF_POLAR@:>@, dnl
@<:@dnl
void $1(const $2 *lat, const int *size, 
    const $2 *slat, $2 *mf, int *iostat)
{
    *iostat = NWP_mf_polar_$3(lat, *size, *slat, mf);
}
@:>@)dnl
MF_POLAR(NWPC8_MF_POLAR_F, float, F)
MF_POLAR(NWPC8_MF_POLAR_F_, float, F)
MF_POLAR(nwpc8_mf_polar_f, float, F)
MF_POLAR(nwpc8_mf_polar_f_, float, F)
MF_POLAR(nwpc8_mf_polar_f__, float, F)
MF_POLAR(NWPC8_MF_POLAR_D, double, D)
MF_POLAR(NWPC8_MF_POLAR_D_, double, D)
MF_POLAR(nwpc8_mf_polar_d, double, D)
MF_POLAR(nwpc8_mf_polar_d_, double, D)
MF_POLAR(nwpc8_mf_polar_d__, double, D)dnl



dnl
define(@<:@DISTANCE@:>@, dnl
@<:@dnl
void $1(const $2 *alat, const $2 *alon,
    const $2 *blat, const $2 *blon, $2 *dist)
{
    *dist = NWP_sphere_distance_$3(*alat, *alon, *blat, *blon);
}
@:>@)dnl
DISTANCE(NWPC8_DISTANCE_F, float, F)
DISTANCE(NWPC8_DISTANCE_F_, float, F)
DISTANCE(nwpc8_distance_f, float, F)
DISTANCE(nwpc8_distance_f_, float, F)
DISTANCE(nwpc8_distance_f__, float, F)
DISTANCE(NWPC8_DISTANCE_D, double, D)
DISTANCE(NWPC8_DISTANCE_D_, double, D)
DISTANCE(nwpc8_distance_d, double, D)
DISTANCE(nwpc8_distance_d_, double, D)
DISTANCE(nwpc8_distance_d__, double, D)dnl




