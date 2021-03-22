/*------------------------------------------------------------------------
  nwpl_map_earth_d.c
------------------------------------------------------------------------*/

# include <stdio.h>
# include <math.h>

# include "nwpl_map_earth.h"

/* private variable */

  static double earth_rad_m = NWP_EARTH_DEFAULT_RAD_M;
  static double earth_rad_e = NWP_EARTH_BESSEL_RAD_E;
  static double earth_rad_p = NWP_EARTH_BESSEL_RAD_P;
  static double earth_ecc   = NWP_EARTH_BESSEL_ECC;


double nwp_get_earth_rad_m( void ) { return earth_rad_m; }

double nwp_get_earth_rad_e( void ) { return earth_rad_e; }

double nwp_get_earth_rad_p( void ) { return earth_rad_p; }

double nwp_get_earth_ecc  ( void ) { return earth_ecc; }

void   nwp_set_earth_rad_m( double rr ) { 
    earth_rad_m = rr; 
}

void   nwp_set_earth_rad_e( double rr ) { 
    earth_rad_e = rr;
    earth_ecc = sqrt( earth_rad_e * earth_rad_e 
                    - earth_rad_p * earth_rad_p ) / earth_rad_e;
}

void   nwp_set_earth_rad_p( double rr ) { 
    earth_rad_p = rr;
    earth_ecc = sqrt( earth_rad_e * earth_rad_e 
                    - earth_rad_p * earth_rad_p ) / earth_rad_e;
}


int nwp_set_earth( int id )
{
    switch( id ) {
    case NWP_EARTH_BESSEL :
	    earth_rad_m   = NWP_EARTH_DEFAULT_RAD_M;
	    earth_rad_p   = NWP_EARTH_BESSEL_RAD_P;
	    earth_rad_e   = NWP_EARTH_BESSEL_RAD_E;
	    earth_ecc     = NWP_EARTH_BESSEL_ECC;
	    break;

    case NWP_EARTH_GRS80  :
	    earth_rad_m   = NWP_EARTH_GRS80_RAD_M;
	    earth_rad_p   = NWP_EARTH_GRS80_RAD_P;
	    earth_rad_e   = NWP_EARTH_GRS80_RAD_E;
	    earth_ecc     = NWP_EARTH_GRS80_ECC  ;
	    break;

    default :
	fprintf( stderr, "ERROR! %s : nwp_set_earth() : Unknown Earth! \n",
	    __FILE__ );
	    earth_rad_m   = 
	    earth_rad_p   = 
	    earth_rad_e   = 
	    earth_ecc     = 1.0;
	return -1;
    }
    return 0;
}

