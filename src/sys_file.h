/** @file
 * @brief �ե��������.
 *
 * ��ñ�ʥե��������ϴ�ñ�� (������ stdio ��Ȥ鷺��) �񤱤�褦�ˡ�
 */
#ifndef NUSDAS_CONFIG_H
# error please include config.h
#endif
#ifndef NULL
# error please include either stddef.h or stdio.h for size_t
#endif

extern unsigned char *
file_read(const char *pathname, size_t *size);
extern size_t
file_size(const char *pathname);
extern size_t
file_read_size(unsigned char *buf, size_t size, const char *pathname);
extern int
make_dirs(const char *pathname);
extern int
check_tape(const char* pathname);
