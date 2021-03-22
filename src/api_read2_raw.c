#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include "sys_err.h"

/** @brief DATA記録内容の直接読取
 * 引数で指定したTYPE, 基準時刻、メンバー、対象時刻、面、要素のデータを
 * ファイルに格納されたままの形式で読み出す。
 * データは、DATA レコードのフォーマット表の項番10〜14までのデータが
 * 格納される。
 * @retval 正 読み出して格納したバイト数。
 * @retval 0 指定したデータは未記録(定義ファイルの elementmap によって書き込まれることは許容されているが、まだデータが書き込まれていない)
 * @retval -2 指定したデータは記録することが許容されていない(elementmap によって禁止されている場合と指定した面名、要素名が登録されていない場合の両方を含む)。
 * @retval -4 格納配列が不足
 * <H3> 履歴 </H3>
 * この関数は NuSDaS1.1 で導入された。
 */
	N_SI4
NuSDaS_read2_raw(const char type1[8], /**< 種別1 */
		const char type2[4], /**< 種別2 */
		const char type3[4], /**< 種別3 */
	     	const N_SI4 *basetime, /**< 基準時刻(通算分) */
		const char member[4], /**< メンバー名 */
		const N_SI4 *validtime1, /**< 対象時刻1 */
		const N_SI4 *validtime2, /**< 対象時刻2 */
		const char plane1[6], /**< 面1 */
		const char plane2[6], /**< 面2 */
		const char element[6], /**< 要素名 */
		void *buf, /**< INTENT(OUT) データ格納配列 */
		const N_SI4 *buf_nbytes) /**< データ格納配列のバイト数 */
{
	N_SI4 r;
	/* NUSDAS_INIT は不要 */
	r = nusdas_inq_data2(type1, type2, type3, basetime, member,
			validtime1, validtime2, plane1, plane2, element,
			N_DATA_CONTENT, buf, buf_nbytes);
	if (r == -1) {
		r = -4;
	}
	return r;
}
