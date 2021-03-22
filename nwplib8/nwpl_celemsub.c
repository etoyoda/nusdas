/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   ������ ���l�\��W�����C�u���� ������
 
   �����v�f�ϊ��֐��Q

   �ŏI�X�V�� | �ŏI�X�V��
   -----------+----------------------------------
   1999.07.   | ���l�\��ۃv���O������ �y��
   2005.10.12 | ���l�\��ۃv���O������ �ےJ
                 include "nwpl_nwpcst.h" ���폜���萔����荞��
                 nwp_t2ept(t,q,p)  �C���E�䎼���������� ���폜

   ���l�F
     elemsub.f�Ɋ܂܂��T�u���[�`���Q�ɑΉ����Ă���C�֐�

   �Q�l�F
     �×͊w���t�̎�  ���o/���� �� �|��*��
     ��ԕ�����      �o �� ��*�qd*�sv
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "nwpl_retcode.h"
#include "nwpl_elemsub.h"

static const double pi = 3.14159265358979;   /* �~������ */

static const double grv = 9.80665;           /* �d�͉����x(m/s) */
static const double Rd = 287.05;             /* ������C�̋C�̒萔(J/(kg*K)) */
static const double Cpd = 1005;              /* ������C�̒舳��M(J/(kg*K)) */
static const double Lv = 2.501e+6;           /* �Ì��̐��M (J/kg) */
static const double MvMd = 0.622;            /* Mv/Md */
static const double t0 = 273.15;             /* ���̗Z�_(K) */

/* --------------------------------------------------------------------------
    �C�����x�␳
   -------------------------------------------------------------------------- */
double nwp_prsvadj(double p,double tv,double z,double gamma) {

/* ����
  double p     : ���n�C��
  double tv    : �␳���x�ł̉����x
  double z     : �W����
  double gamma : �C������ */

  double c,e;

  c = (tv-gamma*z)/tv;
  e = grv/(Rd*gamma);
  return p*pow(c,e);
}

/* --------------------------------------------------------------------------
    �C���䁨���x��
   -------------------------------------------------------------------------- */
double nwp_rp2dz(double rp,double tv,double gamma) {

/* ����                                       
  double rp    : �Q�w�Ԃ̋C����  rp=p(A)/p(B)
  double tv    : ���w���̉����x
  double gamma : �C������ */

  double e;

  if(gamma==0.0) {               /* ���� */
    return Rd*tv/grv*log(rp);
  } else {
    e = -gamma*Rd/grv;
    return tv/gamma*(1-pow(rp,e));
  }
}

/* --------------------------------------------------------------------------
    ���x�����C����
   -------------------------------------------------------------------------- */
double nwp_dz2rp(double dz,double tv,double gamma) {

/* ����                                       
  double dz    : �Q�w�Ԃ̍��x��  dz=z(B)-z(A)
  double tv    : ���w���̉����x
  double gamma : �C������ */

  double e;

  if(gamma==0.0) {               /* ���� */
    return exp(grv*dz/(Rd*tv));
  } else {
    e = -grv/(Rd*gamma);
    return pow((1-gamma*dz/tv),e);
  }
}

/* --------------------------------------------------------------------------
    �w���������x
   -------------------------------------------------------------------------- */
double nwp_th2tv(double th,double pl,double ph) {

/* �����F
  double th : �w��
  double pl : ���[�̋C��
  dobule ph : ��[�̋C�� */

  double lnp;

  lnp = log(pl/ph);
  return grv*th/(Rd*lnp);
}
  
/* --------------------------------------------------------------------------
    �����x���w��
   -------------------------------------------------------------------------- */
double nwp_tv2th(double tv,double pl,double ph) {

/* �����F
  double tv : �w��
  double pl : ���[�̋C��
  dobule ph : ��[�̋C�� */

  double lnp;

  lnp = log(pl/ph);
  return Rd*lnp*tv/grv;
}

/* --------------------------------------------------------------------------
    ����������(U,V)
   -------------------------------------------------------------------------- */
void nwp_df2uv(double *u,double *v,double d,double f) {

/* �����F
   �����Ȃ��Ă��킩�邾��I */

  double t;

  t = d*pi/180.0;
  *u = -f*sin(t);
  *v = -f*cos(t);
}

/* --------------------------------------------------------------------------
    (U,V)����������
   -------------------------------------------------------------------------- */
void nwp_uv2df(double *d,double *f,double u,double v) {

/* �����F
   ��Ɠ������I */

  double t;

  *f = sqrt(pow(u,2.0)+pow(v,2.0));
  if(*f==0.0) { 
    *d = 0.0;
  } else {
    t = asin(fabs(u)/(*f));
    if(u>=0.0 && v>=0.0) { t =   pi+t; }
    if(u>=0.0 && v< 0.0) { t = 2*pi-t; }
    if(u< 0.0 && v>=0.0) { t =   pi-t; }
    *d = t*180.0/pi;
  }
}

/* --------------------------------------------------------------------------
    ���ʏ��(U,V)���Q�W���ܐ������x���g���e�ʏ��(U,V)
   -------------------------------------------------------------------------- */
void nwp_uv2lm(double *ul,double *vl,double u,double v,double lon,double slon, \
           double slate,double slatp) {

/* �����F
  (OUT) double *ul   : �����x���g�ʏ�̂t
  (OUT) double *vl   : �����x���g�ʏ�̂u
  (IN)  double u     : ���ʏ�̂t
  (IN)  double v     : ���ʏ�̂u
  (IN)  double lon   : �n�_�o�x
  (IN)  double slon  : �����x���g�ʏ�ł̕W���o�x
  (IN)  dobule slate : �ԓ����̕W���ܐ�
  (IN)  dobule slatp : �ɑ��̕W���ܐ� */

  double rad,l,t,slater,slatpr;
  rad = pi/180.0;
  slater = slate*rad;
  slatpr = slatp*rad;
  l = log(sin(slater)/sin(slatpr))/log(tan(slater/2.0)/tan(slatpr/2.0));
  t = (lon-slon)*rad*l;

  *ul = u*cos(t)-v*sin(t);
  *vl = u*sin(t)+v*cos(t);
}

/* --------------------------------------------------------------------------
    �Q�W���ܐ������x���g���e�ʏ��(U,V)�����ʏ��(U,V)
   -------------------------------------------------------------------------- */
void nwp_lm2uv(double *u,double *v,double ul,double vl,double lon,double slon, \
           double slate,double slatp) {

/* �����F
  (OUT) double *u    : ���ʏ�̂t
  (OUT) double *v    : ���ʏ�̂u
  (IN)  double ul    : �����x���g�ʏ�̂t
  (IN)  double vl    : �����x���g�ʏ�̂u
  (IN)  double lon   : �n�_�o�x
  (IN)  double slon  : �����x���g�ʏ�ł̕W���o�x
  (IN)  dobule slate : �ԓ����̕W���ܐ�
  (IN)  dobule slatp : �ɑ��̕W���ܐ� */

  double rad,l,t,slater,slatpr;

  rad = pi/180.0;
  slater = slate*rad;
  slatpr = slatp*rad;
  l = log(sin(slater)/sin(slatpr))/log(tan(slater/2.0)/tan(slatpr/2.0));
  t = (lon-slon)*rad*l;

  *u =  ul*cos(t)+vl*sin(t);
  *v = -ul*sin(t)+vl*cos(t);
}

/* --------------------------------------------------------------------------
    �C���^�O�a�����C��(Teten �̎�)
   -------------------------------------------------------------------------- */
double nwp_teten(double t) {

  const double e0=6.11,al=17.3,bl=237.3,ai=21.9,bi=265.3;
  double tc;

  tc = t-t0;
  if(tc>=0.0) {             /* ���ɑ΂���O�a���C�� */
    return e0*exp(al*tc/(bl+tc));
  } else if(tc<=-15.0) {    /* �X�ɑ΂���O�a���C�� */
    return e0*exp(ai*tc/(bi+tc));
  } else {                  /* ���ƕX���� */
    return e0*(exp(al*tc/(bl+tc))*(15.0+tc)/15.0\
              +exp(ai*tc/(bi+tc))*(    -tc)/15.0);
  }
}

double nwp_rvteten(double e) {

  const double e0=6.11,al=17.3,bl=237.3,ai=21.9,bi=265.3;
  double el,ei,em15,cc,t;
  double ee[151],tt[151];
  int i;

  em15 = nwp_teten(t0-15.0);
  if(e<=em15) {              /* �X */
    cc = log(e/e0);
    t = bi*cc/(ai-cc)+t0;
  } else if(e>=e0) {         /* �� */
    cc = log(e/e0);
    t = bl*cc/(al-cc)+t0;
  } else {                   /* �����X */
    /*poption noparallel */
    for (i=0; i<151; i++) {
      tt[i] = -0.1*i;
      el = e0*exp(al*tt[i]/(bl+tt[i]));
      ei = e0*exp(ai*tt[i]/(bi+tt[i]));
      ee[i] = (el*(15.0+tt[i])-ei*tt[i])/15.0;
      switch(i){
        case 0: continue;
        case 150: break;
      }
      if(ee[i]<e) {
        break;
      }
    }
    t = tt[i-1]-(tt[i-1]-tt[i])*(ee[i-1]-e)/(ee[i-1]-ee[i])+t0;
  }
  return t;
}

/* --------------------------------------------------------------------------
    �����x�^�C��
   -------------------------------------------------------------------------- */
double nwp_tv2t(double tv,double q) {
  return tv/(1.0+0.608*1.0e-3*q);
}
double nwp_t2tv(double t,double q) {
  return (1.0+0.608*1.0e-3*q)*t;
}

/* --------------------------------------------------------------------------
    �����x�^�䎼
   -------------------------------------------------------------------------- */
double nwp_tv2q(double tv,double t) {
  return (tv/t-1.0)/0.608*1.0e+3;
}
double nwp_q2tv(double q,double t) {
  return (1.0+0.608*q*1.0e-3)*t;
}

/* --------------------------------------------------------------------------
    ������^�䎼
   -------------------------------------------------------------------------- */
double nwp_r2q(double r) {
  return r/(1.0+r*1.0e-3);
}
double nwp_q2r(double q) {
  return q/(1.0-q*1.0e-3);
}

/* --------------------------------------------------------------------------
    �����C���^�䎼
   -------------------------------------------------------------------------- */
double nwp_vp2q(double e,double p) {
  return e*MvMd/(p-(1.0-MvMd)*e)*1.0e+3;
}
double nwp_q2vp(double q,double p) {
  return p*q*1.0e-3/(MvMd+q*1.0e-3*(1.0-MvMd));
}

/* --------------------------------------------------------------------------
    �����C���^������
   -------------------------------------------------------------------------- */
double nwp_vp2r(double e,double p) {
  return e*MvMd/(p-e)*1.0e+3;
}
double nwp_r2vp(double r,double p) {
  return p*r*1.0e-3/(MvMd+r*1.0e-3);
}

/* --------------------------------------------------------------------------
    �I�_���x�^���Ύ��x
   --------------------------------------------------------------------------*/
double nwp_td2rh(double td,double t) {
  return nwp_teten(td)/nwp_teten(t)*100.0;
}
  
double nwp_rh2td(double rh,double t) {
  double es,e;
  es = nwp_teten(t);
  e = es*rh/100.0;
  return nwp_rvteten(e);
}

/* --------------------------------------------------------------------------
    �䎼�^�I�_���x
   -------------------------------------------------------------------------- */
double nwp_q2td(double q,double p) {
  return nwp_rvteten(nwp_q2vp(q,p));
}
double nwp_td2q(double td,double p) {
  return nwp_vp2q(nwp_teten(td),p);
}
/* --------------------------------------------------------------------------
    �䎼�^���Ύ��x
   -------------------------------------------------------------------------- */
double nwp_q2rh(double q,double t,double p) {
  double es,e;
  es = nwp_teten(t);
  e = nwp_q2vp(q,p);
  return e/es*100.0;
}
double nwp_rh2q(double rh,double t,double p) {
  double e;
  e = rh/100.0*nwp_teten(t);
  return nwp_vp2q(e,p);
}
/* --------------------------------------------------------------------------
    ������^���Ύ��x
   -------------------------------------------------------------------------- */
double nwp_r2rh(double r,double t,double p) {
  double es,e;
  es = nwp_teten(t);
  e = nwp_r2vp(r,p);
  return e/es*100.0;
}
double nwp_rh2r(double rh,double t,double p) {
  double e;
  e = rh/100.0*nwp_teten(t);
  return nwp_vp2r(e,p);
}
/* --------------------------------------------------------------------------
    �����C���^���Ύ��x
   -------------------------------------------------------------------------- */
double nwp_vp2rh(double e,double t) {
  return e/nwp_teten(t)*100.0;
}
double nwp_rh2vp(double rh,double t) {
  return rh/100.0*nwp_teten(t);
}
/* --------------------------------------------------------------------------
    �����x�^���Ύ��x
   -------------------------------------------------------------------------- */
double nwp_tv2rh(double tv,double t,double p) {
  double q,es,e;
  q = nwp_tv2q(tv,t);
  e = nwp_q2vp(q,p);
  es = nwp_teten(t);
  return e/es*100.0;
}
double nwp_rh2tv(double rh,double t,double p) {
  double e,q;
  e = rh/100.0*nwp_teten(t);
  q = nwp_vp2q(e,p);
  return nwp_t2tv(t,q);
}
/* --------------------------------------------------------------------------
    �I�_���x�^�����x
   -------------------------------------------------------------------------- */
double nwp_td2tv(double td,double t,double p) {
  double e,q;
  e = nwp_teten(td);
  q = nwp_vp2q(e,p);
  return nwp_t2tv(t,q);
}
double nwp_tv2td(double tv,double t,double p) {
  double q,e;
  q = nwp_tv2q(tv,t);
  e = nwp_q2vp(q,p);
  return nwp_rvteten(e);
}
/* --------------------------------------------------------------------------
    �C���^����
   -------------------------------------------------------------------------- */
double nwp_t2pt(double t,double p) {
  return t*pow((1000.0/p),Rd/Cpd);
}
double nwp_pt2t(double pt,double p) {
  return pt*pow((1000.0/p),-Rd/Cpd);
}
