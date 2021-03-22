/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   ◇◇◇ 数値予報標準ライブラリ ◇◇◇
 
   物理要素変換関数群

   最終更新日 | 最終更新者
   -----------+----------------------------------
   1999.07.   | 数値予報課プログラム班 及川
   2005.10.12 | 数値予報課プログラム班 保谷
                 include "nwpl_nwpcst.h" を削除し定数を取り込む
                 nwp_t2ept(t,q,p)  気温・比湿→相当温位 を削除

   備考：
     elemsub.fに含まれるサブルーチン群に対応しているC関数

   参考：
     静力学平衡の式  ｄＰ/ｄｚ ＝ －ρ*ｇ
     状態方程式      Ｐ ＝ ρ*Ｒd*Ｔv
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "nwpl_retcode.h"
#include "nwpl_elemsub.h"

static const double pi = 3.14159265358979;   /* 円周率π */

static const double grv = 9.80665;           /* 重力加速度(m/s) */
static const double Rd = 287.05;             /* 乾燥空気の気体定数(J/(kg*K)) */
static const double Cpd = 1005;              /* 乾燥空気の定圧比熱(J/(kg*K)) */
static const double Lv = 2.501e+6;           /* 凝結の潜熱 (J/kg) */
static const double MvMd = 0.622;            /* Mv/Md */
static const double t0 = 273.15;             /* 水の融点(K) */

/* --------------------------------------------------------------------------
    気圧高度補正
   -------------------------------------------------------------------------- */
double nwp_prsvadj(double p,double tv,double z,double gamma) {

/* 引数
  double p     : 現地気圧
  double tv    : 補正高度での仮温度
  double z     : 標高差
  double gamma : 気温減率 */

  double c,e;

  c = (tv-gamma*z)/tv;
  e = grv/(Rd*gamma);
  return p*pow(c,e);
}

/* --------------------------------------------------------------------------
    気圧比→高度差
   -------------------------------------------------------------------------- */
double nwp_rp2dz(double rp,double tv,double gamma) {

/* 引数                                       
  double rp    : ２層間の気圧比  rp=p(A)/p(B)
  double tv    : 下層側の仮温度
  double gamma : 気温減率 */

  double e;

  if(gamma==0.0) {               /* 等温 */
    return Rd*tv/grv*log(rp);
  } else {
    e = -gamma*Rd/grv;
    return tv/gamma*(1-pow(rp,e));
  }
}

/* --------------------------------------------------------------------------
    高度差→気圧比
   -------------------------------------------------------------------------- */
double nwp_dz2rp(double dz,double tv,double gamma) {

/* 引数                                       
  double dz    : ２層間の高度差  dz=z(B)-z(A)
  double tv    : 下層側の仮温度
  double gamma : 気温減率 */

  double e;

  if(gamma==0.0) {               /* 等温 */
    return exp(grv*dz/(Rd*tv));
  } else {
    e = -grv/(Rd*gamma);
    return pow((1-gamma*dz/tv),e);
  }
}

/* --------------------------------------------------------------------------
    層厚→仮温度
   -------------------------------------------------------------------------- */
double nwp_th2tv(double th,double pl,double ph) {

/* 引数：
  double th : 層厚
  double pl : 下端の気圧
  dobule ph : 上端の気圧 */

  double lnp;

  lnp = log(pl/ph);
  return grv*th/(Rd*lnp);
}
  
/* --------------------------------------------------------------------------
    仮温度→層厚
   -------------------------------------------------------------------------- */
double nwp_tv2th(double tv,double pl,double ph) {

/* 引数：
  double tv : 層厚
  double pl : 下端の気圧
  dobule ph : 上端の気圧 */

  double lnp;

  lnp = log(pl/ph);
  return Rd*lnp*tv/grv;
}

/* --------------------------------------------------------------------------
    風向風速→(U,V)
   -------------------------------------------------------------------------- */
void nwp_df2uv(double *u,double *v,double d,double f) {

/* 引数：
   書かなくてもわかるだろ！ */

  double t;

  t = d*pi/180.0;
  *u = -f*sin(t);
  *v = -f*cos(t);
}

/* --------------------------------------------------------------------------
    (U,V)→風向風速
   -------------------------------------------------------------------------- */
void nwp_uv2df(double *d,double *f,double u,double v) {

/* 引数：
   上と同じだ！ */

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
    球面上の(U,V)→２標準緯線ランベルト投影面上の(U,V)
   -------------------------------------------------------------------------- */
void nwp_uv2lm(double *ul,double *vl,double u,double v,double lon,double slon, \
           double slate,double slatp) {

/* 引数：
  (OUT) double *ul   : ランベルト面上のＵ
  (OUT) double *vl   : ランベルト面上のＶ
  (IN)  double u     : 球面上のＵ
  (IN)  double v     : 球面上のＶ
  (IN)  double lon   : 地点経度
  (IN)  double slon  : ランベルト面上での標準経度
  (IN)  dobule slate : 赤道側の標準緯線
  (IN)  dobule slatp : 極側の標準緯線 */

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
    ２標準緯線ランベルト投影面上の(U,V)→球面上の(U,V)
   -------------------------------------------------------------------------- */
void nwp_lm2uv(double *u,double *v,double ul,double vl,double lon,double slon, \
           double slate,double slatp) {

/* 引数：
  (OUT) double *u    : 球面上のＵ
  (OUT) double *v    : 球面上のＶ
  (IN)  double ul    : ランベルト面上のＵ
  (IN)  double vl    : ランベルト面上のＶ
  (IN)  double lon   : 地点経度
  (IN)  double slon  : ランベルト面上での標準経度
  (IN)  dobule slate : 赤道側の標準緯線
  (IN)  dobule slatp : 極側の標準緯線 */

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
    気温／飽和水蒸気圧(Teten の式)
   -------------------------------------------------------------------------- */
double nwp_teten(double t) {

  const double e0=6.11,al=17.3,bl=237.3,ai=21.9,bi=265.3;
  double tc;

  tc = t-t0;
  if(tc>=0.0) {             /* 水に対する飽和蒸気圧 */
    return e0*exp(al*tc/(bl+tc));
  } else if(tc<=-15.0) {    /* 氷に対する飽和蒸気圧 */
    return e0*exp(ai*tc/(bi+tc));
  } else {                  /* 水と氷を按分 */
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
  if(e<=em15) {              /* 氷 */
    cc = log(e/e0);
    t = bi*cc/(ai-cc)+t0;
  } else if(e>=e0) {         /* 水 */
    cc = log(e/e0);
    t = bl*cc/(al-cc)+t0;
  } else {                   /* 水＆氷 */
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
    仮温度／気温
   -------------------------------------------------------------------------- */
double nwp_tv2t(double tv,double q) {
  return tv/(1.0+0.608*1.0e-3*q);
}
double nwp_t2tv(double t,double q) {
  return (1.0+0.608*1.0e-3*q)*t;
}

/* --------------------------------------------------------------------------
    仮温度／比湿
   -------------------------------------------------------------------------- */
double nwp_tv2q(double tv,double t) {
  return (tv/t-1.0)/0.608*1.0e+3;
}
double nwp_q2tv(double q,double t) {
  return (1.0+0.608*q*1.0e-3)*t;
}

/* --------------------------------------------------------------------------
    混合比／比湿
   -------------------------------------------------------------------------- */
double nwp_r2q(double r) {
  return r/(1.0+r*1.0e-3);
}
double nwp_q2r(double q) {
  return q/(1.0-q*1.0e-3);
}

/* --------------------------------------------------------------------------
    水蒸気圧／比湿
   -------------------------------------------------------------------------- */
double nwp_vp2q(double e,double p) {
  return e*MvMd/(p-(1.0-MvMd)*e)*1.0e+3;
}
double nwp_q2vp(double q,double p) {
  return p*q*1.0e-3/(MvMd+q*1.0e-3*(1.0-MvMd));
}

/* --------------------------------------------------------------------------
    水蒸気圧／混合比
   -------------------------------------------------------------------------- */
double nwp_vp2r(double e,double p) {
  return e*MvMd/(p-e)*1.0e+3;
}
double nwp_r2vp(double r,double p) {
  return p*r*1.0e-3/(MvMd+r*1.0e-3);
}

/* --------------------------------------------------------------------------
    露点温度／相対湿度
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
    比湿／露点温度
   -------------------------------------------------------------------------- */
double nwp_q2td(double q,double p) {
  return nwp_rvteten(nwp_q2vp(q,p));
}
double nwp_td2q(double td,double p) {
  return nwp_vp2q(nwp_teten(td),p);
}
/* --------------------------------------------------------------------------
    比湿／相対湿度
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
    混合比／相対湿度
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
    水蒸気圧／相対湿度
   -------------------------------------------------------------------------- */
double nwp_vp2rh(double e,double t) {
  return e/nwp_teten(t)*100.0;
}
double nwp_rh2vp(double rh,double t) {
  return rh/100.0*nwp_teten(t);
}
/* --------------------------------------------------------------------------
    仮温度／相対湿度
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
    露点温度／仮温度
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
    気温／温位
   -------------------------------------------------------------------------- */
double nwp_t2pt(double t,double p) {
  return t*pow((1000.0/p),Rd/Cpd);
}
double nwp_pt2t(double pt,double p) {
  return pt*pow((1000.0/p),-Rd/Cpd);
}
