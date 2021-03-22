! -------------------------------------------
! nwpl_map_earth.f90
! -------------------------------------------

module nwpl_map_earth

  real(8),parameter :: NWP_EARTH_DEFAULT_RAD_M = 6371.3d3
  real(8),parameter :: NWP_EARTH_BESSEL_RAD_E  = 6377.4d3
  real(8),parameter :: NWP_EARTH_BESSEL_RAD_P  = 6356.1d3
  real(8),parameter :: NWP_EARTH_BESSEL_ECC    = 8.166147d-2

  real(8),parameter :: NWP_EARTH_GRS80_RAD_M   = 6371.0d3
  real(8),parameter :: NWP_EARTH_GRS80_RAD_E   = 6378.1370d3
  real(8),parameter :: NWP_EARTH_GRS80_RAD_P   = 6356.7523d3
  real(8),parameter :: NWP_EARTH_GRS80_ECC     = 8.1819218d-2

  ! for VSRF Oblique Lambert.   bounded on ellipse at 37N
  real(8),parameter :: NWP_EARTH_GRS80_OBLM_RAD_M = 6372.2022d3

  integer(4),parameter :: NWP_EARTH_BESSEL      = 1001
  integer(4),parameter :: NWP_EARTH_GRS80       = 1002

CONTAINS

  subroutine nwp_get_earth_rad_m ( rr )
    real(8),intent(out) ::  rr 
    call nwpc_get_earth_rad_m( rr )
  end subroutine

  subroutine nwp_get_earth_rad_e ( rr )
    real(8),intent(out) ::  rr 
    call nwpc_get_earth_rad_e( rr )
  end subroutine

  subroutine nwp_get_earth_rad_p ( rr )
    real(8),intent(out) ::  rr 
    call nwpc_get_earth_rad_p( rr )
  end subroutine

  subroutine nwp_get_earth_ecc   ( rr )
    real(8),intent(out) ::  rr 
    call nwpc_get_earth_ecc( rr )
  end subroutine

  subroutine nwp_set_earth_rad_m ( rr )
    real(8),intent(in)  ::  rr 
    call nwpc_set_earth_rad_m( rr )
  end subroutine

  subroutine nwp_set_earth_rad_p ( rr )
    real(8),intent(in)  ::  rr 
    call nwpc_set_earth_rad_p( rr )
  end subroutine

  subroutine nwp_set_earth_rad_e ( rr )
    real(8),intent(in)  ::  rr 
    call nwpc_set_earth_rad_e( rr )
  end subroutine


  subroutine nwp_set_earth ( earth_id )
    integer(4),intent(in)  :: earth_id
    call nwpc_set_earth(earth_id)
  end subroutine

end module nwpl_map_earth
