#include <stdio.h>
struct lustre_append{
  size_t size;
  int append;
  char* tail;
};
struct lustre_append iosub_prepare_recover_stdio(FILE* file, const size_t expect_size);
struct lustre_append iosub_prepare_recover_posix(const int file, const size_t expect_size);
int iosub_fsync(const int fd);
int iosub_lustre_flush(const int fd);
int iosub_check_size(const char* filename, const size_t expect_size);
int iosub_recover_lustre(const char* filename, const struct lustre_append* la, const unsigned int try_count);
void iosub_free(struct lustre_append* la);
