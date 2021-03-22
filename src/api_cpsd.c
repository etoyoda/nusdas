#include "config.h"
#include "nusdas.h"
#include "sys_kwd.h"
#include "internal_types.h"
#include <stddef.h>
#include <string.h>
# define NEED_MEM2SYM4
#include "sys_sym.h"
# define NEED_PEEK_N_UI4
#include "sys_endian.h"
# define NEED_MEMCPY4
#include "sys_string.h"
#include "sys_err.h"
#include "dfile.h"
#include "ndf_codec.h"

/** @brief 2UPPの生DATAレコードを2UPCに変換したサイズのバイト数を取得
 *
 * nusdas_inq_data() の問い合わせ N_DATA_CONTENT で得られる2UPPバイト列を
 * 2UPCに変換した場合のバイト数を取得する
 *
 * @retval 正 正常終了、値は展開後バイト数
 * @retval -5 入力が2UPPではない
 *
 * <H3>履歴</H3>
 * 本関数は NuSDaS 1.4 で追加された。
 */
N_SI4
NuSDaS_uncpsd_nbytes(const void *pdata /**< パックされたバイト列 */)
{
	const N_UI1 *rawdata = pdata;
	N_UI4 grid_size, header;
	grid_size = PEEK_N_UI4(rawdata) * PEEK_N_UI4(rawdata + 4);
	
	if ( SYM4_2UPP != MEM2SYM4(rawdata + 8) ) { /* packing */
		return nus_err((-5, "UnCPSD input is not 2UPP"));
	}
	switch( MEM2SYM4(rawdata + 12) ) { /* missing */
		case SYM4_NONE: header = 24; break;
		case SYM4_UDFV: header = 26; break;
		case SYM4_MASK: header = 24 + (grid_size + 7) / 8; break;
	}
	return 2 * grid_size + header;
}

/** @brief 2UPPの生DATAレコードを2UPCに変換
 *
 * nusdas_inq_data() の問い合わせ N_DATA_CONTENT で得られる2UPPバイト列を
 * 解読して2UPCバイト列に変換する。
 *
 * @retval 正 正常終了、値は展開後バイト数
 * @retval -4 展開先の大きさ @p usize がデータレコードの要素数より少ない
 * @retval -5 入力が2UPPではない
 * @retval -6 2UPPの展開に失敗
 * 
 * <H3>履歴</H3>
 * 本関数は NuSDaS 1.4 で追加された。
 */
N_SI4
NuSDaS_uncpsd(const void *pdata /**< パックされたバイト列 */,
		void *cdata, /**< 展開先配列 */
		N_SI4 csize /**< 展開先配列の要素数 */)
{
	const N_UI1 *rawdata = pdata;
	const sym4_t packing = SYM4_2UPC;
	N_UI4 grid_size, header, expand_size;
	
	grid_size = PEEK_N_UI4(rawdata) * PEEK_N_UI4(rawdata + 4);
	if ( SYM4_2UPP != MEM2SYM4(rawdata + 8) ) { /* packing */
		return nus_err((-5, "UnCPSD input is not 2UPP"));
	}
	switch( MEM2SYM4(rawdata + 12) ) { /* missing */
		case SYM4_NONE: header = 24; break;
		case SYM4_UDFV: header = 26; break;
		case SYM4_MASK: header = 24 + (grid_size + 7) / 8; break;
	}
	expand_size = 2 * grid_size + header;
	if ((csize < 0) || (expand_size > csize)) {
		return nus_err((-4, "user buffer %Pu < %Pu elements required",
			csize, expand_size));
	}
	/* nus_decode_cpsd の第3引数は実は全く利用されない */
	if ( sizeof(N_UI2) * grid_size != nus_decode_cpsd(rawdata + header, (N_UI1 *)cdata + header, grid_size) ){
		return nus_err((-6, "UnCPSD failure"));
	}
	/* copy header */
	memcpy(cdata, pdata, header);
	memcpy4((char *)cdata + 8, (const char *)&packing);
	return expand_size;
}
