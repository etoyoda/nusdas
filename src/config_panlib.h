/*
 * pandora_lib.c を単独出荷する場合には
 * このファイルを config.h とする。
*/

#ifndef CONFIG_PANLIB_H
# define CONFIG_PANLIB_H
# define UNUSED
# define nus_malloc(size)        malloc((size))
# define nus_free(ptr)           free((ptr))
# define nus_realloc(ptr, size)  realloc((ptr, size))

#endif
