/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

   ◇◇◇ 数値予報標準ライブラリ ◇◇◇
 
   elemsub.c で定義されている関数のプロトタイプ宣言

   最終更新日 | 最終更新者
   -----------+----------------------------------
   1999.07.   | 数値予報課プログラム班 及川

   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

double nwp_prsvadj(double,double,double,double);
double nwp_rp2dz(double,double,double);
double nwp_dz2rp(double,double,double);
double nwp_th2tv(double,double,double);
double nwp_tv2th(double,double,double);
void nwp_df2uv(double*,double*,double,double);
void nwp_uv2df(double*,double*,double,double);
void nwp_uv2lm(double*,double*,double,double,double,double,double,double);
void nwp_lm2uv(double*,double*,double,double,double,double,double,double);
double nwp_teten(double);
double nwp_rvteten(double);
double nwp_tv2t(double,double);
double nwp_t2tv(double,double);
double nwp_tv2q(double,double);
double nwp_q2tv(double,double);
double nwp_r2q(double);
double nwp_q2r(double);
double nwp_vp2q(double,double);
double nwp_q2vp(double,double);
double nwp_vp2r(double,double);
double nwp_r2vp(double,double);
double nwp_td2rh(double,double);
double nwp_rh2td(double,double);
double nwp_q2td(double,double);
double nwp_td2q(double,double);
double nwp_q2rh(double,double,double);
double nwp_rh2q(double,double,double);
double nwp_r2rh(double,double,double);
double nwp_rh2r(double,double,double);
double nwp_vp2rh(double,double);
double nwp_rh2vp(double,double);
double nwp_tv2rh(double,double,double);
double nwp_rh2tv(double,double,double);
double nwp_td2tv(double,double,double);
double nwp_tv2td(double,double,double);
double nwp_t2pt(double t,double p);
double nwp_pt2t(double t,double p);
double nwp_t2ept(double,double,double);

