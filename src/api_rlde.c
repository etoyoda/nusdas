/** @file
 * @brief RLE 圧縮関連のサービスサブルーチン
 */
#include "config.h"
#include "nusdas.h"
#include <stdlib.h>
#include "sys_mem.h"

#define N_DUMMY_INT INT_MAX

int n_nbitflag = 0;
N_UI4 n_b2s[32];

/** @brief 整数の冪乗
 */
	INLINE N_UI4
n_ipow(N_UI4 radix, N_UI4 raiseto)
{
	N_UI4 i, r;
	r = 1;
	for (i = 0; i < raiseto; i++) {
		r *= radix;
	}
	return r;
}

	INLINE N_SI4
n_nbit_unpack(const N_UI1 din[], N_SI4 nin, N_UI4 dout[], N_SI4 nout,N_SI4 nbit)
/* -------------------------------------------------------------------------- */
{
   
  N_UI4 wi;
  int i, bitp, dp;
   
  if (n_nbitflag == 0)  {
    for(i = 0; i < 32; i++) {
      wi = n_ipow(2, nbit) - 1;
      n_b2s[i] = wi << i;
    }
    n_nbitflag = 1;
  }
   
  i = 0; dp = 0; bitp = 0;
  while(dp < nin)  {
    bitp += nbit;
    wi = 0;
    if(dp < nin - 3){
        wi = (((unsigned int)(din[dp])) << 24)
            | (((unsigned int)(din[dp+1])) << 16)
            | (((unsigned int)(din[dp+2])) << 8)
            | (((unsigned int)(din[dp+3])));
    }
    else if(dp == nin - 3){
        if(bitp > 24){
            return (i);
        }
        wi= (((unsigned int)(din[dp])) << 24)
            | (((unsigned int)(din[dp+1])) << 16)
            | (((unsigned int)(din[dp+2])) << 8);
    }
    else if(dp == nin - 2){
        if(bitp > 16){
            return (i);
        }
        wi= (((unsigned int)(din[dp])) << 24)
            | (((unsigned int)(din[dp+1])) << 16);
    }
    else if(dp == nin - 1){
        if(bitp > 8){
            return (i);
        }
        wi= (((unsigned int)(din[dp])) << 24);
    }
    dout[i++] = (wi >> (32 - bitp)) & n_b2s[0];
    while (bitp >= 8) {
      dp++;
      bitp -= 8;
    }
    if (i > nout)
      return(-1);
  }
  
  return(i);
}

	static N_SI4
internal_decode(N_UI1 *udata, const N_UI1 *din, N_SI4 nin, N_SI4 nout,
                         N_SI4 maxv, N_SI4 nbit)
/* ------------------------------------------------------------------------- */
/* decode ranlength compress ( nbit data version )                            */
/*   output:   dout   = user data for put                                     */
/*   input:    din    = compressed data                                       */
/*             nin    = compressed data size (byte)                           */
/*             nout   = allocated item number for user data                   */
/*             maxv   = maximum value of user data                            */
/*             nbit   = number of bit used for a compressed data              */
/*   return:   >=0    = number of decoded data                                */
/*             -4     = uncompressed data size exceeds nout                   */
/*             -6     = first user data is out of the data range              */
/* -------------------------------------------------------------------------- */
{
    N_SI4 i, j, k, l, m = N_DUMMY_INT, n = N_DUMMY_INT, v, p,  ninb;
    N_UI4 *wdat;
    N_UI1 *dout;

    wdat = (N_UI4 *)nus_malloc(sizeof(N_UI4)*nout);
    ninb = n_nbit_unpack(din, nin, wdat, nout, nbit);

    if (ninb<0)  return(-4);
    dout = (N_UI1 *)udata;
    l = n_ipow(2,nbit)-1-maxv;
    v = (N_SI4)(*wdat);

    if (v<0 || v>maxv)  return(-6);

    i = 0; k = 0; p = -1;
    while(i<ninb) {
        v = (*(wdat+i++));
        if (v<=maxv) {
            if (p>=0) {
                for(j=0; j<m; j++) {
                    if (k==nout)  return(-4);
                    *(dout+k++) = p;
                }
            }
            p = v;
            m = 1;
            n = 0;
        } else {
            m += n_ipow(l,n++)*(v-maxv-1);
        }
    }
    for(j=0; j<m; j++) {
        if (k==nout)  return(-4);
        *(dout+k++) = p;
    }

    nus_free(wdat);
    return(k);
}

/** @brief RLE データを展開する
 * <H3>履歴</H3>
 * この関数は NuSDaS 1.0 から存在するが、ドキュメントされていなかった。
 * NuSDaS 1.3 から Fortran API を伴う
 * サービスサブルーチンとして採録された。
 */
	N_SI4
NuSDaS_decode_rlen_nbit_I1(
		unsigned char udata[], /**< INTENT(OUT) 結果格納配列 */
		const unsigned char compressed_data[], /**< 圧縮データ */
		N_SI4 compressed_nbytes, /**< 圧縮データのバイト数 */
		N_SI4 udata_nelems, /**< 圧縮データの要素数 */
		N_SI4 maxvalue, /**< データの最大値 */
		N_SI4 nbit) /**< 圧縮データのビット数 */
{
	return internal_decode(udata, compressed_data,
			compressed_nbytes, udata_nelems, maxvalue, nbit);
}
