    subroutine nwp_ymd2seq(iy, im, id, iseq)
      use nwpl_time, only : wrap_nwp_ymd2seq => nwp_ymd2seq
      integer, intent(in) :: iy, im, id
      integer, intent(out) :: iseq

      call wrap_nwp_ymd2seq(iy, im, id, iseq)
    end subroutine nwp_ymd2seq

    subroutine nwp_ymdh2seq(iy, im, id, ih, iseq)
      use nwpl_time, only : wrap_nwp_ymdh2seq => nwp_ymdh2seq
      integer, intent(in) :: iy, im, id, ih
      integer, intent(out) :: iseq

      call wrap_nwp_ymdh2seq(iy, im, id, ih, iseq)
    end subroutine nwp_ymdh2seq

    subroutine nwp_ymdhm2seq(iy, im, id, ih, imn, iseq)
      use nwpl_time, only : wrap_nwp_ymdhm2seq => nwp_ymdhm2seq
      integer, intent(in) :: iy, im, id, ih, imn
      integer, intent(out) :: iseq

      call wrap_nwp_ymdhm2seq(iy, im, id, ih, imn, iseq)
    end subroutine nwp_ymdhm2seq

    subroutine nwp_seq2ymd(py, pm, pd, iseq)
      use nwpl_time, only : wrap_nwp_seq2ymd => nwp_seq2ymd
      integer, intent(out) :: py, pm, pd
      integer, intent(in) :: iseq

      call wrap_nwp_seq2ymd(py, pm, pd, iseq)
    end subroutine nwp_seq2ymd

    subroutine nwp_seq2ymdh(py, pm, pd, ph, iseq)
      use nwpl_time, only : wrap_nwp_seq2ymdh => nwp_seq2ymdh
      integer, intent(out) :: py, pm, pd, ph
      integer, intent(in) :: iseq

      call wrap_nwp_seq2ymdh(py, pm, pd, ph, iseq)
    end subroutine nwp_seq2ymdh

    subroutine nwp_seq2ymdhm(py, pm, pd, ph, pmn, iseq)
      use nwpl_time, only : wrap_nwp_seq2ymdhm => nwp_seq2ymdhm
      integer, intent(out) :: py, pm, pd, ph, pmn
      integer, intent(in) :: iseq

      call wrap_nwp_seq2ymdhm(py, pm, pd, ph, pmn, iseq)
    end subroutine nwp_seq2ymdhm

    subroutine nwp_gettime(tcname, py, pm, pd, ph, pmn, iseq)
      use nwpl_time, only : wrap_nwp_gettime => nwp_gettime
      character(*), intent(in) :: tcname
      integer, intent(out) :: py, pm, pd, ph, pmn, iseq

      call wrap_nwp_gettime(tcname, py, pm, pd, ph, pmn, iseq)
    end subroutine nwp_gettime

    subroutine nwp_systime(py, pm, pd, ph, pmn, ps, iseq)
      use nwpl_time, only : wrap_nwp_systime => nwp_systime
      integer, intent(out) :: py, pm, pd, ph, pmn, ps, iseq

      call wrap_nwp_systime(py, pm, pd, ph, pmn, ps, iseq)
    end subroutine nwp_systime

    subroutine nwp_filestat(fname, iseq)
      use nwpl_time, only : wrap_nwp_filestat => nwp_filestat
      character(*), intent(in) :: fname
      integer, intent(out) :: iseq

      call wrap_nwp_filestat(fname, iseq)
    end subroutine nwp_filestat

    subroutine nwp_lustre_recover(fname, iseq)
      use nwpl_time, only : wrap_nwp_lustre_recover => nwp_lustre_recover
      character(*), intent(in) :: fname
      integer, intent(out) :: iseq

      call wrap_nwp_lustre_recover(fname, iseq)
    end subroutine nwp_lustre_recover

    subroutine nwp_lustre_close(dev, iseq)
      use nwpl_time, only : wrap_nwp_lustre_close => nwp_lustre_close
      integer, intent(in) :: dev
      integer, intent(out) :: iseq

      call wrap_nwp_lustre_close(dev, iseq)
    end subroutine nwp_lustre_close

    subroutine nwp_lustre_simple_close(dev)
      use nwpl_time, only : &
      wrap_nwp_lustre_simple_close => nwp_lustre_simple_close
      integer, intent(in) :: dev

      call wrap_nwp_lustre_simple_close(dev)
    end subroutine nwp_lustre_simple_close
