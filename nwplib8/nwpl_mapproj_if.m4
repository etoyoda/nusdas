dnl
dnl to avoid error caused by single quotation marks in m4 statement
changequote(@<:@, @:>@)dnl
dnl
module nwpl_mapproj_if
  implicit none

  interface NWP_MPROJ_EL2SP
    module procedure NWPS_MPROJ_EL2SP, NWPD_MPROJ_EL2SP
  end interface

  interface NWP_MPROJ_SP2EL
    module procedure NWPS_MPROJ_SP2EL, NWPD_MPROJ_SP2EL 
  end interface

  interface NWP_MPROJ_SP2OB
    module procedure NWPS_MPROJ_SP2OB, NWPD_MPROJ_SP2OB
  end interface

  interface NWP_MPROJ_OB2SP
    module procedure NWPS_MPROJ_OB2SP, NWPD_MPROJ_OB2SP
  end interface

  interface NWP_MPROJ_LL2LM
    module procedure NWPS_MPROJ_LL2LM, NWPD_MPROJ_LL2LM
  end interface

  interface NWP_MPROJ_LM2LL
    module procedure NWPS_MPROJ_LM2LL, NWPD_MPROJ_LM2LL
  end interface

  interface NWP_MPROJ_LL2MR
    module procedure NWPS_MPROJ_LL2MR, NWPD_MPROJ_LL2MR
  end interface

  interface NWP_MPROJ_MR2LL
    module procedure NWPS_MPROJ_MR2LL, NWPD_MPROJ_MR2LL
  end interface

  interface NWP_MPROJ_LL2PS
    module procedure NWPS_MPROJ_LL2PS, NWPD_MPROJ_LL2PS
  end interface

  interface NWP_MPROJ_PS2LL
    module procedure NWPS_MPROJ_PS2LL, NWPD_MPROJ_PS2LL
  end interface

  interface NWP_MPROJ_ELL2OLM
    module procedure NWPS_MPROJ_ELL2OLM, NWPD_MPROJ_ELL2OLM
  end interface

  interface NWP_MPROJ_OLM2ELL
    module procedure NWPS_MPROJ_OLM2ELL, NWPD_MPROJ_OLM2ELL
  end interface

  interface NWP_MF_LM
    module procedure NWPS_MF_LM, NWPD_MF_LM
  end interface

  interface NWP_MF_MR
    module procedure NWPS_MF_MR, NWPD_MF_MR
  end interface

  interface NWP_MF_PS
    module procedure NWPS_MF_PS, NWPD_MF_PS
  end interface

  interface NWP_DISTANCE
    module procedure NWPS_DISTANCE, NWPD_DISTANCE 
  end interface

  contains 

! =============================================================================
dnl
define(@<:@ELLIPSE2SPHERE@:>@, dnl
@<:@dnl
    subroutine NWP$3_MPROJ_EL2SP(lat, lon, elat, elon, im, jm, slat, opt_slon)
      implicit none
      real($2), intent(out) :: lat(*)
      real($2), intent(out) :: lon(*)
      real($2), intent(in) :: elat(*)
      real($2), intent(in) :: elon(*)
      integer, intent(in) :: im
      integer, intent(in) :: jm
      real($2), intent(in) :: slat
      real($2), intent(in), optional :: opt_slon
      
      real($2) :: slon
      integer :: isize, iostat
      isize = im * jm
      slon = 0.0
      if( present( opt_slon ) ) slon = opt_slon

      call nwpc8_ellipse2sphere_$1(elat, elon, isize, slat, slon, &
      & lat, lon, iostat)

    end subroutine NWP$3_MPROJ_EL2SP
@:>@)dnl
ELLIPSE2SPHERE(f, 4, S)
ELLIPSE2SPHERE(d, 8, D)dnl

! =============================================================================
dnl
define(@<:@SPHERE2ELLIPSE@:>@, dnl
@<:@dnl
    subroutine NWP$3_MPROJ_SP2EL(elat, elon, lat, lon, im, jm, slat, opt_slon)
      implicit none
      real($2), intent(out) :: elat(*)
      real($2), intent(out) :: elon(*)
      real($2), intent(in) :: lat(*)
      real($2), intent(in) :: lon(*)
      integer, intent(in) :: im
      integer, intent(in) :: jm
      real($2), intent(in) :: slat
      real($2), intent(in), optional :: opt_slon

      real($2) :: slon
      integer :: isize, iostat
      isize = im * jm
      slon = 0.0
      if( present( opt_slon ) ) slon = opt_slon
      
      call nwpc8_sphere2ellipse_$1(lat, lon, isize, slat, slon, &
      & elat, elon, iostat)

    end subroutine NWP$3_MPROJ_SP2EL
@:>@)dnl
SPHERE2ELLIPSE(f, 4, S)
SPHERE2ELLIPSE(d, 8, D)dnl

! =============================================================================
dnl
define(@<:@SPHERE2OBLIQUE@:>@, dnl
@<:@dnl
    subroutine NWP$3_MPROJ_SP2OB(olat, olon, lat, lon, im, jm, &
      & slat, slon)
      implicit none
      real($2), intent(out) :: olat(*)
      real($2), intent(out) :: olon(*)
      real($2), intent(in) :: lat(*)
      real($2), intent(in) :: lon(*)
      integer, intent(in) :: im
      integer, intent(in) :: jm
      real($2), intent(in) :: slat
      real($2), intent(in) :: slon

      integer :: isize, iostat
          isize = im * jm
      
      call nwpc8_sphere2oblique_$1(lat, lon, isize, slat, slon, &
      & olat, olon, iostat)

    end subroutine NWP$3_MPROJ_SP2OB
@:>@)dnl
SPHERE2OBLIQUE(f, 4, S)
SPHERE2OBLIQUE(d, 8, D)dnl


! =============================================================================
dnl
define(@<:@OBLIQUE2SPHERE@:>@, dnl
@<:@dnl
    subroutine NWP$3_MPROJ_OB2SP(lat, lon, olat, olon, im, jm, &
      & slat, slon)
      implicit none
      real($2), intent(out) :: lat(*)
      real($2), intent(out) :: lon(*)
      real($2), intent(in) :: olat(*)
      real($2), intent(in) :: olon(*)
      integer, intent(in) :: im
      integer, intent(in) :: jm
      real($2), intent(in) :: slat
      real($2), intent(in) :: slon

      integer :: isize, iostat
          isize = im * jm

      call nwpc8_oblique2sphere_$1(olat, olon, isize, slat, slon, &
      & lat, lon, iostat)

    end subroutine NWP$3_MPROJ_OB2SP
@:>@)dnl
OBLIQUE2SPHERE(f, 4, S)
OBLIQUE2SPHERE(d, 8, D)dnl


! =============================================================================
dnl
define(@<:@SPHERE2LAMBERT@:>@, dnl
@<:@dnl
    subroutine NWP$3_MPROJ_LL2LM(x, y, lat, lon, im, jm, d, &
      & slat1, slat2, slon, rx, ry, rlat, rlon)
      implicit none
      real($2), intent(out) :: x(*)
      real($2), intent(out) :: y(*)
      real($2), intent(in) :: lat(*)
      real($2), intent(in) :: lon(*)
      integer, intent(in) :: im
      integer, intent(in) :: jm
      real($2), intent(in) :: d
      real($2), intent(in) :: slat1
      real($2), intent(in) :: slat2
      real($2), intent(in) :: slon
      real($2), intent(in) :: rx
      real($2), intent(in) :: ry
      real($2), intent(in) :: rlat
      real($2), intent(in) :: rlon

      integer :: isize, iostat
          isize = im * jm
      
      call nwpc8_sphere2lambert_$1(lat, lon, isize, slat1, slat2, slon, &
      & rlat, rlon, rx, ry, d, x, y, iostat)

    end subroutine NWP$3_MPROJ_LL2LM
@:>@)dnl
SPHERE2LAMBERT(f, 4, S)
SPHERE2LAMBERT(d, 8, D)dnl

! =============================================================================
dnl
define(@<:@LAMBERT2SPHERE@:>@, dnl
@<:@dnl
    subroutine NWP$3_MPROJ_LM2LL(lat, lon, im, jm, d, slat1, slat2, &
      & slon, rx, ry, rlat, rlon)
      implicit none
      real($2), intent(out) :: lat(*)
      real($2), intent(out) :: lon(*)
      integer, intent(in) :: im
      integer, intent(in) :: jm
      real($2), intent(in) :: d
      real($2), intent(in) :: slat1
      real($2), intent(in) :: slat2
      real($2), intent(in) :: slon
      real($2), intent(in) :: rx
      real($2), intent(in) :: ry
      real($2), intent(in) :: rlat
      real($2), intent(in) :: rlon

      integer :: isize, iostat
      integer :: i, j, ii
       
      real($2) :: x(im * jm)
      real($2) :: y(im * jm)

      isize = im * jm
!POPTION PARALLEL, TLOCAL(I, J, II)
      do j= 1, jm
        do i= 1, im
          ii = (j - 1) * im + i
          x(ii) = i
          y(ii) = j
        end do 
      end do 

      call nwpc8_lambert2sphere_$1(x, y, isize, slat1, slat2, slon, &
      & rlat, rlon, rx, ry, d, lat, lon, iostat)

    end subroutine NWP$3_MPROJ_LM2LL
@:>@)dnl
LAMBERT2SPHERE(f, 4, S)
LAMBERT2SPHERE(d, 8, D)dnl

! =============================================================================
dnl
define(@<:@SPHERE2MERCATOR@:>@, dnl
@<:@dnl
    subroutine NWP$3_MPROJ_LL2MR(x, y, lat, lon, im, jm, d, slat, &
      rx, ry, rlat, rlon)
      implicit none
      real($2), intent(out) :: x(*)
      real($2), intent(out) :: y(*)
      real($2), intent(in) :: lat(*)
      real($2), intent(in) :: lon(*)
      integer, intent(in) :: im
      integer, intent(in) :: jm
      real($2), intent(in) :: d
      real($2), intent(in) :: slat
      real($2), intent(in) :: rx
      real($2), intent(in) :: ry
      real($2), intent(in) :: rlat
      real($2), intent(in) :: rlon

      integer :: isize, iostat
          isize = im * jm
      
      call nwpc8_sphere2mercator_$1(lat, lon, isize, slat, rlat, rlon, &
      & rx, ry, d, x, y, iostat)

    end subroutine NWP$3_MPROJ_LL2MR
@:>@)dnl
SPHERE2MERCATOR(f, 4, S)
SPHERE2MERCATOR(d, 8, D)dnl

! =============================================================================
dnl
define(@<:@SPHERE2MERCATOR2@:>@, dnl
@<:@dnl
    subroutine NWP$3_MPROJ_LL2MR2(x, y, lat, lon, im, jm, d, slat, &
      rx, ry, rlat, rlon)
      implicit none
      real($2), intent(out) :: x(*)
      real($2), intent(out) :: y(*)
      real($2), intent(in) :: lat(*)
      real($2), intent(in) :: lon(*)
      integer, intent(in) :: im
      integer, intent(in) :: jm
      real($2), intent(in) :: d
      real($2), intent(in) :: slat
      real($2), intent(in) :: rx
      real($2), intent(in) :: ry
      real($2), intent(in) :: rlat
      real($2), intent(in) :: rlon

      integer :: isize, iostat
          isize = im * jm
      
      call nwpc8_sphere2mercator2_$1(lat, lon, isize, slat, rlat, rlon, &
      & rx, ry, d, x, y, iostat)

    end subroutine NWP$3_MPROJ_LL2MR2
@:>@)dnl
SPHERE2MERCATOR2(f, 4, S)
SPHERE2MERCATOR2(d, 8, D)dnl

! =============================================================================
dnl
define(@<:@MERCATOR2SPHERE@:>@, dnl
@<:@dnl
    subroutine NWP$3_MPROJ_MR2LL(lat, lon, im, jm, d, slat, &
      & rx, ry, rlat, rlon)
      implicit none
      real($2), intent(out) :: lat(*)
      real($2), intent(out) :: lon(*)
      integer, intent(in) :: im
      integer, intent(in) :: jm
      real($2), intent(in) :: d
      real($2), intent(in) :: slat
      real($2), intent(in) :: rx
      real($2), intent(in) :: ry
      real($2), intent(in) :: rlat
      real($2), intent(in) :: rlon

      integer :: isize, iostat
      
      real($2) :: x(im * jm)
      real($2) :: y(im * jm)
      integer :: i, j, ii

      isize = im * jm

!POPTION PARALLEL, TLOCAL(I, J, II)
      do j= 1, jm
        do i= 1, im
         ii = (j - 1) * im + i
         x(ii) = i
         y(ii) = j
        end do 
      end do 
      
      call nwpc8_mercator2sphere_$1(x, y, isize, slat, rlat, rlon, &
      & rx, ry, d, lat, lon, iostat)

    end subroutine NWP$3_MPROJ_MR2LL
@:>@)dnl
MERCATOR2SPHERE(f, 4, S)
MERCATOR2SPHERE(d, 8, D)dnl

! =============================================================================
dnl
define(@<:@SPHERE2POLAR@:>@, dnl
@<:@dnl
    subroutine NWP$3_MPROJ_LL2PS(x, y, lat, lon, im, jm, d, slat, slon, &
      & rx, ry, rlat, rlon)
      implicit none
      real($2), intent(out) :: x(*)
      real($2), intent(out) :: y(*)
      real($2), intent(in) :: lat(*)
      real($2), intent(in) :: lon(*)
      integer, intent(in) :: im
      integer, intent(in) :: jm
      real($2), intent(in) :: d
      real($2), intent(in) :: slat
      real($2), intent(in) :: slon
      real($2), intent(in) :: rx
      real($2), intent(in) :: ry
      real($2), intent(in) :: rlat
      real($2), intent(in) :: rlon

      integer :: isize, iostat
          isize = im * jm
      
      call nwpc8_sphere2polar_$1(lat, lon, isize, slat, slon, &
      & rlat, rlon, rx, ry, d, x, y, iostat)

    end subroutine NWP$3_MPROJ_LL2PS
@:>@)dnl
SPHERE2POLAR(f, 4, S)
SPHERE2POLAR(d, 8, D)dnl

! =============================================================================
dnl
define(@<:@POLAR2SPHERE@:>@, dnl
@<:@dnl
    subroutine NWP$3_MPROJ_PS2LL(lat, lon, im, jm, d, slat, slon, &
      & rx, ry, rlat, rlon)
      implicit none
      real($2), intent(out) :: lat(*)
      real($2), intent(out) :: lon(*)
      integer, intent(in) :: im
      integer, intent(in) :: jm
      real($2), intent(in) :: d
      real($2), intent(in) :: slat
      real($2), intent(in) :: slon
      real($2), intent(in) :: rx
      real($2), intent(in) :: ry
      real($2), intent(in) :: rlat
      real($2), intent(in) :: rlon

      integer :: isize, iostat
      
      real($2) :: x(im * jm)
      real($2) :: y(im * jm)
      integer :: i, j, ii

      isize = im * jm

!POPTION PARALLEL, TLOCAL(I, J, II)
      do j= 1, jm
        do i= 1, im
         ii = (j - 1) * im + i
         x(ii) = i
         y(ii) = j
        end do 
      end do 
      
      call nwpc8_polar2sphere_$1(x, y, isize, slat, slon, &
      & rlat, rlon, rx, ry, d, lat, lon, iostat)

    end subroutine NWP$3_MPROJ_PS2LL
@:>@)dnl
POLAR2SPHERE(f, 4, S)
POLAR2SPHERE(d, 8, D)dnl


! =============================================================================
dnl
define(@<:@SPHERE2OBLIQUELAMBERT@:>@, dnl
@<:@dnl
    subroutine NWP$3_MPROJ_ELL2OLM(x, y, elat, elon, im, jm, dels, &
      & slate, slatp, rx, ry, rlat, rlon, oslat, oslon, &
      & slat_el2sp, slon_el2sp_opt )
      implicit none
      real($2), intent(out) :: x(*)
      real($2), intent(out) :: y(*)
      real($2), intent(in) :: elat(*)
      real($2), intent(in) :: elon(*)
      integer,  intent(in) :: im
      integer,  intent(in) :: jm
      real($2), intent(in) :: dels
      real($2), intent(in) :: slate
      real($2), intent(in) :: slatp
      real($2), intent(in) :: rx
      real($2), intent(in) :: ry
      real($2), intent(in) :: rlat
      real($2), intent(in) :: rlon
      real($2), intent(in) :: oslat
      real($2), intent(in) :: oslon
      real($2), intent(in) :: slat_el2sp
      real($2), intent(in), optional :: slon_el2sp_opt

      real($2) :: rlat2(1), rlon2(1)
      real($2) :: splat(im*jm), splon(im*jm), oblat(im*jm), oblon(im*jm)
      real($2) :: slon_el2sp
      slon_el2sp = 0
      if( PRESENT(slon_el2sp_opt) ) slon_el2sp = slon_el2sp_opt

      call nwp_mproj_el2sp( splat, splon, (/rlat/), (/rlon/), 1,1, &
			  & slat_el2sp, slon_el2sp )
      call nwp_mproj_sp2ob( rlat2, rlon2, splat, splon, 1,1, oslat, oslon )

      call nwp_mproj_el2sp( splat, splon, elat, elon, im, jm, &
			  & slat_el2sp, slon_el2sp )
      call nwp_mproj_sp2ob( oblat, oblon, splat, splon, im, jm, oslat, oslon )
      
      call nwp_mproj_ll2lm( x, y, oblat, oblon, im, jm, dels, &
                          & slate, slatp, rlon2(1), rx, ry, rlat2(1), rlon2(1) )

    end subroutine NWP$3_MPROJ_ELL2OLM
@:>@)dnl
SPHERE2OBLIQUELAMBERT(f, 4, S)
SPHERE2OBLIQUELAMBERT(d, 8, D)dnl

! =============================================================================
dnl
define(@<:@OBLIQUELAMBERT2SPHERE@:>@, dnl
@<:@dnl
    subroutine NWP$3_MPROJ_OLM2ELL(elat, elon, im, jm, dels, slate, slatp, &
      & rx, ry, rlat, rlon, oslat, oslon, slat_el2sp, slon_el2sp_opt )
      implicit none
      real($2), intent(out) :: elat(*)
      real($2), intent(out) :: elon(*)
      integer,  intent(in) :: im
      integer,  intent(in) :: jm
      real($2), intent(in) :: dels
      real($2), intent(in) :: slate
      real($2), intent(in) :: slatp
      real($2), intent(in) :: rx
      real($2), intent(in) :: ry
      real($2), intent(in) :: rlat
      real($2), intent(in) :: rlon
      real($2), intent(in) :: oslat
      real($2), intent(in) :: oslon
      real($2), intent(in) :: slat_el2sp
      real($2), intent(in), optional :: slon_el2sp_opt

      real($2) :: splat(im*jm), splon(im*jm), oblat(im*jm), oblon(im*jm)
      real($2) :: rlat2(1), rlon2(1)
      real($2) :: slon_el2sp

      slon_el2sp = 0.
      if( PRESENT(slon_el2sp_opt) ) slon_el2sp = slon_el2sp_opt

      call nwp_mproj_el2sp( splat, splon, (/rlat/), (/rlon/), 1,1, &
			  & slat_el2sp, slon_el2sp )

      call nwp_mproj_sp2ob( rlat2, rlon2, splat, splon, 1,1, oslat, oslon )

      call nwp_mproj_lm2ll( oblat, oblon, im, jm, dels, &
                          & slate, slatp, rlon2(1), rx, ry, rlat2(1), rlon2(1) )

      call nwp_mproj_ob2sp( splat, splon, oblat, oblon, im, jm, oslat, oslon )

      call nwp_mproj_sp2el( elat, elon, splat, splon, im, jm, &
			  & slat_el2sp, slon_el2sp )

    end subroutine NWP$3_MPROJ_OLM2ELL
@:>@)dnl
OBLIQUELAMBERT2SPHERE(f, 4, S)
OBLIQUELAMBERT2SPHERE(d, 8, D)dnl

! =============================================================================
dnl
define(@<:@MF_LAMBERT@:>@, dnl
@<:@dnl
    subroutine NWP$3_MF_LM(mf, lat, im, jm, slat1, slat2)
      implicit none
      real($2), intent(out) :: mf(*)
      real($2), intent(in) :: lat(*)
      integer, intent(in) :: im
      integer, intent(in) :: jm
      real($2), intent(in) :: slat1
      real($2), intent(in) :: slat2

      integer :: isize, iostat
          isize = im * jm
      
      call nwpc8_mf_lambert_$1(lat, isize, slat1, slat2, mf, iostat)

    end subroutine NWP$3_MF_LM
@:>@)dnl
MF_LAMBERT(f, 4, S)
MF_LAMBERT(d, 8, D)dnl

! =============================================================================
dnl
define(@<:@MF_MERCATOR@:>@, dnl
@<:@dnl
    subroutine NWP$3_MF_MR(mf, lat, im, jm, slat)
      implicit none
      real($2), intent(out) :: mf(*)
      real($2), intent(in) :: lat(*)
      integer, intent(in) :: im
      integer, intent(in) :: jm
      real($2), intent(in) :: slat

      integer :: isize, iostat
          isize = im * jm
      
      call nwpc8_mf_mercator_$1(lat, isize, slat, mf, iostat)

    end subroutine NWP$3_MF_MR
@:>@)dnl
MF_MERCATOR(f, 4, S)
MF_MERCATOR(d, 8, D)dnl

! =============================================================================
dnl
define(@<:@MF_POLAR@:>@, dnl
@<:@dnl
    subroutine NWP$3_MF_PS(mf, lat, im, jm, slat)
      implicit none
      real($2), intent(out) :: mf(*)
      real($2), intent(in) :: lat(*)
      integer, intent(in) :: im
      integer, intent(in) :: jm
      real($2), intent(in) :: slat

      integer :: isize, iostat
          isize = im * jm
      
      call nwpc8_mf_polar_$1(lat, isize, slat, mf, iostat)

    end subroutine NWP$3_MF_PS
@:>@)dnl
MF_POLAR(f, 4, S)
MF_POLAR(d, 8, D)dnl

! =============================================================================
dnl
define(@<:@DISTANCE@:>@, dnl
@<:@dnl
    subroutine NWP$3_DISTANCE(alat, alon, blat, blon, dist)
      implicit none
      real($2), intent(in) :: alat
      real($2), intent(in) :: alon
      real($2), intent(in) :: blat
      real($2), intent(in) :: blon
      real($2), intent(out) ::dist 

      call nwpc8_distance_$1(alat, alon, blat, blon, dist)

    end subroutine NWP$3_DISTANCE
@:>@)dnl
DISTANCE(f, 4, S)
DISTANCE(d, 8, D)dnl

end module nwpl_mapproj_if

