/** @file
 * @brief gzip 圧縮・展開を行うサービスサブルーチン
 */
#include "config.h"
#include "nusdas.h"
#include "nus_gzlib.h"
#include <stddef.h>
#include "sys_err.h"

/** @brief gzip 圧縮
 *
 * 入力データ @p in_data を gzip 圧縮して @p out_buf に格納する。
 * @retval -98 NuSDaS が ZLib を使うように設定されていない。
 * @retval -9 ZLib の deflateEnd 関数がエラーを起こした。
 * @retval -4 結果領域の長さ @p out_nbytes が不足している。
 * @retval -2 ZLib の deflateInit2 関数がエラーを起こした。
 * @retval -1 ZLib の deflate 関数がエラーを起こした。
 * @retval 他 圧縮データの長さ
 * <H3>履歴</H3>
 * 本関数は NuSDaS 1.3 で新設された。
 */
	N_SI4
NuSDaS_gzip(const void *in_data UNUSED, /**< 入力データ */
		N_UI4 in_nbytes UNUSED, /**< 入力データのバイト数 */
		void *out_buf UNUSED, /**< INTENT(OUT) 圧縮結果を格納する領域 */
		N_UI4 out_nbytes UNUSED /**< 結果領域のバイト数 */)
{
#ifdef USE_ZLIB
	return nusgz_compress((void *)in_data, in_nbytes, out_buf, out_nbytes);
#else
	return NUSERR_NoZLib;
#endif
}

/** @brief gzip 圧縮データの展開後の長さを得る
 *
 * 入力データ @p in_data を gzip 展開するときに必要となる
 * 結果格納領域のバイト数を返す。
 * @retval -98 NuSDaS が ZLib を使うように設定されていない。
 * @retval 正 展開後の長さ
 * <H3>履歴</H3>
 * 本関数は NuSDaS 1.3 で新設された。
 */
	N_SI4
NuSDaS_gunzip_nbytes(const void *in_data UNUSED, /**< 圧縮データ */
		N_UI4 in_nbytes UNUSED /**< 圧縮データのバイト数 */)
{
#ifdef USE_ZLIB
	return nusgz_inq_decompressed_size((void *)in_data, in_nbytes);
#else
	return NUSERR_NoZLib;
#endif
}

/** @brief gzip 圧縮データを展開
 *
 * 入力データ @p in_data を gzip 展開して @p out_buf に格納する。
 * @retval -98 NuSDaS が ZLib を使うように設定されていない。
 * @retval -99 入力は gzip 圧縮形式ではない。
 * @retval -5 展開結果の長さが圧縮データと不整合。
 * @retval -4 結果領域の長さ @p out_nbytes が不足している。
 * @retval -3 展開結果の CRC32 が圧縮データと不整合。
 * @retval -2 ZLib の inflateInit 関数がエラーを起こした。
 * @retval -1 ZLib の inflate 関数がエラーを起こした。
 * @retval 他 展開データのバイト数
 * <H3>履歴</H3>
 * 本関数は NuSDaS 1.3 で新設された。
 */

	N_SI4
NuSDaS_gunzip(const void *in_data UNUSED, /**< 圧縮データ */
		N_UI4 in_nbytes UNUSED, /**< 圧縮データのバイト数 */
		void *out_buf UNUSED, /**< INTENT(OUT) 展開結果を格納する領域 */
		N_UI4 out_nbytes UNUSED /**< 結果領域のバイト数 */
	     )
{
#ifdef USE_ZLIB
	return nusgz_decompress((void *)in_data, in_nbytes, out_buf, out_nbytes);
#else
	return NUSERR_NoZLib;
#endif
}
