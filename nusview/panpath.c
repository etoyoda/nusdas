/* panpath.c - パンドラパス文字列と nus_dims_t の相互変換
 * 2001-08-30 豊田英司
 * 2003-02-03 KH = k.hatano - nusdims_to_path() plane1,plane2,element length
 */

#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>
#include "nusdas.h"
#include "nusdim.h"
#include "stringplus.h"

#if 0
static char rcsid[] = "$Id: panpath.c,v 1.1 2007-02-23 06:57:30 suuchi43 Exp $";
#endif
/* 
 * nus_dims 型をクリアする
 */
	void
nusdims_clear(struct nus_dims_t *dp)
{
#	define SPACE ' '
	dp->stat = 0;
	memset(&(dp->type.type1), SPACE, 8);
	memset(&(dp->type.type2), SPACE, 4);
	memset(&(dp->type.type3), SPACE, 4);
	dp->base = 1;
	memset(&(dp->member), SPACE, 4);
	dp->valid = 1;
	dp->valid2 = 1;
	memset(&(dp->plane), SPACE, 6);
	memset(&(dp->plane2), SPACE, 6);
	memset(&(dp->element), SPACE, 6);
}

/* nus_dims_t からパス文字列を作成する。
 * 結果は static 領域で返される。
 */
	char *
nusdims_to_path(const struct nus_dims_t *dp)
{
	/* 配列長の見積り:
	* (type:18) + 1 + 3*((time:15) + 1) + (member:4) + 1 
	*  + 3*(plane-or-elem:6 + 1) = 93
	*/
	static char	buffer[128];
	char	*tail, *endbuf;

	tail = buffer;
	endbuf = buffer + sizeof(buffer);

	/* --- type --- */
	tail = string_copy(tail, endbuf-tail, "/");
	tail = fstring_to_string(tail, endbuf-tail, dp->type.type1, 8);
	tail = string_copy(tail, endbuf-tail, ".");
	tail = fstring_to_string(tail, endbuf-tail, dp->type.type2, 4);
	tail = string_copy(tail, endbuf-tail, ".");
	tail = fstring_to_string(tail, endbuf-tail, dp->type.type3, 4);
	/* 空だった場合のハック */
	if (streq(buffer, "/..")) {
		tail = string_copy(buffer, endbuf-buffer, "/none");
	}

	tail = string_copy(tail, endbuf-tail, "/");

	/* --- base --- */
	minute_to_str(dp->base, tail, endbuf-tail);
	strcat(tail, "/");
	tail += strlen(tail);

	/* --- member --- */
	if (memcmp(dp->member, "    ", sizeof(dp->member)) == 0) {
		tail = string_copy(tail, endbuf-tail, "none");
	} else {
		tail = fstring_to_string(tail, endbuf-tail, dp->member,
			sizeof(dp->member));
	}
	tail = string_copy(tail, endbuf-tail, "/");

	/* --- valid1 --- */
	minute_to_str(dp->valid, tail, endbuf-tail);
	strcat(tail, "/");
	tail += strlen(tail);

	/* --- valid2 --- */
	minute_to_str(dp->valid2, tail, endbuf-tail);
	strcat(tail, "/");
	tail += strlen(tail);

	/* --- plane1 --- */
	tail = fstring_to_string(tail, endbuf-tail, dp->plane, 6); /*KH*/
	tail = string_copy(tail, endbuf-tail, "/");

	/* --- plane2 --- */
	tail = fstring_to_string(tail, endbuf-tail, dp->plane2, 6); /*KH*/
	tail = string_copy(tail, endbuf-tail, "/");

	/* --- element --- */
	tail = fstring_to_string(tail, endbuf-tail, dp->element, 6); /*KH*/

	return buffer;
}

/*パス文字列から nus_dims_t を作成する。
 * 返却値はコンポーネントを指すビットの論理和。
 */
	int
path_to_nusdims(const char *path, struct nus_dims_t *dp)
{
    char *head, *tail;
    char mypath[256];
    const char *type, *base, *member, *valid, *valid2, *plane, *plane2;

    nusdims_clear(dp);
    if (path == NULL)
	return 0;

    strncpy(mypath, path, sizeof mypath - 1);
    mypath[sizeof mypath - 1] = '\0';
    head = mypath;

#  ifdef SQUEEZE_SLASH
#    define SQUEEZE 	while (*head == '/') head++;
#  else
#    define SQUEEZE ;
#  endif

    /*--- 先頭要素のスキップ ---*/
    if (*head == '/') {
	head++;
    }
    SQUEEZE
    if (*head == '\0')
	return 0;
#ifdef TOPSKIP
    tail = strchr(head, '/');
    if (tail) {
	*tail++ = '\0';
        SQUEEZE
    }
    if (tail == NULL || *tail == '\0') {
	return 0;
    }

    /*--- type ---*/
    head = tail;
#endif
    tail = strchr(head, '/');
    if (tail) {
	*tail++ = '\0';
        SQUEEZE
    }
    type = head;
    split_type(type, &(dp->type));
    if (*type) {
	dp->stat |= NUSDIM_TYPE;
    }
    if (tail == NULL || *tail == '\0') {
	return dp->stat;
    }

    /*--- basetime ---*/
    head = tail;
    tail = strchr(head, '/');
    if (tail) {
	*tail++ = '\0';
        SQUEEZE
    }
    base = head;
    dp->base = str_to_minute(base);
    if (*base) {
	dp->stat |= NUSDIM_BASE;
    }
    if (tail == NULL || *tail == '\0') {
	return dp->stat;
    }

    /*--- member ---*/
    head = tail;
    tail = strchr(head, '/');
    if (tail) {
	*tail++ = '\0';
        SQUEEZE
    }
    member = head;
    string_to_fstring(dp->member, sizeof(dp->member), member);
    if (streq(head, "none")) {
	string_to_fstring(dp->member, sizeof(dp->member), "");
    }
    if (*member) {
	dp->stat |= NUSDIM_MEMBER;
    }
    if (tail == NULL || *tail == '\0') {
	return dp->stat;
    }

    /*--- validtime1 ---*/
    head = tail;
    tail = strchr(head, '/');
    if (tail) {
	*tail++ = '\0';
        SQUEEZE
    }
    /* 相対時刻サポートの必要あり */
    valid = head;
    dp->valid = str_to_minute(valid);
    if (*valid) {
	dp->stat |= NUSDIM_VALID;
    }
    if (tail == NULL || *tail == '\0') {
	return dp->stat;
    }

    /*--- validtime2 ---*/
    head = tail;
    tail = strchr(head, '/');
    if (tail) {
	*tail++ = '\0';
        SQUEEZE
    }
    /* 相対時刻サポートの必要あり */
    valid2 = head;
    if (*valid2) {
	dp->stat |= NUSDIM_VALID2;
    }
    if (*valid2 == '\0') {
        valid2 = valid;
    }
    dp->valid2 = str_to_minute(valid2);
    if (tail == NULL || *tail == '\0') {
	return dp->stat;
    }

    /*--- plane1 ---*/
    head = tail;
    tail = strchr(head, '/');
    if (tail) {
	*tail++ = '\0';
        SQUEEZE
    }
    plane = head;
    string_to_fstring(dp->plane, sizeof(dp->plane), plane);
    if (*plane) {
	dp->stat |= NUSDIM_PLANE;
    }
    if (tail == NULL || *tail == '\0') {
	return dp->stat;
    }

    /*--- plane2 ---*/
    head = tail;
    tail = strchr(head, '/');
    if (tail) {
	*tail++ = '\0';
        SQUEEZE
    }
    plane2 = head;
    if (*plane2 == '\0')
	plane2 = plane;
    string_to_fstring(dp->plane2, sizeof(dp->plane2), plane2);
    if (*plane2) {
	dp->stat |= NUSDIM_PLANE2;
    }
    if (tail == NULL || *tail == '\0') {
	return dp->stat;
    }

    /* --- element --- */
    string_to_fstring(dp->element, sizeof(dp->element), tail);
    if (*tail) {
        dp->stat |= NUSDIM_ELEMENT;
    }

    return dp->stat;
}

#ifdef TESTRUN

    int
main(int argc, char **argv)
{
	struct nus_dims_t dims;
	int	r;
	if (argv[1] == NULL) {
		return 100;
	} else {
		char	*path;
		r = path_to_nusdims(argv[1], &dims);
		path = nusdims_to_path(&dims);
		fprintf(stderr, "%d %s\n", r, path);
	}
	return r;
}

#endif
