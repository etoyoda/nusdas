      integer, parameter:: N_ON = 1
      integer, parameter:: N_OFF = 0

      integer, parameter:: N_FOPEN_NOT = 0
      integer, parameter:: N_FOPEN_READ = 1
      integer, parameter:: N_FOPEN_WRITE = 2
      integer, parameter:: N_FOPEN_ALL = 3

      integer, parameter:: N_PC_MISSING_UI1 = 1
      integer, parameter:: N_PC_MISSING_SI2 = 2
      integer, parameter:: N_PC_MISSING_SI4 = 3
      integer, parameter:: N_PC_MISSING_R4 = 4
      integer, parameter:: N_PC_MISSING_R8 = 5
      integer, parameter:: N_PC_MASK_BIT = 6
      integer, parameter:: N_PC_ID_SET = 7
      integer, parameter:: N_PC_PACKING = 8
      integer, parameter:: N_PC_SIZEX = 9
      integer, parameter:: N_PC_SIZEY = 10
!     integer, parameter:: N_PC_PANDORA = 11
!     integer, parameter:: N_PC_PANDORA_I1 = 12
      integer, parameter:: N_PC_OPTIONS = 13
      integer, parameter:: N_PC_WBUFFER = 14
      integer, parameter:: N_PC_RBUFFER = 15
      integer, parameter:: N_PC_KEEP_CFILE = 16

      integer, parameter:: N_IO_MARK_END = 1
      integer, parameter:: N_IO_W_FCLOSE = 3
      integer, parameter:: N_IO_WARNING_OUT = 4
      integer, parameter:: N_IO_R_FCLOSE = 8
      integer, parameter:: N_IO_BADGRID = 16

      character(len=3), parameter:: N_IO_PUT = 'put'
      character(len=3), parameter:: N_IO_GET = 'get'
      integer, parameter:: UCHAR_MAX = 255
      integer, parameter:: SHRT_MIN = (-32768)
      integer, parameter:: LONG_MIN = (-2147483647-1)
      real, parameter:: FLT_MAX = HUGE(0.0)
      double precision, parameter:: DBL_MAX = HUGE(0.0d0)
      integer, parameter:: N_MV_UI1 = 255
      integer, parameter:: N_MV_SI2 = (-32768)
      integer, parameter:: N_MV_SI4 = (-2147483647-1)
      real, parameter:: N_MV_R4 = HUGE(0.0)
      double precision, parameter:: N_MV_R8 = 1.7976931348623157d+308

      character(len=2), parameter:: N_I1 = 'I1'
      character(len=2), parameter:: N_I2 = 'I2'
      character(len=2), parameter:: N_I4 = 'I4'
      character(len=2), parameter:: N_R4 = 'R4'
      character(len=2), parameter:: N_R8 = 'R8'
      character(len=2), parameter:: N_NC = 'NC'
      character(len=2), parameter:: N_ND = 'ND'
      character(len=4), parameter:: N_P_1PAC = '1PAC'
      character(len=4), parameter:: N_P_2PAC = '2PAC'
      character(len=4), parameter:: N_P_2UPC = '2UPC'
      character(len=4), parameter:: N_P_4PAC = '4PAC'
      character(len=4), parameter:: N_P_R4 = 'R4  '
      character(len=4), parameter:: N_P_R8 = 'R8  '
      character(len=4), parameter:: N_P_I1 = 'I1  '
      character(len=4), parameter:: N_P_I2 = 'I2  '
      character(len=4), parameter:: N_P_I4 = 'I4  '
      character(len=4), parameter:: N_P_N1I2 = 'N1I2'
!     character(len=4), parameter:: N_P_GRIB = 'GRIB'
      character(len=4), parameter:: N_P_RLEN = 'RLEN'

      integer, parameter:: N_MEMBER_NUM = 0
      integer, parameter:: N_MEMBER_LIST = 1
      integer, parameter:: N_VALIDTIME_NUM = 2
      integer, parameter:: N_VALIDTIME_LIST = 3
      integer, parameter:: N_PLANE_NUM = 4
      integer, parameter:: N_PLANE_LIST = 5
      integer, parameter:: N_ELEMENT_NUM = 6
      integer, parameter:: N_ELEMENT_LIST = 7
      integer, parameter:: N_ELEMENT_MAP = 8
      integer, parameter:: N_GRID_SIZE = 9
      integer, parameter:: N_GRID_DISTANCE = 10
      integer, parameter:: N_GRID_BASEPOINT = 11
      integer, parameter:: N_VALIDTIME_UNIT = 12
      integer, parameter:: N_VALIDTIME2_UNIT = 13
      integer, parameter:: N_PLANE2_LIST = 14
      integer, parameter:: N_BASETIME_NUM = 15
      integer, parameter:: N_BASETIME_LIST = 16
      integer, parameter:: N_MISSING_MODE = 17
      integer, parameter:: N_MISSING_VALUE = 18
      integer, parameter:: N_PROJECTION = 19
      integer, parameter:: N_STAND_LATLON = 20
      integer, parameter:: N_SPARE_LATLON = 21
      integer, parameter:: N_DATA_NBYTES = 22
      integer, parameter:: N_DATA_CONTENT = 23
      integer, parameter:: N_DATA_QUADRUPLET = 24
      integer, parameter:: N_SUBC_NUM = 25
      integer, parameter:: N_SUBC_LIST = 26
      integer, parameter:: N_SUBC_NBYTES = 27
      integer, parameter:: N_SUBC_CONTENT = 28
      integer, parameter:: N_INFO_NUM = 29
      integer, parameter:: N_INFO_LIST = 30
      integer, parameter:: N_INFO_NBYTES = 31
      integer, parameter:: N_INFO_CONTENT = 32
      integer, parameter :: N_NUSD_NBYTES = 33
      integer, parameter :: N_NUSD_CONTENT = 34
      integer, parameter :: N_CNTL_NBYTES = 35
      integer, parameter :: N_CNTL_CONTENT = 36
      integer, parameter :: N_RECORD_TIME = 37
      integer, parameter :: N_DATA_MAP = 38
      integer, parameter :: N_INDX_SIZE = 39
      integer, parameter :: N_DATA_EXIST = 40

!     integer, parameter:: PDR_LOCAL_ONLY = 0
!     integer, parameter:: PDR_NET_ONLY = 1
!     integer, parameter:: PDR_LOCAL_NET = 2
!     integer, parameter:: PDR_I1_RLEN_U8  =  0
!     integer, parameter:: PDR_I1_RLEN_ONLY  =  1
!     integer, parameter:: PDR_I1_UI8_ONLY  =  2
