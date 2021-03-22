/** @file
 * @brief RLE 圧縮関連のサービスサブルーチン
 */
#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include "internal_types.h"
#include "dfile.h"
#include "ndf_codec.h"

/** @brief 1バイト整数を RLE 圧縮する
 * <H3>履歴</H3>
 * この関数は NuSDaS 1.0 から存在するが、ドキュメントされていなかった。
 * NuSDaS 1.3 から Fortran API を伴う
 * サービスサブルーチンとして採録された。
 */
	N_SI4
NuSDaS_encode_rlen_8bit_I1(const unsigned char udata[], /**< 元データ配列 */
		unsigned char compressed_data[], /**< INTENT(OUT) 結果格納配列 */
		N_SI4 udata_nelems, /**< 元データの要素数 */
		N_SI4 max_compress_nbytes, /**< 結果配列のバイト数 */
		N_SI4 *maxvalue) /**< INTENT(OUT) 元データの最大値 */
{
	return nus_compress_rlen_i1(udata, udata_nelems, maxvalue,
			compressed_data, max_compress_nbytes);
}

/** @brief 4バイト整数を RLE 圧縮する
 * <H3>履歴</H3>
 * この関数は NuSDaS 1.0 から存在するが、ドキュメントされていなかった。
 * NuSDaS 1.3 から Fortran API を伴う
 * サービスサブルーチンとして採録された。
 */
	N_SI4
NuSDaS_encode_rlen_8bit(const N_SI4 udata[], /**< 元データ配列 */
		unsigned char compressed_data[], /**< INTENT(OUT) 結果格納配列 */
		N_SI4 udata_nelems, /**< 元データの要素数 */
		N_SI4 max_compress_nbytes, /**< 結果配列のバイト数 */
		N_SI4 *maxvalue) /**< INTENT(OUT) 元データの最大値 */
{
	return nus_compress_rlen_i4(udata, udata_nelems, maxvalue,
			compressed_data, max_compress_nbytes);
}
