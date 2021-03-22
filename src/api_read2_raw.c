#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include "sys_err.h"

/** @brief DATA��Ͽ���Ƥ�ľ���ɼ�
 * �����ǻ��ꤷ��TYPE, ��������С����оݻ���̡����ǤΥǡ�����
 * �ե�����˳�Ǽ���줿�ޤޤη������ɤ߽Ф���
 * �ǡ����ϡ�DATA �쥳���ɤΥե����ޥå�ɽ�ι���10��14�ޤǤΥǡ�����
 * ��Ǽ����롣
 * @retval �� �ɤ߽Ф��Ƴ�Ǽ�����Х��ȿ���
 * @retval 0 ���ꤷ���ǡ�����̤��Ͽ(����ե������ elementmap �ˤ�äƽ񤭹��ޤ�뤳�Ȥϵ��Ƥ���Ƥ��뤬���ޤ��ǡ������񤭹��ޤ�Ƥ��ʤ�)
 * @retval -2 ���ꤷ���ǡ����ϵ�Ͽ���뤳�Ȥ����Ƥ���Ƥ��ʤ�(elementmap �ˤ�äƶػߤ���Ƥ�����Ȼ��ꤷ����̾������̾����Ͽ����Ƥ��ʤ�����ξ����ޤ�)��
 * @retval -4 ��Ǽ������­
 * <H3> ���� </H3>
 * ���δؿ��� NuSDaS1.1 ��Ƴ�����줿��
 */
	N_SI4
NuSDaS_read2_raw(const char type1[8], /**< ����1 */
		const char type2[4], /**< ����2 */
		const char type3[4], /**< ����3 */
	     	const N_SI4 *basetime, /**< ������(�̻�ʬ) */
		const char member[4], /**< ���С�̾ */
		const N_SI4 *validtime1, /**< �оݻ���1 */
		const N_SI4 *validtime2, /**< �оݻ���2 */
		const char plane1[6], /**< ��1 */
		const char plane2[6], /**< ��2 */
		const char element[6], /**< ����̾ */
		void *buf, /**< INTENT(OUT) �ǡ�����Ǽ���� */
		const N_SI4 *buf_nbytes) /**< �ǡ�����Ǽ����ΥХ��ȿ� */
{
	N_SI4 r;
	/* NUSDAS_INIT ������ */
	r = nusdas_inq_data2(type1, type2, type3, basetime, member,
			validtime1, validtime2, plane1, plane2, element,
			N_DATA_CONTENT, buf, buf_nbytes);
	if (r == -1) {
		r = -4;
	}
	return r;
}
