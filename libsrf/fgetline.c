#include <stdio.h>
#include <string.h>

/** @brief 1行の読み取り
 *
 * やることはほとんど fgets(3) と同じで存在意義はない。
 * 違いはバッファオーバーフローが起こった時にヌル終端されないことと、
 * fread(3) がエラーを起こした場合に探知できないことである。だめじゃん。
 *
 * 以前この関数は srf_amd_rdic() で使われていた。
 */
int
fgetline(char line[], /**< 1行読み取るバッファ */
	 FILE * fp, /**< 入力ファイル */
	 int max /**< 行バッファの長さ */)
{
#if 0
    char c;
    int i, j;
    i = 0;
    while (1) {
	j = fread(&c, 1, 1, fp);
	if (c == '\n' || j == 0) {
	    if (j == 0 && i == 0)
		return (-1);
	    line[i++] = '\0';
	    return i;
	}
	line[i++] = c;
	if (i > max) {
	    return -2;
	}
    }
#endif
    char *p;
    size_t n;
    p = fgets(line, max, fp);
    if (p == NULL)
	    return -1;
    n = strlen(line);
    if (n == (size_t)(max - 1))
	    return -2;
    return n;
}
