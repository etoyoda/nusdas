/** @file 
 * @brief DDS データセットの探索
 */
#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h> /* for getenv */
#include <dirent.h> /* for opendir */
# define NEED_STRING_COPY
#include "sys_string.h"
#include "glb.h"
#include "dset.h"


/** @brief NRD の探索
 *
 * NRD を探索し、その中にある全てのデータセット (DDS) を
 * 開き、これらをすべて nusglb_pushdset() によって登録する。
 */
	int
nusdds_scan(void)
{
	char	nusdasnn[256];
	char	*nrdname;
	char	dirname[256];
	char	deffile[256];
	char	*basename;
	int		nrd;
	char	nrdlist[100];
	DIR	*dp;
	struct dirent	*entp;
	/* カレントディレクトリを開きNUSDAS??をnrdlistに登録 */
	memset(nrdlist, 0x00, sizeof(char) * 100);
	dp = opendir(".");
	if (dp != NULL){
		while ((entp = readdir(dp)) != NULL) {
			size_t leaflen;
			if (entp->d_name[0] == '.')
				continue;
			leaflen = strlen(entp->d_name);
			if (leaflen != 8)
				continue;
			if (memcmp(entp->d_name, "NUSDAS", 6)) {
				continue;
			}
			nrd = atoi(entp->d_name + 6);
			if (nrd > 0 && nrd < 100)
				nrdlist[nrd]++;
		}
	}
	closedir(dp);
	
	for (nrd = 1; nrd <= 99; nrd++) {
		/* ディレクトリを開く */
		nusdas_snprintf(nusdasnn, sizeof nusdasnn, "NUSDAS%02u", nrd);
		if ((nrdname = getenv(nusdasnn)) == NULL) {
			if(nrdlist[nrd] == 0)
				continue;
			nrdname = nusdasnn;
		}
		nusdas_snprintf(dirname, sizeof dirname,
			"%s/nusdas_def", nrdname);
		dp = opendir(dirname);
		if (dp == NULL) {
			/* ディレクトリが存在しないことはエラーではない */
			continue;
		}
		strcpy(deffile, dirname);
		basename = deffile + strlen(deffile);
		*basename++ = '/';
		while ((entp = readdir(dp)) != NULL) {
			nusdset_t *ds;
			size_t leaflen;
			if (entp->d_name[0] == '.')
				continue;
			leaflen = strlen(entp->d_name);
			if (leaflen < 4)
				continue;
			if (memcmp(entp->d_name + leaflen - 4, ".def", 4)
			 && memcmp(entp->d_name + leaflen - 4, ".DEF", 4)) {
				continue;
			}
			string_copy(basename, entp->d_name,
					deffile + sizeof(deffile) - basename);
			ds = dds_open(nrdname, deffile, nrd);
			if (ds == NULL) {
				/* 1つの定義ファイルにエラーがあっても
				 * メッセージは出すが探索は続行する
				 */
				continue;
			}
			nusglb_pushdset(ds, nrd);
		}
		closedir(dp);
	}
	return 0;
}
