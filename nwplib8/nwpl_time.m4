dnl nwpl_time.m4: source for nwpl_time.f90
dnl
dnl If you see this line, you can ignore the next one.
! DO NOT EDIT; this file is produced from the corresponding m4 source
! as follows:
!   m4 source.m4 > thisfile
!
! NWP LIB: Numerical Weather Prediction Library
!
! nwpl_time.f90: NWP LIB Fortran interface block
!
dnl
dnl to avoid error caused by single quotation marks in m4 statement
changequote(@<:@, @:>@)dnl
dnl

module nwpl_time
  implicit none
  private
  public nwp_ymd2seq, nwp_ymdh2seq, nwp_ymdhm2seq, &
    &    nwp_seq2ymd, nwp_seq2ymdh, nwp_seq2ymdhm, &
    &    nwp_gettime, nwp_systime, nwp_filestat,   &
    &    nwp_lustre_recover, nwp_lustre_close,     &
    &    nwp_lustre_simple_close

  contains
    subroutine nwp_ymd2seq(iy, im, id, iseq)
      integer, intent(in) :: iy, im, id
      integer, intent(out) :: iseq

      call nwpc_ymd2seq(iy, im, id, iseq)
    end subroutine nwp_ymd2seq

    subroutine nwp_ymdh2seq(iy, im, id, ih, iseq)
      integer, intent(in) :: iy, im, id, ih
      integer, intent(out) :: iseq

      call nwpc_ymdh2seq(iy, im, id, ih, iseq)
    end subroutine nwp_ymdh2seq

    subroutine nwp_ymdhm2seq(iy, im, id, ih, imn, iseq)
      integer, intent(in) :: iy, im, id, ih, imn
      integer, intent(out) :: iseq

      call nwpc_ymdhm2seq(iy, im, id, ih, imn, iseq)
    end subroutine nwp_ymdhm2seq

    subroutine nwp_seq2ymd(py, pm, pd, iseq)
      integer, intent(out) :: py, pm, pd
      integer, intent(in) :: iseq

      call nwpc_seq2ymd(py, pm, pd, iseq)
    end subroutine nwp_seq2ymd

    subroutine nwp_seq2ymdh(py, pm, pd, ph, iseq)
      integer, intent(out) :: py, pm, pd, ph
      integer, intent(in) :: iseq

      call nwpc_seq2ymdh(py, pm, pd, ph, iseq)
    end subroutine nwp_seq2ymdh

    subroutine nwp_seq2ymdhm(py, pm, pd, ph, pmn, iseq)
      integer, intent(out) :: py, pm, pd, ph, pmn
      integer, intent(in) :: iseq

      call nwpc_seq2ymdhm(py, pm, pd, ph, pmn, iseq)
    end subroutine nwp_seq2ymdhm

    subroutine nwp_gettime(tcname, py, pm, pd, ph, pmn, iseq)
      character(*), intent(in) :: tcname
      integer, intent(out) :: py, pm, pd, ph, pmn, iseq

      call nwpc_gettime(tcname, py, pm, pd, ph, pmn, iseq)
    end subroutine nwp_gettime

    subroutine nwp_systime(py, pm, pd, ph, pmn, ps, iseq)
      integer, intent(out) :: py, pm, pd, ph, pmn, ps, iseq

      call nwpc_systime(py, pm, pd, ph, pmn, ps, iseq)
    end subroutine nwp_systime

    subroutine nwp_filestat(fname, iseq)
      character(*), intent(in) :: fname
      integer, intent(out) :: iseq

      call nwpc_filestat(fname, iseq)
    end subroutine nwp_filestat

    subroutine nwp_lustre_recover(fname, iseq)
      character(*), intent(in) :: fname
      integer, intent(out) :: iseq

      call nwpc_lustre_recover(fname, iseq)
    end subroutine nwp_lustre_recover

    subroutine nwp_lustre_close(dev, iseq)
      character(4096) :: fname
      integer, intent(out) :: iseq
      integer, intent(in) :: dev
      logical :: has_name
      character(8) :: w
      w = ""

      if (0 /= dev .and. 5 /= dev .and. 6 /= dev) then
        inquire(dev, named=has_name, write=w, name=fname)
      end if
      close(dev)
      if (has_name .and. "YES" == w) then
        call nwp_lustre_recover(fname, iseq)
      else
        iseq = 2
      end if
    end subroutine nwp_lustre_close

    subroutine nwp_lustre_simple_close(dev)
      integer :: iseq
      integer, intent(in) :: dev
      
      call nwp_lustre_close(dev, iseq)
      if (4 == iseq) stop 149
    end subroutine nwp_lustre_simple_close

end module nwpl_time
dnl nwpl_time.m4
