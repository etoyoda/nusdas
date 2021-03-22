  type srf_amd_sinfo
    integer            :: SNUM, TYPE    ! 地点番号、地点種別
    real               :: LAT, LON      ! 緯度、経度
    real               :: HH, WH        ! 地点標高、風速計の高さ
    character (len=10) :: NAME1         ! 地点名カタカナ
    character (len=14) :: NAME2         ! 地点名漢字
  end type srf_amd_sinfo
!
  integer, parameter :: SRF_KANS= 0
  integer, parameter :: SRF_ELM4= 1
  integer, parameter :: SRF_AMEL= 2
  integer, parameter :: SRF_AMEN= 3
  integer, parameter :: SRF_AIRP= 4
  integer, parameter :: SRF_YUKI= 5
  integer, parameter :: SRF_ALL =10
