/** @file
 * @brief �ؿ� nusdas_snprintf() �μ���
 */

#include "config.h"
#include "nusdas.h"
#include <stddef.h>
#include <stdarg.h>
# define NEED_VSNPRINTF
#include "sys_err.h"

/** @brief ����Х��ȿ��б� snprintf()
 *
 * �� @p fmt �˽�������ʹߤΰ�����ʸ���󲽤���
 * Ĺ�� @p bufsize ������ @p buf �˳�Ǽ���롣
 * printf(3) ��Ʊ�͡��Ѵ������ `%' ʸ�����ե饰 (���ץ����)��
 * ������ (1--9) �ˤ������� (���ץ����)��
 * �ԥꥪ�� (`.') �����֤���������ˤ������ (���ץ����)��
 * ������ (���ץ����)��
 * �Ѵ�����ʸ������ʤ롣
 * �Ѵ�����ʳ��� @p fmt ���ʸ���Ϥ��Τޤ�ž�̤���롣
 *
 * @retval �� �ºݤ˽񤭹��ޤ줿ʸ���� (�̥뽪ü��ޤޤʤ�)
 * @retval -1 ���顼
 *
 * <H3>�ե饰</H3>
 * <DL>
 * <DT>0 (����)<DD>�����Ѵ�
 * (`a', `d', `b', `x', `X', `u', `o') �η�̤�
 * �������������ʤ���硢��¦�˥��������롣
 * <DT>$-$<DD>�Ѵ����ʸ������������Ǻ��󤻤��롣
 * <DT>+<DD>�Ѵ����� `d' �ޤ��� `a' �ˤĤ��ơ�
 * ���ͤ����ΤȤ� `+' ������ά���ʤ���
 * <DT>#<DD>����Ū�񼰤���Ѥ��롣
 * </DL>
 *
 * <H3>������</H3>
 * <UL>
 * <LI>�������ϡ��Ѵ��η�������ޤ��ʸ��������Ǿ��λ�������ꤹ�롣
 * <LI>�Ѵ���̤��������������ʤ���硢��¦�˥��ڡ����������
 * (����ե饰�ˤ�äƵ�ư���Ѥ�����)��
 * <LI>�Ѵ���̤���������Ķ�����硢���Τޤ��Ѥ����롣
 * �Ĥޤꡢ��ʬ�ʰ�������Ϳ���ʤ��Ƚ񼰤�����뤳�Ȥ����롣
 * <LI>������ `y', `m' �ޤ����Ѵ����� `%', `c' �Ǥϰ������ϸ��̤�⤿�ʤ���
 * </UL>
 *
 * <H3>����</H3>
 * <UL>
 * <LI>��ư���������Ѵ� (`a') �ˤĤ��Ƥϡ����٤����������������Ȥʤ롣
 * <LI>��ư���������Ѵ� (`g') �ˤĤ��Ƥϡ����٤����������������Ȥʤ롣
 * <LI>ʸ�����Ѵ� (`s') �ˤĤ��Ƥϡ��Ѵ��������ޤ��ʸ���Ϻ����
 * (����) �ĤǤ��롣
 * Fortran �����Ϥ��줿�ǡ����Τ褦�˥̥뽪ü����Ƥ��ʤ�
 * ����Ĺʸ����ΰ������Ѥ����뤬��
 * �̥�ʸ��������Ф����ǻߤޤäƤ��ޤ� (�㳰������)��
 * <LI>����¾���Ѵ�����ˤ����Ƥ����٤ϸ��̤�⤿�ʤ���
 * </UL>
 *
 * <H3>������</H3>
 * Ŭ�Ѳ�ǽ���Ѵ�������դ��Ƥϸġ����Ѵ�����򸫤衣
 * <DL>
 * <DT>l (��ʸ���Υ���)<DD>������ long ���ޤ��� unsigned long ���Ǥ��롣
 * <DT>z<DD>������ size_t ���Ǥ��� (C99 ���)��
 * <DT>P<DD>������ N_SI4 ���ޤ��� N_UI4 ���Ǥ��롣
 * <DT>Q<DD>������ N_SI8 ���ޤ��� N_UI8 ���Ǥ��롣
 * �����η�����¤�ΤȤ��Ƽ�������Ƥ���Ķ��Ǥ⡢
 * ���ץꥱ�������ץ������Ϲ�¤�ΤؤΥݥ��󥿤ǤϤʤ���¤�Τ��Τ�Τ�
 * �������Ϥ����ȡ�
 * <DT>y<DD>������ struct nustype_t (�饤�֥���������������¤��)
 * �ؤΥݥ��󥿤Ǥ��롣`s' �Ѵ�����ˤ������ѤǤ��ơ�
 * ��ɤΤȤ��� <TT>%ys</TT> �Ϥ������� <TT>%Qs%Ps%Ps</TT> �Ǥ��뤫�Τ褦��
 * ����1, ����2, ����3 ��Ϣ�뤷�ư������롣
 * �ޤ� <TT>%#ys</TT> �Ϥ������� <TT>%Qs.%Ps.%Ps</TT> �Ǥ��뤫�Τ褦��
 * �ԥꥪ�ɶ��ڤ�ǰ������� (�ѥ�ɥ���)��
 * ���ΤȤ������������٤ϸ��̤�⤿�ʤ���
 * <DT>m<DD>������ struct nusdims_t (�饤�֥���������������¤��)
 * �ؤΥݥ��󥿤Ǥ��롣`s' �Ѵ�����ˤ������ѤǤ��ơ�
 * ��ɤΤȤ��� <TT>%ms</TT> �Ϥ�������
 * <TT>%12PT/%4.4s/%12PT/%6.6s/%6.6s</TT> �Ǥ��뤫�Τ褦��
 * ����å�����ڤ�Ǵ��������̾���оݻ���1, ��1, ����̾��������롣
 * ����̾�����٤ƥ��ڡ����ΤȤ��� ``<TT>none</TT>'' ���ִ�����롣
 * �ޤ� <TT>%#ms</TT> �Ϥ�������
 * <TT>%#15PT/%4.4s/%#15PT/%Ps/%Ps</TT> �Ǥ��뤫�Τ褦��
 * ���֤˶��ڤ�ʸ���������ȤȤ�˶����ͤ�� (�ѥ�ɥ���)��
 * ���ΤȤ������������٤ϸ��̤�⤿�ʤ���
 * </DL>
 *
 * <H3>�Ѵ�����</H3>
 * <DL>
 * <DT>%<DD>�ѡ�����ȵ��� `<TT>%</TT>' ���Τ�Τ�������롣�����ϻȤ�ʤ���
 * <DT>d<DD>
 * ����դ������������򽽿�ɽ�����롣
 * ������� `Q', `P', `l', `z' ��ǧ�����롣
 * ����ʤ��а����� int �Ȥߤʤ���롣
 * <DT>b<DD>
 * ���̵�����������������ɽ�����롣
 * ������� `Q', `P', `l', `z' ��ǧ�����롣
 * ����ʤ��а����� unsigned �Ȥߤʤ���롣
 * <DT>x<DD>
 * ���̵��������������ϻ��ɽ������ (�ѻ��Ͼ�ʸ�����Ѥ���)��
 * ������� `Q', `P', `l', `z' ��ǧ�����롣
 * ����ʤ��а����� unsigned �Ȥߤʤ���롣
 * <DT>X<DD>
 * ���̵��������������ϻ��ɽ������ (�ѻ�����ʸ�����Ѥ���)��
 * ������� `Q', `P', `l', `z' ��ǧ�����롣
 * ����ʤ��а����� unsigned �Ȥߤʤ���롣
 * <DT>u<DD>
 * ���̵�������������򽽿�ɽ�����롣
 * ������� `Q', `P', `l', `z' ��ǧ�����롣
 * ����ʤ��а����� unsigned �Ȥߤʤ���롣
 * <DT>o<DD>
 * ���̵��������������Ȭ��ɽ�����롣
 * ������� `Q', `P', `l', `z' ��ǧ�����롣
 * ����ʤ��а����� unsigned �Ȥߤʤ���롣
 * <DT>p<DD>
 * �ݥ��󥿤������˥��㥹�Ȥ�����Τ�ϻ��ɽ�����롣
 * AIX �� 64 �ӥåȥ⡼�ɤǤϥ����ƥ�� printf(3) �������ˤ�
 * ���� 32 �ӥåȤΤߤ�ɽ������Τ��Ф���
 * �ܴؿ��������� 64 �ӥåȤ�ɽ���Ǥ��롣
 * <DT>T<DD>
 * ����դ����������������ͽ���̻�ʬ�Ȥߤʤ��ư������롣
 * `#' �ե饰����ꤹ���ǯ������ϥ��ե� (`<TT>-</TT>') �Ƕ��ڤꡢ
 * ���Ȼ��δ֤� `<TT>t</TT>' ʸ������������ (�ѥ�ɥ���)��
 * ������� `Q', `P', `l', `z' ��ǧ�����롣
 * `Q' ����ꤹ���
 * �������� <TT>%PT/%PT</TT> �Ǥ��뤫�Τ褦��
 * ��̡����� 32 �ӥåȤο��ͤ� 2 �Ĥλ���Ȥ��ư������롣
 * <DT>a<DD>
 * ��ư����������ϻ�ʻؿ�ɽ������ (C99 ���)��
 * �׻�������ư�����������ˤ�����餺 IEEE �����ˤ�롣
 * <DT>g<DD>
 * ��ư���������򽽿ʻؿ�ɽ�����롣
 * �ؿ����� -4 ���� (���� - 1) �ΤȤ��ϸ��꾮����ɽ���Ȥʤ롣
 * ��ά�������� 6 �Ǥ��롣
 * ������ˤ��ƤⲾ���������Υ����ϰ������줺���������������ʤ�
 * ���������������ʤ���
 * <DT>c<DD>
 * ������ unsigned �����ͤȤߤʤ��Ƥ���ʸ�������ɤ���ʸ����������롣
 * �������ϸ��̤�⤿�ʤ���
 * <DT>s<DD>
 * ������ʸ����Ȥ���ɾ�����ư������롣
 * �����꤬�ʤ����ϰ����ϥ̥뽪ü�� char * �Ȥߤʤ���롣
 * �����꤬ `Q' �ޤ��� `P' �ΤȤ���
 * �����ͤ�ɽ�魯 8 �Х��Ȥޤ��� 4 �Х��ȤΥХ������
 * ʸ����Ȥ��Ƥ��Τޤ� (�Х��ȥ��������Ѵ���������) ��ᤷ����ΤǤ��롣
 * ���ΤȤ��̥�ʸ���Ͻ�ü�ǤϤʤ���
 * ������Ǥ�ոĤΥ��ڡ������������
 * ��������ʸ����ü�Ǥ��뤫�Τ褦�˰����롣
 * ������ `y' �� `m' �ˤĤ��Ƥ����໲�ȡ�
 * </DL>
 *
 * <H3>����</H3>
 * <UL>
 * <LI>����Ĺ @p bufsize ����­�����票�顼�Ȥʤ롣
 * ����ˤ�äƥХåե������С��ե�����������򤱤뤳�Ȥ��Ǥ��롣
 * <LI>ɸ��ؿ� sprintf(3) �ν񼰤ΰ����ϥ��ݡ��Ȥ���Ƥ��ʤ���
 * <LI>ʸ���� (s) �Ѵ������ ASCII �޷�ʸ���ʳ��ΥХ��ʥ��
 * �������褦�Ȥ����
 * <TT>\BACKSLASH 0</TT>,
 * <TT>\BACKSLASH t</TT>,
 * <TT>\BACKSLASH n</TT>,
 * <TT>\BACKSLASH r</TT> ���뤤��
 * <TT>\BACKSLASH 377</TT> �Τ褦��Ȭ��ɽ���ǰ�������롣
 * ����� <TT>%.8s</TT> �Τ褦�����٤���ꤹ��� 8 �Х��Ȥ��٤�
 * ��������ʤ����Ȥ����롣
 * <LI>�Х�: �����ե饰�Ͽ��ͤ��Ѵ���̤���椬�դ������б����Ƥ��ʤ�
 * (NuSDaS �����ǤϻȤäƤ��ʤ�)��
 * </UL>
 * <H3>����</H3>
 * ���δؿ��� NuSDaS 1.3 ��Ƴ�����줿��
 */
int NuSDaS_snprintf(char *buf, /**< ��̳�Ǽ���� */
		unsigned bufsize, /**< ������󥵥��� */
		const char *fmt, /**< �� */
		... /**< �������٤��� */)
{
    va_list ap;
    int len;
    va_start(ap, fmt);
    len = nus_vsnprintf(buf, bufsize, fmt, ap);
    va_end(ap);
    return len;
}