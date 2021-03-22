/** @file
 * @brief メモリ管理
 */

#ifdef USE_NUS_MALLOC
void *nus_malloc(size_t nbytes);
void *nus_realloc(void *ptr, size_t nbytes);
void nus_free(void *ptr);
#else
# define nus_malloc(nbytes)	malloc(nbytes)
# define nus_realloc(ptr, nbytes)	realloc(ptr, nbytes)
# define nus_free(ptr)	free(ptr)
#endif
