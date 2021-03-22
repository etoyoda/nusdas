/*
 * NWP LIB: Numerical Weather Prediction Library
 *
 * nwpl_capi.h: NWP LIB C Application Interface
 */

#ifndef NWPCAPI_H_INCLUDED
#define NWPCAPI_H_INCLUDED

#include "nwplib.h"

/* nwpl_jdsqcv.c */
#define nwp_ymd2seq     NWP_ymd2seq
#define nwp_ymdh2seq    NWP_ymdh2seq
#define nwp_ymdhm2seq   NWP_ymdhm2seq
#define nwp_seq2ymd     NWP_seq2ymd
#define nwp_seq2ymdh    NWP_seq2ymdh
#define nwp_seq2ymdhm   NWP_seq2ymdhm

/* nwpl_systime.c */
#define nwp_gettime     NWP_gettime
#define nwp_systime     NWP_systime

/* nwpl_util.c */
/* do not implement because NWP_* require fortran string length */
/*
#define nwp_filestat             NWP_filestat
#define nwp_lustre_recover       NWP_lustre_recover
*/

/* nwpl_mapproj_d.c */
#define nwp_ellipse2sphere_d     NWP_ellipse2sphere_D
#define nwp_sphere2ellipse_d     NWP_sphere2ellipse_D 
#define nwp_sphere2oblique_d     NWP_sphere2oblique_D
#define nwp_oblique2sphere_d     NWP_oblique2sphere_D
#define nwp_sphere2lambert_d     NWP_sphere2lambert_D
#define nwp_lambert2sphere_d     NWP_lambert2sphere_D
#define nwp_sphere2mercator_d    NWP_sphere2mercator_D
#define nwp_sphere2mercator2_d   NWP_sphere2mercator2_D
#define nwp_mercator2sphere_d    NWP_mercator2sphere_D
#define nwp_sphere2polar_d       NWP_sphere2polar_D
#define nwp_polar2sphere_d       NWP_polar2sphere_D
#define nwp_mf_lambert_d         NWP_mf_lambert_D
#define nwp_mf_mercator_d        NWP_mf_mercator_D
#define nwp_mf_polar_d           NWP_mf_polar_D
#define nwp_sphere_distance_d    NWP_sphere_distance_D

/* nwpl_mapproj_f.c */
#define nwp_ellipse2sphere_f     NWP_ellipse2sphere_F
#define nwp_sphere2ellipse_f     NWP_sphere2ellipse_F 
#define nwp_sphere2oblique_f     NWP_sphere2oblique_F
#define nwp_oblique2sphere_f     NWP_oblique2sphere_F
#define nwp_sphere2lambert_f     NWP_sphere2lambert_F
#define nwp_lambert2sphere_f     NWP_lambert2sphere_F
#define nwp_sphere2mercator_f    NWP_sphere2mercator_F
#define nwp_mercator2sphere_f    NWP_mercator2sphere_F
#define nwp_mercator2sphere2_f   NWP_mercator2sphere2_F
#define nwp_sphere2polar_f       NWP_sphere2polar_F
#define nwp_polar2sphere_f       NWP_polar2sphere_F
#define nwp_mf_lambert_f         NWP_mf_lambert_F
#define nwp_mf_mercator_f        NWP_mf_mercator_F
#define nwp_mf_polar_f           NWP_mf_polar_F
#define nwp_sphere_distance_f    NWP_sphere_distance_F


#endif /* NWPCAPI_H_INCLUDED */

/* nwpl_capi.h */
