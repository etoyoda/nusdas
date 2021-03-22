dnl
dnl to avoid error caused by single quotation marks in m4 statement
changequote(@<:@, @:>@)dnl
dnl
module nwpl_mapproj_fort
  implicit none

! private
! public nwp_ellipse2sphere, nwp_sphere2ellipse, &
!   & nwp_sphere2oblique, nwp_oblique2sphere, nwp_sphere2lambert, &
!   & nwp_lambert2sphere, nwp_sphere2mercator, nwp_mercator2sphere, &
!   & nwp_sphere2polar, nwp_polar2sphere, nwp_mf_lambert, &
!   & nwp_mf_mercator, nwp_mf_polar, nwp_sphere_distance  nwp_sphere2mercator2


  interface nwp_ellipse2sphere
    module procedure nwp_ellipse2sphere_f, nwp_ellipse2sphere_d
  end interface

  interface nwp_sphere2ellipse
    module procedure nwp_sphere2ellipse_f, nwp_sphere2ellipse_d
  end interface

  interface nwp_sphere2oblique
    module procedure nwp_sphere2oblique_f, nwp_sphere2oblique_d
  end interface

  interface nwp_oblique2sphere
    module procedure nwp_oblique2sphere_f, nwp_oblique2sphere_d
  end interface

  interface nwp_sphere2lambert
    module procedure nwp_sphere2lambert_f, nwp_sphere2lambert_d
  end interface

  interface nwp_lambert2sphere
    module procedure nwp_lambert2sphere_f, nwp_lambert2sphere_d
  end interface

  interface nwp_sphere2mercator
    module procedure nwp_sphere2mercator_f, nwp_sphere2mercator_d
  end interface

  interface nwp_sphere2mercator2
    module procedure nwp_sphere2mercator2_f, nwp_sphere2mercator2_d
  end interface

  interface nwp_mercator2sphere
    module procedure nwp_mercator2sphere_f, nwp_mercator2sphere_d
  end interface

  interface nwp_sphere2polar
    module procedure nwp_sphere2polar_f, nwp_sphere2polar_d
  end interface

  interface nwp_polar2sphere
    module procedure nwp_polar2sphere_f, nwp_polar2sphere_d
  end interface

  interface nwp_mf_lambert
    module procedure nwp_mf_lambert_f, nwp_mf_lambert_d
  end interface

  interface nwp_mf_mercator
    module procedure nwp_mf_mercator_f, nwp_mf_mercator_d
  end interface

  interface nwp_mf_polar
    module procedure nwp_mf_polar_f, nwp_mf_polar_d
  end interface

  interface nwp_sphere_distance
    module procedure nwp_distance_f, nwp_distance_d
  end interface

  contains

! =============================================================================
dnl
define(@<:@ELLIPSE2SPHERE@:>@, dnl
@<:@dnl
    subroutine nwp_ellipse2sphere_$1(elat, elon, size, slat, slon, &
      & lat, lon, iostat)
      implicit none
      real($2), intent(in) :: elat(*)
      real($2), intent(in) :: elon(*)
      integer, intent(in) :: size
      real($2), intent(in) :: slat
      real($2), intent(in) :: slon
      real($2), intent(out) :: lat(*)
      real($2), intent(out) :: lon(*)
      integer, intent(out) :: iostat

      call nwpc8_ellipse2sphere_$1(elat, elon, size, slat, slon, &
      & lat, lon, iostat)

    end subroutine nwp_ellipse2sphere_$1
@:>@)dnl
ELLIPSE2SPHERE(f, 4)
ELLIPSE2SPHERE(d, 8)dnl


! =============================================================================
dnl
define(@<:@SPHERE2ELLIPSE@:>@, dnl
@<:@dnl
    subroutine nwp_sphere2ellipse_$1(lat, lon, size, slat, slon, &
      & elat, elon, iostat)
      implicit none
      real($2), intent(in) :: lat(*)
      real($2), intent(in) :: lon(*)
      integer, intent(in) :: size
      real($2), intent(in) :: slat
      real($2), intent(in) :: slon
      real($2), intent(out) :: elat(*)
      real($2), intent(out) :: elon(*)
      integer, intent(out) :: iostat

      call nwpc8_sphere2ellipse_$1(lat, lon, size, slat, slon, &
      & elat, elon, iostat)

    end subroutine nwp_sphere2ellipse_$1
@:>@)dnl
SPHERE2ELLIPSE(f, 4)
SPHERE2ELLIPSE(d, 8)dnl


! =============================================================================
dnl
define(@<:@SPHERE2OBLIQUE@:>@, dnl
@<:@dnl
    subroutine nwp_sphere2oblique_$1(lat, lon, size, slat, slon, &
      & olat, olon, iostat)
      implicit none
      real($2), intent(in) :: lat(*)
      real($2), intent(in) :: lon(*)
      integer, intent(in) :: size
      real($2), intent(in) :: slat
      real($2), intent(in) :: slon
      real($2), intent(out) :: olat(*)
      real($2), intent(out) :: olon(*)
      integer, intent(out) :: iostat

      call nwpc8_sphere2oblique_$1(lat, lon, size, slat, slon, &
      & olat, olon, iostat)

    end subroutine nwp_sphere2oblique_$1
@:>@)dnl
SPHERE2OBLIQUE(f, 4)
SPHERE2OBLIQUE(d, 8)dnl


! =============================================================================
dnl
define(@<:@OBLIQUE2SPHERE@:>@, dnl
@<:@dnl
    subroutine nwp_oblique2sphere_$1(olat, olon, size, slat, slon, &
      & lat, lon, iostat)
      implicit none
      real($2), intent(in) :: olat(*)
      real($2), intent(in) :: olon(*)
      integer, intent(in) :: size
      real($2), intent(in) :: slat
      real($2), intent(in) :: slon
      real($2), intent(out) :: lat(*)
      real($2), intent(out) :: lon(*)
      integer, intent(out) :: iostat

      call nwpc8_oblique2sphere_$1(olat, olon, size, slat, slon, &
      & lat, lon, iostat)

    end subroutine nwp_oblique2sphere_$1
@:>@)dnl
OBLIQUE2SPHERE(f, 4)
OBLIQUE2SPHERE(d, 8)dnl


! =============================================================================
dnl
define(@<:@SPHERE2LAMBERT@:>@, dnl
@<:@dnl
    subroutine nwp_sphere2lambert_$1(lat, lon, size, slat1, slat2, slon, &
      & rlat, rlon, rx, ry, d, x, y, iostat)
      implicit none
      real($2), intent(in) :: lat(*)
      real($2), intent(in) :: lon(*)
      integer, intent(in) :: size
      real($2), intent(in) :: slat1
      real($2), intent(in) :: slat2
      real($2), intent(in) :: slon
      real($2), intent(in) :: rlat
      real($2), intent(in) :: rlon
      real($2), intent(in) :: rx
      real($2), intent(in) :: ry
      real($2), intent(in) :: d
      real($2), intent(out) :: x(*)
      real($2), intent(out) :: y(*)
      integer, intent(out) :: iostat

      call nwpc8_sphere2lambert_$1(lat, lon, size, slat1, slat2, slon, &
      & rlat, rlon, rx, ry, d, x, y, iostat)

    end subroutine nwp_sphere2lambert_$1
@:>@)dnl
SPHERE2LAMBERT(f, 4)
SPHERE2LAMBERT(d, 8)dnl


! =============================================================================
dnl
define(@<:@LAMBERT2SPHERE@:>@, dnl
@<:@dnl
    subroutine nwp_lambert2sphere_$1(x, y, size, slat1, slat2, slon, &
      & rlat, rlon, rx, ry, d, lat, lon, iostat)
      implicit none
      real($2), intent(in) :: x(*)
      real($2), intent(in) :: y(*)
      integer, intent(in) :: size
      real($2), intent(in) :: slat1
      real($2), intent(in) :: slat2
      real($2), intent(in) :: slon
      real($2), intent(in) :: rlat
      real($2), intent(in) :: rlon
      real($2), intent(in) :: rx
      real($2), intent(in) :: ry
      real($2), intent(in) :: d
      real($2), intent(out) :: lat(*)
      real($2), intent(out) :: lon(*)
      integer, intent(out) :: iostat

      call nwpc8_lambert2sphere_$1(x, y, size, slat1, slat2, slon, &
      & rlat, rlon, rx, ry, d, lat, lon, iostat)

    end subroutine nwp_lambert2sphere_$1
@:>@)dnl
LAMBERT2SPHERE(f, 4)
LAMBERT2SPHERE(d, 8)dnl


! =============================================================================
dnl
define(@<:@SPHERE2MERCATOR@:>@, dnl
@<:@dnl
    subroutine nwp_sphere2mercator_$1(lat, lon, size, slat, rlat, rlon, &
      & rx, ry, d, x, y, iostat)
      implicit none
      real($2), intent(in) :: lat(*)
      real($2), intent(in) :: lon(*)
      integer, intent(in) :: size
      real($2), intent(in) :: slat
      real($2), intent(in) :: rlat
      real($2), intent(in) :: rlon
      real($2), intent(in) :: rx
      real($2), intent(in) :: ry
      real($2), intent(in) :: d
      real($2), intent(out) :: x(*)
      real($2), intent(out) :: y(*)
      integer, intent(out) :: iostat

      call nwpc8_sphere2mercator_$1(lat, lon, size, slat, rlat, rlon, &
      & rx, ry, d, x, y, iostat)

    end subroutine nwp_sphere2mercator_$1
@:>@)dnl
SPHERE2MERCATOR(f, 4)
SPHERE2MERCATOR(d, 8)dnl

! =============================================================================
dnl
define(@<:@SPHERE2MERCATOR2@:>@, dnl
@<:@dnl
    subroutine nwp_sphere2mercator2_$1(lat, lon, size, slat, rlat, rlon, &
      & rx, ry, d, x, y, iostat)
      implicit none
      real($2), intent(in) :: lat(*)
      real($2), intent(in) :: lon(*)
      integer, intent(in) :: size
      real($2), intent(in) :: slat
      real($2), intent(in) :: rlat
      real($2), intent(in) :: rlon
      real($2), intent(in) :: rx
      real($2), intent(in) :: ry
      real($2), intent(in) :: d
      real($2), intent(out) :: x(*)
      real($2), intent(out) :: y(*)
      integer, intent(out) :: iostat

      call nwpc8_sphere2mercator2_$1(lat, lon, size, slat, rlat, rlon, &
      & rx, ry, d, x, y, iostat)

    end subroutine nwp_sphere2mercator2_$1
@:>@)dnl
SPHERE2MERCATOR2(f, 4)
SPHERE2MERCATOR2(d, 8)dnl

! =============================================================================
dnl
define(@<:@MERCATOR2SPHERE@:>@, dnl
@<:@dnl
    subroutine nwp_mercator2sphere_$1(x, y, size, slat, rlat, rlon, &
      & rx, ry, d, lat, lon, iostat)
      implicit none
      real($2), intent(in) :: x(*)
      real($2), intent(in) :: y(*)
      integer, intent(in) :: size
      real($2), intent(in) :: slat
      real($2), intent(in) :: rlat
      real($2), intent(in) :: rlon
      real($2), intent(in) :: rx
      real($2), intent(in) :: ry
      real($2), intent(in) :: d
      real($2), intent(out) :: lat(*)
      real($2), intent(out) :: lon(*)
      integer, intent(out) :: iostat

      call nwpc8_mercator2sphere_$1(x, y, size, slat, rlat, rlon, &
      & rx, ry, d, lat, lon, iostat)

    end subroutine nwp_mercator2sphere_$1
@:>@)dnl
MERCATOR2SPHERE(f, 4)
MERCATOR2SPHERE(d, 8)dnl


! =============================================================================
dnl
define(@<:@SPHERE2POLAR@:>@, dnl
@<:@dnl
    subroutine nwp_sphere2polar_$1(lat, lon, size, slat, slon, &
      & rlat, rlon, rx, ry, d, x, y, iostat)
      implicit none
      real($2), intent(in) :: lat(*)
      real($2), intent(in) :: lon(*)
      integer, intent(in) :: size
      real($2), intent(in) :: slat
      real($2), intent(in) :: slon
      real($2), intent(in) :: rlat
      real($2), intent(in) :: rlon
      real($2), intent(in) :: rx
      real($2), intent(in) :: ry
      real($2), intent(in) :: d
      real($2), intent(out) :: x(*)
      real($2), intent(out) :: y(*)
      integer, intent(out) :: iostat

      call nwpc8_sphere2polar_$1(lat, lon, size, slat, slon, &
      & rlat, rlon, rx, ry, d, x, y, iostat)

    end subroutine nwp_sphere2polar_$1
@:>@)dnl
SPHERE2POLAR(f, 4)
SPHERE2POLAR(d, 8)dnl


! =============================================================================
dnl
define(@<:@POLAR2SPHERE@:>@, dnl
@<:@dnl
    subroutine nwp_polar2sphere_$1(x, y, size, slat, slon, &
      & rlat, rlon, rx, ry, d, lat, lon, iostat)
      implicit none
      real($2), intent(in) :: x(*)
      real($2), intent(in) :: y(*)
      integer, intent(in) :: size
      real($2), intent(in) :: slat
      real($2), intent(in) :: slon
      real($2), intent(in) :: rlat
      real($2), intent(in) :: rlon
      real($2), intent(in) :: rx
      real($2), intent(in) :: ry
      real($2), intent(in) :: d
      real($2), intent(out) :: lat(*)
      real($2), intent(out) :: lon(*)
      integer, intent(out) :: iostat

      call nwpc8_polar2sphere_$1(x, y, size, slat, slon, &
      & rlat, rlon, rx, ry, d, lat, lon, iostat)

    end subroutine nwp_polar2sphere_$1
@:>@)dnl
POLAR2SPHERE(f, 4)
POLAR2SPHERE(d, 8)dnl


! =============================================================================
dnl
define(@<:@MF_LAMBERT@:>@, dnl
@<:@dnl
    subroutine nwp_mf_lambert_$1(lat, size, slat1, slat2, &
      & mf, iostat)
      implicit none
      real($2), intent(in) :: lat(*)
      integer, intent(in) :: size
      real($2), intent(in) :: slat1
      real($2), intent(in) :: slat2
      real($2), intent(out) :: mf(*)
      integer, intent(out) :: iostat

      call nwpc8_mf_lambert_$1(lat, size, slat1, slat2, &
      & mf, iostat)

    end subroutine nwp_mf_lambert_$1
@:>@)dnl
MF_LAMBERT(f, 4)
MF_LAMBERT(d, 8)dnl


! =============================================================================
dnl
define(@<:@MF_MERCATOR@:>@, dnl
@<:@dnl
    subroutine nwp_mf_mercator_$1(lat, size, slat, mf, iostat)
      implicit none
      real($2), intent(in) :: lat(*)
      integer, intent(in) :: size
      real($2), intent(in) :: slat
      real($2), intent(out) :: mf(*)
      integer, intent(out) :: iostat

      call nwpc8_mf_mercator_$1(lat, size, slat, mf, iostat)

    end subroutine nwp_mf_mercator_$1
@:>@)dnl
MF_MERCATOR(f, 4)
MF_MERCATOR(d, 8)dnl


! =============================================================================
dnl
define(@<:@MF_POLAR@:>@, dnl
@<:@dnl
    subroutine nwp_mf_polar_$1(lat, size, slat, mf, iostat)
      implicit none
      real($2), intent(in) :: lat(*)
      integer, intent(in) :: size
      real($2), intent(in) :: slat
      real($2), intent(out) :: mf(*)
      integer, intent(out) :: iostat

      call nwpc8_mf_polar_$1(lat, size, slat, mf, iostat)

    end subroutine nwp_mf_polar_$1
@:>@)dnl
MF_POLAR(f, 4)
MF_POLAR(d, 8)dnl


! =============================================================================
dnl
define(@<:@DISTANCE@:>@, dnl
@<:@dnl
    subroutine nwp_distance_$1(alat, alon, blat, blon, dist)
      implicit none
      real($2), intent(in) :: alat
      real($2), intent(in) :: alon
      real($2), intent(in) :: blat
      real($2), intent(in) :: blon
      real($2), intent(out) ::dist

      call nwpc8_distance_$1(alat, alon, blat, blon, dist)

    end subroutine nwp_distance_$1
@:>@)dnl
DISTANCE(f, 4)
DISTANCE(d, 8)dnl


end module nwpl_mapproj_fort


