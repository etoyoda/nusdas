  type srf_amd_sinfo
    integer            :: SNUM, TYPE    ! �n�_�ԍ��A�n�_���
    real               :: LAT, LON      ! �ܓx�A�o�x
    real               :: HH, WH        ! �n�_�W���A�����v�̍���
    character (len=10) :: NAME1         ! �n�_���J�^�J�i
    character (len=14) :: NAME2         ! �n�_������
  end type srf_amd_sinfo
!
  integer, parameter :: SRF_KANS= 0
  integer, parameter :: SRF_ELM4= 1
  integer, parameter :: SRF_AMEL= 2
  integer, parameter :: SRF_AMEN= 3
  integer, parameter :: SRF_AIRP= 4
  integer, parameter :: SRF_YUKI= 5
  integer, parameter :: SRF_ALL =10
