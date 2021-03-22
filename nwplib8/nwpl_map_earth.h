/*------------------------------------------------------------------------
  nwpl_map_earth.h
------------------------------------------------------------------------*/

# ifndef NWPL_MAP_EARTH_H_INCLUDED
# define NWPL_MAP_EARTH_H_INCLUDED


# define NWP_EARTH_DEFAULT_RAD_M (6371.3e3)   
# define NWP_EARTH_BESSEL_RAD_E  (6377.4e3)   
# define NWP_EARTH_BESSEL_RAD_P  (6356.1e3)   
# define NWP_EARTH_BESSEL_ECC    (8.166147e-2) 

# define NWP_EARTH_GRS80_RAD_M (6371.0e3) 
# define NWP_EARTH_GRS80_RAD_E (6378.1370e3) 
# define NWP_EARTH_GRS80_RAD_P (6356.7523e3)  
# define NWP_EARTH_GRS80_ECC   (8.1819218e-2) 

/* for VSRF Oblique Lambert.   bounded on ellipse at 37N */
# define NWP_EARTH_GRS80_OBLM_RAD_M (6372.2022e3) 

enum {   NWP_EARTH_BESSEL =     1001,
         NWP_EARTH_GRS80  =     1002  };

/* for FORTRAN -> C linkage */

# define  nwp_set_earth           NWP_set_earth

# define  nwp_get_earth_rad_m     NWP_get_earth_rad_m
# define  nwp_get_earth_rad_e     NWP_get_earth_rad_e
# define  nwp_get_earth_rad_p     NWP_get_earth_rad_p
# define  nwp_get_earth_ecc       NWP_get_earth_ecc  

# define  nwp_set_earth_rad_m     NWP_set_earth_rad_m
# define  nwp_set_earth_rad_e     NWP_set_earth_rad_e
# define  nwp_set_earth_rad_p     NWP_set_earth_rad_p




int nwp_set_earth( int );

double nwp_get_earth_rad_m( void );
double nwp_get_earth_rad_e( void );
double nwp_get_earth_rad_p( void );
double nwp_get_earth_ecc  ( void );

void   nwp_set_earth_rad_m( double );
void   nwp_set_earth_rad_e( double );
void   nwp_set_earth_rad_p( double );


# endif
