#define _FILE_OFFSET_BITS 64
#include "config.h"
#include "nusdas.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h> /* for nus_malloc */
#include <string.h> /* for memcpy */
# define NEED_MEMCPY_HTON4
#include "sys_endian.h"
# define NEED_LONG_TO_SI8
#include "sys_int.h"
#include "sys_mem.h"

struct nusdas_bigfile {
	int	bf_fd;
	int	bf_mode;
	int	bf_active;
};

#define POOLSIZE 16

/* ゼロ初期化をあてにしている */
static N_BIGFILE Pool[POOLSIZE];

/** @brief ファイルを開く
 *
 * ファイルを開く。
 * OS がサポートしていれば 32ビット環境でも
 * 2GB または 4GB を超えるラージファイルを開くことができる。
 *
 * @retval NULL 失敗
 * @retval 他 成功。このポインタを今後のファイル操作に用いる
 * <H3>履歴</H3>
 * この関数は NuSDaS 1.3 で追加された。
 * */
	N_BIGFILE *
bfopen(const char *pathname, /**< ファイル名 */
	const char *mode /**< モード指定 */)
{
	N_BIGFILE *bf;
	const char *p;
	int i;
	if (mode == NULL)
		return NULL;
	for (i = 0; i < POOLSIZE; i++) {
		if (Pool[i].bf_active == 0) {
			bf = &(Pool[i]);
			bf->bf_active = 1;
			goto Found;
		}
	}
	return NULL;
Found:
	for (p = mode; *p; p++) {
		switch (*p) {
			case 'r':
				bf->bf_mode = O_RDONLY;
				break;
			case 'w':
				bf->bf_mode = O_WRONLY | O_CREAT;
				break;
			case '+':
				if (bf->bf_mode & O_APPEND) {
					bf->bf_mode = O_RDWR | O_CREAT
						| O_APPEND;
				} else {
					bf->bf_mode = O_RDWR | O_CREAT;
				}
				break;
			case 'a':
				bf->bf_mode = O_WRONLY | O_CREAT | O_APPEND;
				bf->bf_active = 0;
				return NULL;
			default:
				/* do nothing */;
		}
	}
	if ((bf->bf_fd = open(pathname, bf->bf_mode, 0666)) == -1) {
		bf->bf_active = 0;
		return NULL;
	}
	return bf;
}

/** @brief ファイルを閉じる
 *
 * あらかじめ bfopen() で開かれた
 * ファイル @p bf を閉じる。これ以後ポインタ @p bf を参照してはならない。
 * @retval 0 正常終了
 * @retval -1 エラー
 * <H3>履歴</H3>
 * この関数は NuSDaS 1.3 で追加された。
 */
	int
bfclose(N_BIGFILE *bf) /**< ファイル */
{
	bf->bf_mode = 0;
	return close(bf->bf_fd);
}

/** @brief ファイル入力
 *
 * あらかじめ bfopen() で開かれた
 * ファイル @p bf から @p ptr が指すバッファに @p nbytes バイト読み出す。
 * @retval 正 読み出されたバイト数 (ファイル末尾などでは @p nbytes より少ない)
 * @retval 0 ちょうどファイル末尾から読み出そうとしたか、エラー
 * <H3>履歴</H3>
 * この関数は NuSDaS 1.3 で追加された。
 */
	unsigned long
bfread(void *ptr, /**< 読みだし先バッファ */
	unsigned long nbytes, /**< バイト数 */
	N_BIGFILE *bf) /**< ファイル */
{
	ssize_t r;
	r = read(bf->bf_fd, ptr, nbytes);
	if (r < 0) {
		return 0;
	}
	return r;
}

/** @brief ファイル出力
 *
 * あらかじめ bfopen() で開かれた
 * ファイル @p bf に対して @p ptr が指すバッファから @p nbytes バイト
 * 書き出す。
 * @retval 正 書き込まれたバイト数 (エラー時に @p nbytes より少ないことがある)
 * @retval 0 ちょうど書き込み開始時にエラーが起こった
 * <H3>履歴</H3>
 * この関数は NuSDaS 1.3 で追加された。
 */
	unsigned long
bfwrite(void *ptr, /**< 書き込み元バッファ */
	unsigned long nbytes, /**< バイト数 */
	N_BIGFILE *bf) /**< ファイル */
{
	ssize_t r;
	r = write(bf->bf_fd, ptr, nbytes);
	if (r < 0) {
		return 0;
	}
	return r;
}

/** @brief バイトオーダー変換付きファイル入力
 *
 * あらかじめ bfopen() で開かれた
 * ファイル @p bf から幅 @size バイトのオブジェクトを @p nmemb 個読出す。
 * ファイルにはビッグエンディアンで書かれていることが仮定され、
 * 結果は機械に自然なバイトオーダーで @p ptr に書き出される。
 * @retval 正 読み込まれたオブジェクト数 (エラー時に nmemb より少ないことがある)
 * @retval 0 ちょうどファイル末尾から読出そうとしたか、エラー
 * <H3>参考</H3>
 * <UL>
 * <LI>引数 @p size にふさわしい値は sizeof 演算子によって得られる。
 * </UL>
 * <H3>履歴</H3>
 * この関数は NuSDaS 1.3 で追加された。
 */
	unsigned long
bfread_native(void *ptr, /**< 読出し先バッファ */
		unsigned long size, /**< オブジェクトの幅 */
		unsigned long nmemb, /**< オブジェクトの個数 */
		N_BIGFILE *bf /**< ファイル */)
{
	unsigned long nbytes, r;
	nbytes = size * nmemb;
	r = bfread(ptr, nbytes, bf);
	if (r < nbytes) {
		return r / size;
	}
	switch (size) {
		case 1:
			/* do nothing */;
			break;
		case 2:
			endian_swab2(ptr, nmemb);
			break;
		case 4:
			endian_swab4(ptr, nmemb);
			break;
		case 8:
			endian_swab8(ptr, nmemb);
		default:
			return 0;
	}
	return r / size;
}

/** @brief バイトオーダー変換付きファイル出力
 *
 * あらかじめ bfopen() で開かれた
 * ファイル @p bf に幅 @size バイトのオブジェクトを @p nmemb 個書き込む。
 * 書き込むデータは機械に自然なバイトオーダーで @p ptr から読み込まれ、
 * ファイルにはビッグエンディアンで書かれる。
 * @retval 正 書き出されたオブジェクト数 (エラー時に nmemb より少ないことがある)
 * @retval 0 エラー
 * <H3>参考</H3>
 * <UL>
 * <LI>引数 @p size にふさわしい値は sizeof 演算子によって得られる。
 * </UL>
 * <H3>履歴</H3>
 * この関数は NuSDaS 1.3 で追加された。
 */
	unsigned long
bfwrite_native(void *ptr, /**< データ */
		unsigned long size, /**< オブジェクト長 */
		unsigned long nmemb, /**< オブジェクト数 */
		N_BIGFILE *bf /**< ファイル */)
{
	unsigned long r;
	size_t nbytes = size * nmemb;
	void *buf;
	if ((buf = nus_malloc(nbytes)) == NULL) {
		return 0;
	}
	switch (size) {
		case 1:
			memcpy(buf, ptr, nmemb);
			break;
		case 2:
			memcpy(buf, ptr, nbytes);
			endian_swab2(ptr, nmemb);
			break;
		case 4:
			memcpy_hton4(buf, ptr, nmemb);
			break;
		case 8:
			memcpy(buf, ptr, nbytes);
			endian_swab8(ptr, nmemb);
			break;
		default:
			nus_free(buf);
			return 0;
	}
	r = bfwrite(buf, nbytes, bf);
	nus_free(buf);
	return r / size;
}

/** @brief ファイル位置取得
 *
 * あらかじめ bfopen() で開かれた
 * ファイル @p bf の現在位置を @p pos に書き出す。
 * @retval 0 正常終了
 * @retval -1 エラー
 * <H3>注意</H3>
 * <UL>
 * <LI>64 ビット整数がないコンパイラ用に configure した場合
 * N_SI8 は構造体であり算術演算に用いることはできない。
 * </UL>
 * <H3>履歴</H3>
 * この関数は NuSDaS 1.3 で追加された。
 */
	int
bfgetpos(N_BIGFILE *bf, /**< ファイル */
		N_SI8 *pos /**< 位置 */)
{
	off_t opos;
	opos = lseek(bf->bf_fd, 0, SEEK_CUR);
	if (opos == (off_t)-1) {
		return -1;
	}
	*pos = off_to_si8(opos);
	return 0;
}

/** @brief ファイル位置設定
 *
 * あらかじめ bfopen() で開かれた
 * ファイル @p bf の現在位置を
 * あらかじめ bfgetpos() で得られた位置 @p pos に設定する。
 * @retval 0 正常終了
 * @retval -1 エラー
 * <H3>履歴</H3>
 * この関数は NuSDaS 1.3 で追加された。
 */
	int
bfsetpos(N_BIGFILE *bf /**< ファイル */,
		N_SI8 pos /**< 位置 */)
{
	off_t opos;
	opos = si8_to_off(pos);
	opos = lseek(bf->bf_fd, opos, SEEK_SET);
	if (opos == (off_t)-1) {
		return -1;
	}
	return 0;
}

/** @brief ファイル位置設定
 *
 * あらかじめ bfopen() で開かれた
 * ファイル @p bf の現在位置を設定する。
 *
 * ファイル位置の起算原点は @p whence によって異なる。
 * <DL>
 * <DT>SEEK_SET<DD>ファイル先頭から @p offset バイト (非負) 進んだ位置
 * <DT>SEEK_CUR<DD>現在位置から @p offset バイト (負でもよい) 進んだ位置
 * <DT>SEEK_END<DD>ファイル末尾から @p offset バイト進んだ位置
 * </DL>
 * @retval 0 正常終了
 * @retval -1 エラー
 * <H3>注意</H3>
 * <UL>
 * <LI>long が 32 ビット幅の場合、2 ギガバイトを超えるファイルでは
 * 指定できない場所がある。
 * <LI>@p whence に SEEK_END を指定して正の @p offset を指定した場合の
 * 挙動については OS の lseek(2) 等のマニュアルを参照されたい。
 * </UL>
 * <H3>履歴</H3>
 * この関数は NuSDaS 1.3 で追加された。
 */
	int
bfseek(N_BIGFILE *bf, /**< ファイル */
		long offset, /**< 相対位置 */
		int whence /**< 起点 */)
{
	off_t opos;
	opos = lseek(bf->bf_fd, offset, whence);
	if (opos == (off_t)-1) {
		return -1;
	}
	return 0;
}
