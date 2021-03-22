#include <stdio.h>
#include <string.h>

/** @brief 1�s�̓ǂݎ��
 *
 * ��邱�Ƃ͂قƂ�� fgets(3) �Ɠ����ő��݈Ӌ`�͂Ȃ��B
 * �Ⴂ�̓o�b�t�@�I�[�o�[�t���[���N���������Ƀk���I�[����Ȃ����ƂƁA
 * fread(3) ���G���[���N�������ꍇ�ɒT�m�ł��Ȃ����Ƃł���B���߂����B
 *
 * �ȑO���̊֐��� srf_amd_rdic() �Ŏg���Ă����B
 */
int
fgetline(char line[], /**< 1�s�ǂݎ��o�b�t�@ */
	 FILE * fp, /**< ���̓t�@�C�� */
	 int max /**< �s�o�b�t�@�̒��� */)
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
