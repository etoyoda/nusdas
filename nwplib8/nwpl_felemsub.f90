! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
!
! ������ ���l�\��W�����C�u���� ������
!
!     �v�f�ϊ�SUBROUTINE�Q                   1996.11.12 K.ONOGI
!     NWPLIB�Ƃ��ē]�p��F90���W���[����      2000.08    Y.Oikawa
!     nwpl_nwpcst.f90 �̒萔����荞��, T2EPT �폜 2005.10 N.Yasutani
!
!---------------------------------------------------------------------
!     TETEN�̎��ŉt���ƌő����l��
!---------------------------------------------------------------------
!  �P�� �C��:(K),�䎼(g/kg),������(g/kg),���Ύ��x(%)
!---------------------------------------------------------------------
!     ���̃����o�Ɋ܂܂��SUBROUTINE
!     PRSVADJ        �C���̍��x�␳
!     RP2DZ DZ2RP    �C���� <---> ���x��
!     TH2TV TV2TH    �w�� <--->�w�ԉ����x
!     DF2UV UV2DF    �����E���� <---> ����U,V����
!     UVLAMB         ����U,V�����̉�](������k<-->�����x���g)
!     TETEN RVTETEN  �C�� ----- �O�a�����C��
!     TV2T  T2TV     �C�� <---> �����x
!     TV2Q  Q2TV     �䎼 <---> �����x
!     RM2Q  Q2RM     �䎼 <---> ������
!     VP2Q  Q2VP     �䎼 <---> �����C��
!     RM2VP VP2RM    ������ <---> �����C��
!     TD2RH RH2TD    ���Ύ��x <---> �I�_���x
!     TD2Q  Q2TD     �䎼 <---> �I�_���x
!     Q2RH  RH2Q     �䎼 <---> ���Ύ��x
!     RM2RH RH2RM    ������ <---> ���Ύ��x
!     VP2RH RH2VP    �����C�� <---> ���Ύ��x
!     TV2RH RH2TV    �����x <---> ���Ύ��x
!     TV2TD TD2TV    �����x <---> �I�_���x
!     T2PT  PT2T     �C�� <---> ����
!�폜!T2EPT          �C��+�䎼 --> ��������

! -----------------------------------------------------------------------------
!   �T�u���[�`���C���^�[�t�F�[�X��`
!   ��̃T�u���[�`�����̖��ŁA�P�E�{�����x�̃T�u���[�`�����Ăяo��
! -----------------------------------------------------------------------------
module nwpl_felemsub


  real(8), private, parameter :: pi = 3.14159265358979d0   ! �~������ 

  real(8), private, parameter :: grv = 9.80665d0  ! �d�͉����x(m/s)
  real(8), private, parameter :: rd = 287.05d0   ! ������C�̋C�̒萔(J/(kg*K))
  real(8), private, parameter :: cpd = 1005.d0   ! ������C�̒舳��M(J/(kg*K))
  real(8), private, parameter :: lv = 2501.0d+03 ! �Ì��̐��M (J/kg)


  interface nwp_prsvadj
    module procedure nwps_prsvadj,nwpd_prsvadj
  end interface

  interface nwp_rp2dz
    module procedure nwps_rp2dz,nwpd_rp2dz
  end interface

  interface nwp_dz2rp
    module procedure nwps_dz2rp,nwpd_dz2rp
  end interface

  interface nwp_th2tv
    module procedure nwps_th2tv,nwpd_th2tv
  end interface

  interface nwp_tv2th
    module procedure nwps_tv2th,nwpd_tv2th
  end interface

  interface nwp_df2uv
    module procedure nwps_df2uv,nwpd_df2uv
  end interface

  interface nwp_uv2df
    module procedure nwps_uv2df,nwpd_uv2df
  end interface

  interface nwp_uvlamb
    module procedure nwps_uvlamb,nwpd_uvlamb
  end interface

  interface nwp_teten
    module procedure nwps_teten,nwpd_teten
  end interface

  interface nwp_rvteten
    module procedure nwps_rvteten,nwpd_rvteten
  end interface

  interface nwp_tv2t
    module procedure nwps_tv2t,nwpd_tv2t
  end interface

  interface nwp_t2tv
    module procedure nwps_t2tv,nwpd_t2tv
  end interface

  interface nwp_tv2q
    module procedure nwps_tv2q,nwpd_tv2q
  end interface

  interface nwp_q2tv
    module procedure nwps_q2tv,nwpd_q2tv
  end interface

  interface nwp_rm2q
    module procedure nwps_rm2q,nwpd_rm2q
  end interface

  interface nwp_q2rm
    module procedure nwps_q2rm,nwpd_q2rm
  end interface

  interface nwp_vp2q
    module procedure nwps_vp2q,nwpd_vp2q
  end interface

  interface nwp_q2vp
    module procedure nwps_q2vp,nwpd_q2vp
  end interface

  interface nwp_rm2vp
    module procedure nwps_rm2vp,nwpd_rm2vp
  end interface

  interface nwp_vp2rm
    module procedure nwps_vp2rm,nwpd_vp2rm
  end interface

  interface nwp_td2rh
    module procedure nwps_td2rh,nwpd_td2rh
  end interface

  interface nwp_rh2td
    module procedure nwps_rh2td,nwpd_rh2td
  end interface

  interface nwp_td2q
    module procedure nwps_td2q,nwpd_td2q
  end interface

  interface nwp_q2td
    module procedure nwps_q2td,nwpd_q2td
  end interface

  interface nwp_q2rh
    module procedure nwps_q2rh,nwpd_q2rh
  end interface

  interface nwp_rh2q
    module procedure nwps_rh2q,nwpd_rh2q
  end interface

  interface nwp_rm2rh
    module procedure nwps_rm2rh,nwpd_rm2rh
  end interface

  interface nwp_rh2rm
    module procedure nwps_rh2rm,nwpd_rh2rm
  end interface

  interface nwp_vp2rh
    module procedure nwps_vp2rh,nwpd_vp2rh
  end interface

  interface nwp_rh2vp
    module procedure nwps_rh2vp,nwpd_rh2vp
  end interface

  interface nwp_tv2rh
    module procedure nwps_tv2rh,nwpd_tv2rh
  end interface

  interface nwp_rh2tv
    module procedure nwps_rh2tv,nwpd_rh2tv
  end interface

  interface nwp_td2tv
    module procedure nwps_td2tv,nwpd_td2tv
  end interface

  interface nwp_tv2td
    module procedure nwps_tv2td,nwpd_tv2td
  end interface

  interface nwp_t2pt
    module procedure nwps_t2pt,nwpd_t2pt
  end interface

  interface nwp_pt2t
    module procedure nwps_pt2t,nwpd_pt2t
  end interface

! interface nwp_t2ept
!   module procedure nwps_t2ept,nwpd_t2ept
! end interface

! =============================================================================
!   �T�u���[�`���{�̒�`
! =============================================================================

  contains

!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �C���̍��x�␳(hPa)
!     ��` (��ʍ��x��0,�W������Z�Ƃ���)
!     P = P0 * EXP(-g/Rd*��dz/Tv)
!     GAM  ; �C������ (= (Tsea-Tsrf)/z)   (K/m)
!     TV   ; �����x
!     Z    ; �W����(m)  (�o�̓��x���̕W�� - ���̓��x���̕W��)
!            �C�ʍX���̂Ƃ�  Z < 0
!            �t�C�ʍX���̂Ƃ�  Z > 0
!       (ex) �W��100m�̒n�_�̌��n�C������
!            �C�ʍX���C�������߂�Ƃ��� Z=-100

      SUBROUTINE NWPS_PRSVADJ (POUT, PIN,TV,Z,GAM,n)
      integer n
      real,dimension(n)::pout,pin,tv,z,gam
      do i=1,n
        POUT(i) = PIN(i)*((-GAM(i)*Z(i)+TV(i))/TV(i))**(GRV/(GAM(i)*RD))
      end do
      RETURN
      end subroutine nwps_prsvadj

      SUBROUTINE NWPD_PRSVADJ (POUT, PIN,TV,Z,GAM,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::pout,pin,tv,z,gam
      do i=1,n
        POUT(i) = PIN(i)*((-GAM(i)*Z(i)+TV(i))/TV(i))**(GRV/(GAM(i)*RD))
      end do
      RETURN
      end subroutine nwpd_prsvadj
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �C����ɑΉ����鍂�x�������߂�
!     GAM  ; �C������ (= (Tsea-Tsrf)/z)   (K/m)
!     TV   ; �����x
!     RP   ; �C����(���ł͂Ȃ�!)
!     DZ   ; ���x��(m)
!             (RP��1 �� DZ��0)
      SUBROUTINE NWPS_RP2DZ (DZ, RP,TV,GAM,n)
      integer n
      real,dimension(n)::dz,rp,tv,gam
      
      do i=1,n
        IF (NINT(GAM(i)*1000).EQ.0) THEN
          DZ(i) = RD * TV(i) / GRV * LOG(RP(i))
        ELSE
          DZ(i) = TV(i) / GAM(i) * ( 1. - RP(i)**(-RD*GAM(i)/GRV) )
        END IF
      end do
      RETURN
      end subroutine nwps_rp2dz

      SUBROUTINE NWPD_RP2DZ (DZ, RP,TV,GAM,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::dz,rp,tv,gam
      do i=1,n
        IF (NINT(GAM(i)*1000).EQ.0) THEN
          DZ(i) = RD * TV(i) / GRV * LOG(RP(i))
        ELSE
          DZ(i) = TV(i) / GAM(i) * ( 1. - RP(i)**(-RD*GAM(i)/GRV) )
        END IF
      end do
      RETURN
      end subroutine nwpd_rp2dz
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- ���x���ɑΉ�����C��������߂�
!     GAM  ; �C������ (= (Tsea-Tsrf)/z)   (K/m)
!     TV   ; �����x
!     DZ   ; ���x��(m)
!     RP   ; �C����(���ł͂Ȃ�!)
!             (DZ��0 �� RP��1)
      SUBROUTINE NWPS_DZ2RP (RP, DZ,TV,GAM,n)
      integer n
      real,dimension(n)::rp,dz,tv,gam
      do i=1,n
        IF (NINT(GAM(i)*1000).EQ.0) THEN
          RP(i) = EXP(GRV*DZ(i)/(RD*TV(i)))
        ELSE
          RP(i) = (1 - GAM(i)*DZ(i)/TV(i))**(-GRV/(RD*GAM(i)))
        END IF
      end do
      RETURN
      end subroutine nwps_dz2rp

      SUBROUTINE NWPD_DZ2RP (RP, DZ,TV,GAM,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::rp,dz,tv,gam
      do i=1,n
        IF (NINT(GAM(i)*1000).EQ.0) THEN
          RP(i) = EXP(GRV*DZ(i)/(RD*TV(i)))
        ELSE
          RP(i) = (1 - GAM(i)*DZ(i)/TV(i))**(-GRV/(RD*GAM(i)))
        END IF
      end do
      RETURN
      end subroutine nwpd_dz2rp
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �w���ɑΉ�����w�ԉ����x�����߂�
!     PL ; ���[�̋C��
!     PH ; ��[�̋C��
!     TH ; �w��
!     TV ; �w�ԉ����x
      SUBROUTINE NWPS_TH2TV (TV, TH,PL,PH,n)
      integer n
      real,dimension(n)::tv,th,pl,ph
      do i=1,n
        IF (PH(i).GT.0.) THEN
          PLG = LOG(PL(i)/PH(i))
          IF (TH(i).GT.0 .AND. PLG.GT.0.) THEN
            TV(i) = GRV/RD*TH(i)/PLG
          END IF
        END IF
      end do
      RETURN
      end subroutine nwps_th2tv

      SUBROUTINE NWPD_TH2TV (TV, TH,PL,PH,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::tv,th,pl,ph
      do i=1,n
        IF (PH(i).GT.0.) THEN
          PLG = LOG(PL(i)/PH(i))
          IF (TH(i).GT.0 .AND. PLG.GT.0.) THEN
            TV(i) = GRV/RD*TH(i)/PLG
          END IF
        END IF
      end do
      RETURN
      end subroutine nwpd_th2tv
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �w�ԉ����x�ɑΉ�����w�������߂�
!     PL ; ���[�̋C��
!     PH ; ��[�̋C��
!     TV ; �w�ԉ����x
!     TH ; �w��
      SUBROUTINE NWPS_TV2TH (TH, TV,PL,PH,n)
      integer n
      real,dimension(n)::th,tv,pl,ph

      do i=1,n
        IF (PH(i).GT.0.) THEN
          PLG = LOG(PL(i)/PH(i))
          TH(i) = RD/GRV*TV(i)*PLG
        END IF
      end do
      RETURN
      end subroutine nwps_tv2th

      SUBROUTINE NWPD_TV2TH (TH, TV,PL,PH,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::th,tv,pl,ph

      do i=1,n
        IF (PH(i).GT.0.) THEN
          PLG = LOG(PL(i)/PH(i))
          TH(i) = RD/GRV*TV(i)*PLG
        END IF
      end do
      RETURN
      end subroutine nwpd_tv2th
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!     ���֌W
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- ����(�x)�E����(m/s) --> �t�E�u(������k) (m/s)
      SUBROUTINE NWPS_DF2UV (U,V, D,F,n)
      integer n
      real,dimension(n)::u,v,d,f

      do i=1,n
        TH = 90.-D(i)
        U(i)  = -F(i)*COS(TH*PI/180.)
        V(i)  = -F(i)*SIN(TH*PI/180.)
      end do
      RETURN
      end subroutine nwps_df2uv

      SUBROUTINE NWPD_DF2UV (U,V, D,F,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::u,v,d,f

      do i=1,n
        TH = 90.-D(i)
        U(i)  = -F(i)*COS(TH*PI/180.)
        V(i)  = -F(i)*SIN(TH*PI/180.)
      end do
      RETURN
      end subroutine nwpd_df2uv
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �t�E�u(������k) (m/s) --> ����(�x)�E����(m/s)
      SUBROUTINE NWPS_UV2DF (D,F, U,V,n)
      integer n
      real,dimension(n)::d,f,u,v
 
      do i=1,n

        F(i)  = SQRT(U(i)**2+V(i)**2)
        IF (F(i).EQ.0.) THEN
          D(i) = 0.
          cycle
        END IF

        ANG = ATAN2(V(i), U(i))
!        IF (V(i) >= 0) THEN
!          IF (U(i) >= 0) THEN
!            ANG = ACOS(U(i)/F(i))  ! �쐼�� "
!          ELSE
!            ANG = ACOS(U(i)/F(i))  ! �k���� "
!          ENDIF
!        ELSE
!          IF (U(i) < 0) THEN
!            ANG =-ACOS(U(i)/F(i))  ! �k���� "
!          ELSE
!            ANG = ASIN(V(i)/F(i))  ! �쓌�� "
!          ENDIF
!        ENDIF

        D(i) = 270.-ANG/PI*180.
        IF (D(i).GT.360.) D(i) = D(i)-360.
        IF (D(i).LT.  0.) D(i) = D(i)+360.
  
      end do

      RETURN
      end subroutine nwps_uv2df

      SUBROUTINE NWPD_UV2DF (D,F, U,V,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::d,f,u,v
 
!
      do i=1,n

        F(i)  = SQRT(U(i)**2+V(i)**2)
        IF (F(i).EQ.0.) THEN
          D(i) = 0.
          cycle
        END IF

        ANG = ATAN2(V(i), U(i))
!        IF (V(i) >= 0) THEN
!          IF (U(i) >= 0) THEN
!            ANG = ACOS(U(i)/F(i))  ! �쐼�� "
!          ELSE
!            ANG = ACOS(U(i)/F(i))  ! �k���� "
!          ENDIF
!        ELSE
!          IF (U(i) < 0) THEN
!            ANG =-ACOS(U(i)/F(i))  ! �k���� "
!          ELSE
!            ANG = ASIN(V(i)/F(i))  ! �쓌�� "
!          ENDIF
!        ENDIF

        D(i) = 270.-ANG/PI*180.
        IF (D(i).GT.360.) D(i) = D(i)-360.
        IF (D(i).LT.  0.) D(i) = D(i)+360.
  
      end do

      RETURN
      end subroutine nwpd_uv2df
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!---------------------------------------------------------------------
!     ����U,V�����̕ϊ�
!     ������k ---> LAMBERT���W��X,Y����    IDX =  1
!     ������k <--- LAMBERT���W��X,Y����    IDX = -1
!     LAMBERT���W�̊�ܓx   SLAT= 30 N , 60 N
!---------------------------------------------------------------------
      SUBROUTINE NWPS_UVLAMB (U,V, OLON,SLON,IDX,n)
      integer n
      real,dimension(n)::u,v,olon
      real slon

      RAD = ASIN(1.0)/90.0

      do i=1,n

        ANGR =  IDX*(OLON(i)-SLON)*RAD / SQRT(2.)

        U1 = U(i)*COS(ANGR) - V(i)*SIN(ANGR)
        V1 = U(i)*SIN(ANGR) + V(i)*COS(ANGR)
        U(i)  = U1
        V(i)  = V1

      end do

      RETURN
      end subroutine nwps_uvlamb

      SUBROUTINE NWPD_UVLAMB (U,V, OLON,SLON,IDX,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::u,v,olon
      double precision slon

      RAD = DASIN(1.D0)/90.D0

      do i=1,n

        ANGR =  IDX*(OLON(i)-SLON)*RAD / SQRT(2.)

        U1 = U(i)*COS(ANGR) - V(i)*SIN(ANGR)
        V1 = U(i)*SIN(ANGR) + V(i)*COS(ANGR)
        U(i)  = U1
        V(i)  = V1

      end do

      RETURN
      end subroutine nwpd_uvlamb
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
!     �����C�֌W�̊�{�I�Ȏ�
!     �����C�֌W�̗v�f�ϊ��͈ȉ��̎� or ���̑g�����ŕ\���ł���
!     �i���̕����̒P�ʂ�(kg/kg)�ŕ\���j
!     E = 6.11*10**((7.5*(T-273.15))/(237.3+(T-273.15))) TETEN�̎�(��)
!    (E = 6.11*EXP((17.3*(T-273.15))/(237.3+(T-273.15))) TETEN�̎�(��))
!     E = 6.11*10**((9.5*(T-273.15))/(265.3+(T-273.15))) TETEN�̎�(�X)
!    (E = 6.11*EXP((21.9*(T-273.15))/(265.3+(T-273.15))) TETEN�̎�(�X))
!     TV = T*(1+0.608*Q)
!     RM = 0.622*E/(P-E)
!     Q  = RM/(1+RM)
!     E  = Q*P/(0.378*Q+0.622)
!     RH = E/ES
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �C��(K)�ɑΉ�����O�a�����C��(hPa)�����߂�
!     TETEN�̎����g�� (�X���l���j                1996.11.12 K.ONOGI
!     -40���ȉ�               �i�X�j
!     -40�����傫�� 0������ �i�X�Ɛ����ĕ��j
!       0���ȏ�               �i���j

!     �X�ւ̈ڍs��-40������-15���ɕύX           1999.07.06 H.TADA
!     -15���ȉ�               �i�X�j
!     -15�����傫�� 0������ �i�X�Ɛ����ĕ��j
!       0���ȏ�               �i���j

      SUBROUTINE NWPS_TETEN (E, T,n)
      integer n
      real,dimension(n)::e,t
      DATA T0    /273.15/
      DATA E0C   /  6.11/
      DATA AL,BL / 17.3, 237.3/
      DATA AI,BI / 21.9, 265.3/

      do i=1,n
        TC = T(i)-T0
                                                      ! -40����:
        IF (TC.GE.0.) THEN               
          E(i) = E0C * EXP(AL*TC/(BL+TC))
        ELSE IF (TC.LE.-15.) THEN                     ! (TC.LE.-40.)
          E(i) = E0C * EXP(AI*TC/(BI+TC))
        ELSE
          E(i) = E0C *(EXP(AL*TC/(BL+TC))*(15.+TC)/15. +  & ! *(40+TC)/40.
                 EXP(AI*TC/(BI+TC))*(-TC)/15.)              ! *(-TC)/40.
        END IF
      end do
      RETURN
      end subroutine nwps_teten

      SUBROUTINE NWPD_TETEN (E, T,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::e,t
      DATA T0    /273.15/
      DATA E0C   /  6.11/
      DATA AL,BL / 17.3, 237.3/
      DATA AI,BI / 21.9, 265.3/

      do i=1,n
        TC = T(i)-T0
                                                      ! -40����:
        IF (TC.GE.0.) THEN               
          E(i) = E0C * EXP(AL*TC/(BL+TC))
        ELSE IF (TC.LE.-15.) THEN                     ! (TC.LE.-40.)
          E(i) = E0C * EXP(AI*TC/(BI+TC))
        ELSE
          E(i) = E0C *(EXP(AL*TC/(BL+TC))*(15.+TC)/15. +  & ! *(40+TC)/40.
                 EXP(AI*TC/(BI+TC))*(-TC)/15.)              ! *(-TC)/40.
        END IF
      end do
      RETURN
      end subroutine nwpd_teten
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �O�a�����C��(hPa)�ɑΉ�����C��(�I�_���x)(K)�����߂�
!     TETEN�̎����g�� (�X���l���j                1996.11.12 K.ONOGI
!     -40���ȉ�               �i�X�j
!     -40�����傫�� 0������ �i�X�Ɛ����ĕ��j
!       0���ȏ�               �i���j

!     �X�ւ̈ڍs��-40������-15���ɕύX           1999.07.06 H.TADA
!     -15���ȉ�               �i�X�j
!     -15�����傫�� 0������ �i�X�Ɛ����ĕ��j
!       0���ȏ�               �i���j
      SUBROUTINE NWPS_RVTETEN(T, E,n)
      implicit none
      integer n
      real,dimension(n)::t,e

!40   integer,parameter :: NXTC=401  ! -40����
      integer,parameter :: NXTC=151  ! -15����

      REAL(4),save :: EE(NXTC), TT(NXTC)
      REAL(8),parameter ::  T0 = 273.15
      REAL(8),parameter ::  E0C =  6.11
      REAL(8),parameter ::  AL  =  17.3 , BL = 237.3
      REAL(8),parameter ::  AI  =  21.9 , BI = 265.3
      REAL(8),save ::  EMC
      LOGICAL,save ::  LX = .TRUE.
      integer :: I, J
      REAL(8) ::  EL, EI, CC

      IF (LX) THEN
                                                      ! -40����:
!       -15���̖O�a�����C���i�X�j                     ! -40��
        EMC = E0C*EXP(AI*(-15.)/(BI-15.))             ! (-40.)/(BI-40.)
                                                      !
!   --- -15���`0���ɂ��Ă�0.1�x���݂̃e�[�u�����쐬 ! -40���`
        DO 10 I = 1, NXTC                             !
          TT(I) = -0.1*(I-1)                          !
          EL = E0C * EXP(AL*TT(I)/(BL+TT(I)))         !
          EI = E0C * EXP(AI*TT(I)/(BI+TT(I)))         !
          EE(I) = ( EL*(15.0+TT(I)) - EI*TT(I) )/15.0 ! (40.0+TT(I))
   10   CONTINUE                                      ! /40.0

        LX = .FALSE.

      END IF

      do j=1,n

      IF (E(j).LE.EMC) THEN
        CC = LOG(E(j)/E0C)
        T(j) = CC*BI/(AI-CC) + T0     ! �X
      ELSE IF (E(j).GE.E0C) THEN
        CC = LOG(E(j)/E0C)
        T(j) = CC*BL/(AL-CC) + T0     ! ��
      ELSE
        DO 100 I=2,NXTC
          IF (EE(I).LT.E(j)) THEN
            T(j) = TT(I-1)-(TT(I-1)-TT(I))*(EE(I-1)-E(j))/(EE(I-1)-EE(I))
            GOTO 110
          END IF
  100   CONTINUE
  110   CONTINUE
        T(j) = T(j)+T0
      END IF

      end do

      RETURN
      end subroutine nwps_rvteten

      SUBROUTINE NWPD_RVTETEN(T, E,n)
      implicit none
      integer n
      double precision,dimension(n)::t,e

!40   integer,parameter :: NXTC=401  ! -40����
      integer,parameter :: NXTC=151  ! -15����

      REAL(8),save :: EE(NXTC), TT(NXTC)
      REAL(8),parameter ::  T0 = 273.15
      REAL(8),parameter ::  E0C =  6.11
      REAL(8),parameter ::  AL  =  17.3 , BL = 237.3
      REAL(8),parameter ::  AI  =  21.9 , BI = 265.3
      REAL(8),save ::  EMC
      LOGICAL,save ::  LX = .TRUE.
      integer :: I, J
      REAL(8) ::  EL, EI, CC
      
      IF (LX) THEN
                                                      ! -40����:
!       -15���̖O�a�����C���i�X�j                     ! -40��
        EMC = E0C*EXP(AI*(-15.)/(BI-15.))             ! (-40.)/(BI-40.)
                                                      !
!   --- -15���`0���ɂ��Ă�0.1�x���݂̃e�[�u�����쐬 ! -40���`
        DO 10 I = 1, NXTC                             !
          TT(I) = -0.1*(I-1)                          !
          EL = E0C * EXP(AL*TT(I)/(BL+TT(I)))         !
          EI = E0C * EXP(AI*TT(I)/(BI+TT(I)))         !
          EE(I) = ( EL*(15.0+TT(I)) - EI*TT(I) )/15.0 ! (40.0+TT(I))
   10   CONTINUE                                      ! /40.0

        LX = .FALSE.

      END IF

      do j=1,n

      IF (E(j).LE.EMC) THEN
        CC = LOG(E(j)/E0C)
        T(j) = CC*BI/(AI-CC) + T0     ! �X
      ELSE IF (E(j).GE.E0C) THEN
        CC = LOG(E(j)/E0C)
        T(j) = CC*BL/(AL-CC) + T0     ! ��
      ELSE
        DO 100 I=2,NXTC
          IF (EE(I).LT.E(j)) THEN
            T(j) = TT(I-1)-(TT(I-1)-TT(I))*(EE(I-1)-E(j))/(EE(I-1)-EE(I))
            GOTO 110
          END IF
  100   CONTINUE
  110   CONTINUE
        T(j) = T(j)+T0
      END IF

      end do

      RETURN
      end subroutine nwpd_rvteten
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �����x(K) --> �C��(K)
      SUBROUTINE NWPS_TV2T (T, TV,Q,n)
      integer n
      real,dimension(n)::t,tv,q
      T = TV/(1.+0.608*Q*0.001)
      RETURN
      end subroutine nwps_tv2t

      SUBROUTINE NWPD_TV2T (T, TV,Q,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::t,tv,q
      T = TV/(1.+0.608*Q*0.001)
      RETURN
      end subroutine nwpd_tv2t
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �C��(K) --> �����x(K)
      SUBROUTINE NWPS_T2TV(TV,T,Q,n)
      integer n
      real,dimension(n)::tv,t,q
      TV = T*(1.+0.608*Q*0.001)
      RETURN
      end subroutine nwps_t2tv

      SUBROUTINE NWPD_T2TV(TV,T,Q,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::tv,t,q
      TV = T*(1.+0.608*Q*0.001)
      RETURN
      end subroutine nwpd_t2tv
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �����x(K) --> �䎼(g/kg)
      SUBROUTINE NWPS_TV2Q (Q, TV,T,n)
      integer n
      real,dimension(n)::q,tv,t
      Q = (TV/T-1.)/0.608 * 1000.
      RETURN
      end subroutine nwps_tv2q

      SUBROUTINE NWPD_TV2Q (Q, TV,T,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::q,tv,t
      Q = (TV/T-1.)/0.608 * 1000.
      RETURN
      end subroutine nwpd_tv2q
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �䎼(g/kg) --> �����x(K)
      SUBROUTINE NWPS_Q2TV (TV, Q,T,n)
      integer n
      real,dimension(n)::tv,q,t
      TV = T*(1.+0.608*Q*0.001)
      RETURN
      end subroutine nwps_q2tv

      SUBROUTINE NWPD_Q2TV (TV, Q,T,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::tv,q,t
      TV = T*(1.+0.608*Q*0.001)
      RETURN
      end subroutine nwpd_q2tv
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- ������(g/kg) --> �䎼(g/kg)
      SUBROUTINE NWPS_RM2Q (Q, RM,n)
      integer n
      real,dimension(n)::q,rm
      Q = RM/(1000.+RM) * 1000.
      RETURN
      end subroutine nwps_rm2q

      SUBROUTINE NWPD_RM2Q (Q, RM,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::q,rm
      Q = RM/(1000.+RM) * 1000.
      RETURN
      end subroutine nwpd_rm2q
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �䎼(g/kg) --> ������(g/kg)
      SUBROUTINE NWPS_Q2RM (RM, Q,n)
      integer n
      real,dimension(n)::rm,q
      RM = Q/(1000.-Q) * 1000.
      RETURN
      end subroutine nwps_q2rm

      SUBROUTINE NWPD_Q2RM (RM, Q,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::rm,q
      RM = Q/(1000.-Q) * 1000.
      RETURN
      end subroutine nwpd_q2rm
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �����C��(hPa) --> �䎼(g/kg)
      SUBROUTINE NWPS_VP2Q (Q, E,P,n)
      integer n
      real,dimension(n)::q,e,p
      Q = 0.622*E/(P-0.378*E) * 1000.
      RETURN
      end subroutine nwps_vp2q

      SUBROUTINE NWPD_VP2Q (Q, E,P,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::q,e,p
      Q = 0.622*E/(P-0.378*E) * 1000.
      RETURN
      end subroutine nwpd_vp2q
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �䎼(g/kg) --> �����C��(hPa)
      SUBROUTINE NWPS_Q2VP (E, Q,P,n)
      integer n
      real,dimension(n)::e,q,p
      E = Q*0.001*P/(0.378*Q*0.001+0.622)
      RETURN
      end subroutine nwps_q2vp

      SUBROUTINE NWPD_Q2VP (E, Q,P,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::e,q,p
      E = Q*0.001*P/(0.378*Q*0.001+0.622)
      RETURN
      end subroutine nwpd_q2vp
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- ������(g/kg) --> �����C��(hPa)
      SUBROUTINE NWPS_RM2VP (E, RM,P,n)
      integer n
      real,dimension(n)::e,rm,p
      E = RM*0.001*P/(0.622+RM*0.001)
      RETURN
      end subroutine nwps_rm2vp

      SUBROUTINE NWPD_RM2VP (E, RM,P,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::e,rm,p
      E = RM*0.001*P/(0.622+RM*0.001)
      RETURN
      end subroutine nwpd_rm2vp
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �����C��(hPa) --> ������(g/kg)
      SUBROUTINE NWPS_VP2RM (RM, E,P,n)
      integer n
      real,dimension(n)::rm,e,p
      RM = 0.622*E/(P-E) * 1000.
      RETURN
      end subroutine nwps_vp2rm

      SUBROUTINE NWPD_VP2RM (RM, E,P,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::rm,e,p
      RM = 0.622*E/(P-E) * 1000.
      RETURN
      end subroutine nwpd_vp2rm
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �I�_���x(K) --> ���Ύ��x(%)
      SUBROUTINE NWPS_TD2RH (RH, TD,T,n)
      integer n
      real,dimension(n)::rh,td,t,e,es

      CALL NWP_TETEN (E,TD,n)
      CALL NWP_TETEN (ES,T,n)
      RH = E/ES*100.

      RETURN
      end subroutine nwps_td2rh

      SUBROUTINE NWPD_TD2RH (RH, TD,T,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::rh,td,t,e,es

      CALL NWP_TETEN (E,TD,n)
      CALL NWP_TETEN (ES,T,n)
      RH = E/ES*100.

      RETURN
      end subroutine nwpd_td2rh
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- ���Ύ��x(%) --> �I�_���x(K)
      SUBROUTINE NWPS_RH2TD (TD, RH,T,n)
      integer n
      real,dimension(n)::td,rh,t,es,e

      CALL NWP_TETEN (ES,T,n)
      E = RH/100.*ES
      CALL NWP_RVTETEN (TD,E,n)

      RETURN
      end subroutine nwps_rh2td

      SUBROUTINE NWPD_RH2TD (TD, RH,T,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::td,rh,t,es,e

      CALL NWP_TETEN (ES,T,n)
      E = RH/100.*ES
      CALL NWP_RVTETEN (TD,E,n)

      RETURN
      end subroutine nwpd_rh2td
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �I�_���x(K) --> �䎼(g/kg)
      SUBROUTINE NWPS_TD2Q (Q, TD,P,n)
      integer n
      real,dimension(n)::q,td,p,e

      CALL NWP_TETEN (E, TD,n)
      CALL NWP_VP2Q (Q, E,P,n)

      RETURN
      end subroutine nwps_td2q

      SUBROUTINE NWPD_TD2Q (Q, TD,P,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::q,td,p,e

      CALL NWP_TETEN (E, TD,n)
      CALL NWP_VP2Q (Q, E,P,n)

      RETURN
      end subroutine nwpd_td2q
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �䎼(g/kg) --> �I�_���x(K)
      SUBROUTINE NWPS_Q2TD (TD, Q,P,n)
      integer n
      real,dimension(n)::td,q,p,e

      CALL NWP_Q2VP (E, Q,P,n)
      CALL NWP_RVTETEN (TD, E,n)

      RETURN
      end subroutine nwps_q2td

      SUBROUTINE NWPD_Q2TD (TD, Q,P,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::td,q,p,e

      CALL NWP_Q2VP (E, Q,P,n)
      CALL NWP_RVTETEN (TD, E,n)

      RETURN
      end subroutine nwpd_q2td
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �䎼(g/kg) --> ���Ύ��x(%)
      SUBROUTINE NWPS_Q2RH (RH, Q,T,P,n)
      integer n
      real,dimension(n)::rh,q,t,p,e,es

      CALL NWP_Q2VP (E, Q,P,n)
      CALL NWP_TETEN (ES, T,n)
      RH = E/ES*100.

      RETURN
      end subroutine nwps_q2rh

      SUBROUTINE NWPD_Q2RH (RH, Q,T,P,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::rh,q,t,p,e,es

      CALL NWP_Q2VP (E, Q,P,n)
      CALL NWP_TETEN (ES, T,n)
      RH = E/ES*100.

      RETURN
      end subroutine nwpd_q2rh
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- ���Ύ��x(%) --> �䎼(g/kg)
      SUBROUTINE NWPS_RH2Q (Q, RH,T,P,n)
      integer n
      real,dimension(n)::q,rh,t,p,es,e

      CALL NWP_TETEN (ES, T,n)
      E = RH/100.*ES
      CALL NWP_VP2Q (Q, E,P,n)

      RETURN
      end subroutine nwps_rh2q

      SUBROUTINE NWPD_RH2Q (Q, RH,T,P,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::q,rh,t,p,es,e

      CALL NWP_TETEN (ES, T,n)
      E = RH/100.*ES
      CALL NWP_VP2Q (Q, E,P,n)

      RETURN
      end subroutine nwpd_rh2q
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- ������(g/kg) --> ���Ύ��x(%)
      SUBROUTINE NWPS_RM2RH (RH, RM,T,P,n)
      integer n
      real,dimension(n)::rh,rm,t,p,e,es

      CALL NWP_RM2VP (E, RM,P,n)
      CALL NWP_TETEN (ES, T,n)
      RH = E/ES*100.

      RETURN
      end subroutine nwps_rm2rh

      SUBROUTINE NWPD_RM2RH (RH, RM,T,P,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::rh,rm,t,p,e,es

      CALL NWP_RM2VP (E, RM,P,n)
      CALL NWP_TETEN (ES, T,n)
      RH = E/ES*100.

      RETURN
      end subroutine nwpd_rm2rh
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- ���Ύ��x(%) --> ������(g/kg)
      SUBROUTINE NWPS_RH2RM (RM, RH,T,P,n)
      integer n
      real,dimension(n)::rm,rh,t,p,es,e

      CALL NWP_TETEN (ES, T,n)
      E  = RH/100.*ES
      CALL NWP_VP2RM (RM, E,P,n)

      RETURN
      end subroutine nwps_rh2rm

      SUBROUTINE NWPD_RH2RM (RM, RH,T,P,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::rm,rh,t,p,es,e

      CALL NWP_TETEN (ES, T,n)
      E  = RH/100.*ES
      CALL NWP_VP2RM (RM, E,P,n)

      RETURN
      end subroutine nwpd_rh2rm
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �����C��(hPa) --> ���Ύ��x(%)
      SUBROUTINE NWPS_VP2RH (RH, E,T,n)
      integer n
      real,dimension(n)::rh,e,t,es

      CALL NWP_TETEN (ES, T,n)
      RH = E/ES*100.

      RETURN
      end subroutine nwps_vp2rh

      SUBROUTINE NWPD_VP2RH (RH, E,T,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::rh,e,t,es

      CALL NWP_TETEN (ES, T,n)
      RH = E/ES*100.

      RETURN
      end subroutine nwpd_vp2rh
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- ���Ύ��x(%) --> �����C��(hPa)
      SUBROUTINE NWPS_RH2VP (E, RH,T,n)
      integer n
      real,dimension(n)::e,rh,t,es

      CALL NWP_TETEN (ES, T,n)
      E = RH/100.*ES

      RETURN
      end subroutine nwps_rh2vp

      SUBROUTINE NWPD_RH2VP (E, RH,T,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::e,rh,t,es

      CALL NWP_TETEN (ES, T,n)
      E = RH/100.*ES

      RETURN
      end subroutine nwpd_rh2vp
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �����x(K) --> ���Ύ��x(%)
      SUBROUTINE NWPS_TV2RH (RH, TV,T,P,n)
      integer n
      real,dimension(n)::rh,tv,t,p,q,e,es

      CALL NWP_TV2Q (Q, TV,T,n)
      CALL NWP_Q2VP (E, Q,P,n)
      CALL NWP_TETEN (ES, T,n)
      RH = E/ES*100.

      RETURN
      end subroutine nwps_tv2rh

      SUBROUTINE NWPD_TV2RH (RH, TV,T,P,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::rh,tv,t,p,q,e,es

      CALL NWP_TV2Q (Q, TV,T,n)
      CALL NWP_Q2VP (E, Q,P,n)
      CALL NWP_TETEN (ES, T,n)
      RH = E/ES*100.

      RETURN
      end subroutine nwpd_tv2rh
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- ���Ύ��x(%) --> �����x(K)
      SUBROUTINE NWPS_RH2TV (TV, RH,T,P,n)
      integer n
      real,dimension(n)::tv,rh,t,p,es,e,q

      CALL NWP_TETEN (ES, T,n)
      E = ES*RH/100.
      CALL NWP_VP2Q (Q, E,P,n)
      CALL NWP_T2TV (TV, T,Q,n)

      RETURN
      end subroutine nwps_rh2tv

      SUBROUTINE NWPD_RH2TV (TV, RH,T,P,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::tv,rh,t,p,es,e,q

      CALL NWP_TETEN (ES, T,n)
      E = ES*RH/100.
      CALL NWP_VP2Q (Q, E,P,n)
      CALL NWP_T2TV (TV, T,Q,n)

      RETURN
      end subroutine nwpd_rh2tv
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �I�_���x(K) --> �����x(K)
      SUBROUTINE NWPS_TD2TV (TV, TD,T,P,n)
      integer n
      real,dimension(n)::tv,td,t,p,e,q

      CALL NWP_TETEN (E, TD,n)
      CALL NWP_VP2Q (Q, E,P,n)
      CALL NWP_T2TV (TV, T,Q,n)

      RETURN
      end subroutine nwps_td2tv

      SUBROUTINE NWPD_TD2TV (TV, TD,T,P,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::tv,td,t,p,e,q

      CALL NWP_TETEN (E, TD,n)
      CALL NWP_VP2Q (Q, E,P,n)
      CALL NWP_T2TV (TV, T,Q,n)

      RETURN
      end subroutine nwpd_td2tv
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �����x(K) --> �I�_���x(K)
      SUBROUTINE NWPS_TV2TD (TD, TV,T,P,n)
      integer n
      real,dimension(n)::td,tv,t,p,q,e

      CALL NWP_TV2Q (Q, TV,T,n)
      CALL NWP_Q2VP (E, Q,P,n)
      CALL NWP_RVTETEN (TD, E,n)

      RETURN
      end subroutine nwps_tv2td

      SUBROUTINE NWPD_TV2TD (TD, TV,T,P,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::td,tv,t,p,q,e

      CALL NWP_TV2Q (Q, TV,T,n)
      CALL NWP_Q2VP (E, Q,P,n)
      CALL NWP_RVTETEN (TD, E,n)

      RETURN
      end subroutine nwpd_tv2td
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �C��(K) --> ����(K)
      SUBROUTINE NWPS_T2PT (PT, T,P,n)
      integer n
      real,dimension(n)::pt,t,p

      RCP = RD/CPD

      PT = T * (1000./P)**RCP

      RETURN
      end subroutine nwps_t2pt

      SUBROUTINE NWPD_T2PT (PT, T,P,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::pt,t,p

      RCP = RD/CPD

      PT = T * (1000./P)**RCP

      RETURN
      end subroutine nwpd_t2pt
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- ����(K) --> �C��(K)
      SUBROUTINE NWPS_PT2T (T, PT,P,n)
      integer n
      real,dimension(n)::t,pt,p

      RCP = RD/CPD

      T = PT * (P/1000.)**RCP

      RETURN
      end subroutine nwps_pt2t

      SUBROUTINE NWPD_PT2T (T, PT,P,n)
      implicit double precision(a-h,o-z)
      integer n
      double precision,dimension(n)::t,pt,p

      RCP = RD/CPD

      T = PT * (P/1000.)**RCP

      RETURN
      end subroutine nwpd_pt2t
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! --- �C��(K) + �䎼(g/kg) --> ��������(K)  (�U��������)
!     SUBROUTINE NWPS_T2EPT (EPT, T,Q,P,n)
!     integer n
!     real,dimension(n)::ept,t,q,p,rm,te
!
!     RM = Q/(1000.-Q)          ! �䎼(g/kg) --> ������(kg/kg)
!
!     TE = T * EXP(LV*RM/CPD/T) ! �������x(K)
!
!     CALL NWP_T2PT (EPT, TE,P,n)   ! �������x(K) --> ��������(K)
!
!     RETURN
!     end subroutine nwps_t2ept
!
!     SUBROUTINE NWPD_T2EPT (EPT, T,Q,P,n)
!     implicit double precision(a-h,o-z)
!     integer n
!     double precision,dimension(n)::ept,t,q,p,rm,te
!
!     RM = Q/(1000.-Q)          ! �䎼(g/kg) --> ������(kg/kg)
!
!     TE = T * EXP(LV*RM/CPD/T) ! �������x(K)
!
!     CALL NWP_T2PT (EPT, TE,P,n)   ! �������x(K) --> ��������(K)
!
!     RETURN
!     end subroutine nwpd_t2ept

end module nwpl_felemsub

