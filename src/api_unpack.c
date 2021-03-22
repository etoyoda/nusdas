#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <stddef.h>
#include "sys_kwd.h"
# define NEED_STR2SYM4
# define NEED_MEM2SYM4
#include "sys_sym.h"
# define NEED_PEEK_N_UI4
#include "sys_endian.h"
#include "sys_err.h"
#include "dfile.h"
#include "ndf_codec.h"

/** @brief 生DATAレコードの解読
 *
 * nusdas_inq_data() の問い合わせ N_DATA_CONTENT で得られるバイト列を
 * 解読して数値配列を得る。
 *
 * @retval 正 正常終了、値は要素数
 * @retval -4 展開先の大きさ @p usize がデータレコードの要素数より少ない
 * @retval -5 パッキング型・欠損値型・展開型の組合せが不適
 *
 * <H3>履歴</H3>
 * 本関数は NuSDaS 1.3 で追加された。
 */
N_SI4
NuSDaS_unpack(const void *pdata /**< パックされたバイト列 */,
		void *udata, /**< 展開先配列 */
		const char utype[2], /**< 展開する型 */
		N_SI4 usize /**< 展開先配列の要素数 */)
{
	struct ndf_codec_t *codec;
	struct ibuffer_t ibuf;
	const N_UI1 *rawdata = pdata;
	sym4_t packing, missing, buffmt;
	N_UI4 nx, ny;
	N_SI4 r;
	nx = PEEK_N_UI4(rawdata);
	ny = PEEK_N_UI4(rawdata + 4);
	if ((usize < 0) || (nx * ny > (N_UI4)usize)) {
		return nus_err((-4, "user buffer %Pu < %Pu elements required",
			usize, nx * ny));
	}
	packing = MEM2SYM4(rawdata + 8);
	missing = MEM2SYM4(rawdata + 12);
	buffmt = mem2sym4(utype);
	codec = ndf_get_codec(packing, missing, buffmt);
	if (codec == NULL || codec->decode == NULL) {
		return nus_err((-5, "missing codec <%Ps|%Ps|%.2s>",
			       packing, missing, utype));
	}
	if (packing == SYM4_2UPJ) {
		return nus_err((-6, "nusdas_unpack does not support %Ps",
			       packing));
	}
	ibuf.ib_ptr = udata;
	ibuf.ib_fmt = buffmt;
	ibuf.nelems = nx * ny;
	cut_rectangle_disable(&ibuf.ib_cut);
	r = codec->decode(&ibuf, rawdata + 16, nx, ny);
	NUSDAS_CLEANUP;
	if (r < 0) {
		return NUS_ERR_CODE();
	}
	return r;
}
