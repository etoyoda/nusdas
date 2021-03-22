dnl to avoid error caused by single quotation marks in m4 statement
changequote(@<:@, @:>@)dnl
dnl

# include "nwpl_map_earth.h"

/*
 * nwp_get_earth_rad_[mep]
 */
dnl
define(@<:@GETRADM@:>@, dnl
@<:@dnl
void $1( double *rr ) { *rr =  nwp_get_earth_rad_$2(); }
@:>@)dnl
GETRADM(NWPC_GET_EARTH_RAD_M  , m)
GETRADM(NWPC_GET_EARTH_RAD_M_ , m)
GETRADM(nwpc_get_earth_rad_m  , m)
GETRADM(nwpc_get_earth_rad_m_ , m)
GETRADM(nwpc_get_earth_rad_m__, m)
GETRADM(NWPC_GET_EARTH_RAD_E  , e)
GETRADM(NWPC_GET_EARTH_RAD_E_ , e)
GETRADM(nwpc_get_earth_rad_e  , e)
GETRADM(nwpc_get_earth_rad_e_ , e)
GETRADM(nwpc_get_earth_rad_e__, e)
GETRADM(NWPC_GET_EARTH_RAD_P  , p)
GETRADM(NWPC_GET_EARTH_RAD_P_ , p)
GETRADM(nwpc_get_earth_rad_p  , p)
GETRADM(nwpc_get_earth_rad_p_ , p)
GETRADM(nwpc_get_earth_rad_p__, p)dnl

/*
 * nwp_get_earth_ecc
 */
dnl
define(@<:@GETECC@:>@, dnl
@<:@dnl
void $1( double *rr ) { *rr =  nwp_get_earth_ecc(); }
@:>@)dnl
GETECC(NWPC_GET_EARTH_ECC  )
GETECC(NWPC_GET_EARTH_ECC_ )
GETECC(nwpc_get_earth_ecc  )
GETECC(nwpc_get_earth_ecc_ )
GETECC(nwpc_get_earth_ecc__)dnl

/*
 * nwp_set_earth_rad_[mep]
 */
dnl
define(@<:@SETRM@:>@, dnl
@<:@dnl
void $1( const double *rr ) { nwp_set_earth_rad_$2( *rr ); }
@:>@)dnl
SETRM(NWPC_SET_EARTH_RAD_M  , m)
SETRM(NWPC_SET_EARTH_RAD_M_ , m)
SETRM(nwpc_set_earth_rad_m  , m)
SETRM(nwpc_set_earth_rad_m_ , m)
SETRM(nwpc_set_earth_rad_m__, m)
SETRM(NWPC_SET_EARTH_RAD_E  , e)
SETRM(NWPC_SET_EARTH_RAD_E_ , e)
SETRM(nwpc_set_earth_rad_e  , e)
SETRM(nwpc_set_earth_rad_e_ , e)
SETRM(nwpc_set_earth_rad_e__, e)
SETRM(NWPC_SET_EARTH_RAD_P  , p)
SETRM(NWPC_SET_EARTH_RAD_P_ , p)
SETRM(nwpc_set_earth_rad_p  , p)
SETRM(nwpc_set_earth_rad_p_ , p)
SETRM(nwpc_set_earth_rad_p__, p)dnl
/*
 * nwp_set_earth()
 */
dnl
define(@<:@SETEARTH@:>@, dnl
@<:@dnl
void $1( const int *id ) { nwp_set_earth(*id); }
@:>@)dnl
SETEARTH(NWPC_SET_EARTH  )
SETEARTH(NWPC_SET_EARTH_ )
SETEARTH(nwpc_set_earth  )
SETEARTH(nwpc_set_earth_ )
SETEARTH(nwpc_set_earth__)dnl

