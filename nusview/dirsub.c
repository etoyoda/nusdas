/* dirsub.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef USE_NWPLIB8
#include "nwpl_capi.h"
#else
#include <nwpl_timecnv.h>
#endif
#include "stringplus.h"
#include <nusdas.h>
#include "nusdir.h"
#include "textout.h"

/*
 * === NuSDaS Version 2 を用いる新ルーチン ===
 */

#define MAXENTRYSTR 2048

typedef struct entry {
	char str[MAXENTRYSTR];
	struct entry *left;
	struct entry *right;
} nusdim_entry;

nusdim_entry *entry_top = NULL;

	static nusdim_entry *
add_entry(nusdim_entry *p, char *str) {
	int cond;

	if (p == NULL) {
		p = (nusdim_entry *)malloc(sizeof(nusdim_entry));
		if (p == NULL)
			return NULL;
		strcpy(p->str, str);
		p->left = p->right = NULL;
	} else if ((cond = strcmp(str, p->str)) < 0)
		p->left  = add_entry(p->left,  str);
	else   if (cond > 0)
		p->right = add_entry(p->right, str);
	return p;
}
	void
flush_entries(nusdim_entry *p) {
	if (p != NULL) {
		flush_entries(p->left);
		tprintf(NULL, "%s\n", p->str);
		flush_entries(p->right);
	}
}

#if 0
	int
nusdir_generic(struct nus_dims_t *dp, struct nusdir_opts *op)
{
	int	r;
	r = nusdas_list_dims(dp, (void *)op, iprint, cprint, op->debug);
	flush_entries(entry_top);
	return r;
}
#endif

/*
 * === NuSDaS Version 1 を用いる旧ルーチン ===
 */

#define MAX_BASETIMES 100000

	int
nusdir_base(struct nus_dims_t *dp, struct nusdir_opts *op)
{
	struct nus_type_t	*sp;
	int	i, r;
	N_SI4	siz;
	N_SI4	*longbuf;

	sp = &dp->type;

	longbuf = malloc(MAX_BASETIMES * sizeof(N_SI4));
	if (longbuf == NULL)
		return 1;
	siz = MAX_BASETIMES;
	r = nusdas_inq_nrdbtime(sp->type1, sp->type2, sp->type3, longbuf,
		&siz, (op->longe ? N_ON : N_OFF));
	if (r < 0) {
		eprintf(HTTP_ERR, "nusdas_inq_nrdbtime %d\n", r);
		return r;
	}

	for (i = 0; i < r; i++) {
		const char *p = minute_to_str(longbuf[i], NULL, 0);
		if (p == NULL)
			continue;
		if (strncmp(p, "min", 3) == 0)
			tprintf(0, "%s\n", p);
		else if (longbuf[i] < 31536000)
			errprintf("basetime %d\n", longbuf[i]);
		else
			tprintf(0, "%s\n", p);
	}
	free(longbuf);
	
	return 0;
}

	int
nusdir_member(struct nus_dims_t *dp,
		struct nusdir_opts *op __attribute__((unused)))
{
	struct nus_type_t	*sp;
	int	i, r;
	N_SI4	siz, n;
	char	*charbuf;

	sp = &dp->type;

	siz = 1;
	r = nusdas_inq_def(sp->type1, sp->type2, sp->type3, N_MEMBER_NUM,
		&n, &siz);
	if (r < 0) return r;
	charbuf = malloc(n * 4);
	r = nusdas_inq_def(sp->type1, sp->type2, sp->type3, N_MEMBER_LIST,
		charbuf, &n);
	if (r < 0) return r;

	for (i = 0; i < n; i++) {
		char *p;
		p = charbuf + (4 * i);
		if (strncmp(p, "    ", 4) == 0) p = "none";
		tprintf(0, "%.4s\n", p);
	}
	free(charbuf);

	return 0;
}

	int
nusdir_valid(struct nus_dims_t *dp, struct nusdir_opts *op)
{
	struct nus_type_t	*sp;
	int	i, r;
	N_SI4	siz;
	N_SI4	*longbuf;

	sp = &dp->type;
	longbuf = malloc(MAX_BASETIMES * sizeof(N_SI4));
	if (longbuf == NULL)
		return 1;
	siz = MAX_BASETIMES;
	r = nusdas_inq_nrdvtime(sp->type1, sp->type2, sp->type3, longbuf,
		&siz, &dp->base, (op->debug ? N_ON : N_OFF));
	if (r < 0) {
		eprintf(HTTP_ERR, "nusdas_inq_nrdvtime %d\n", r);
		return r;
	}

	for (i = 0; i < r; i++) {
		const char *p = minute_to_str(longbuf[i], NULL, 0);
		if (p == NULL)
			continue;
		if (strncmp(p, "min", 3) == 0)
			tprintf(0, "%s\n", p);
		else if (longbuf[i] < 31536000)
			errprintf("validtime %d\n", longbuf[i]);
		else
			tprintf(0, "%s\n", p);
	}
	free(longbuf);

	return 0;
}

	int
nusdir_valid2(struct nus_dims_t *dp __attribute__((unused)),
		struct nusdir_opts *op __attribute__((unused)))
{
	tprintf(0, "min1\n");
	return 0;
}

	int
nusdir_plane(struct nus_dims_t *dp,
		struct nusdir_opts *op __attribute__((unused)))
{
	struct nus_type_t	*s;
	int	i, r;
	N_SI4	siz, n;
	char	*charbuf;

	s = &dp->type;
	siz = 1;
	r = nusdas_inq_cntl(s->type1, s->type2, s->type3, &(dp->base),
		dp->member, &(dp->valid), N_PLANE_NUM, &n, &siz);
	if (r < 0)
		return r;
	charbuf = malloc(n * 6);
	r = nusdas_inq_cntl(s->type1, s->type2, s->type3, &(dp->base),
		dp->member, &(dp->valid), N_PLANE_LIST, charbuf, &n);
	if (r < 0)
		return r;

	for (i = 0; i < n; i++) {
		char *p;
		p = charbuf + (6 * i);
		if (strncmp(p, "      ", 6) == 0) p = "none";
		tprintf(0, "%.6s\n", p);
	}
	free(charbuf);
	return 0;
}

	int
nusdir_plane2(struct nus_dims_t *dp,
		struct nusdir_opts *op __attribute__((unused)))
{
	struct nus_type_t	*s;
	int	i, r;
	N_SI4	siz, n;
	char	*charbuf;

	if (memcmp(dp->plane, "      ", 6) == 0) {
		s = &dp->type;
		siz = 1;
		r = nusdas_inq_cntl2(s->type1, s->type2, s->type3, &(dp->base),
			dp->member, &(dp->valid), &(dp->valid2), N_PLANE_NUM, 
			&n, &siz);
		if (r < 0)
			return r;
		charbuf = malloc(n * 6);
		r = nusdas_inq_cntl2(s->type1, s->type2, s->type3, &(dp->base),
			dp->member, &(dp->valid), &(dp->valid2), N_PLANE2_LIST,
			 charbuf, &n);
		if (r < 0)
			return r;

		for (i = 0; i < n; i++) {
			char *p;
			p = charbuf + (6 * i);
			if (strncmp(p, "      ", 6) == 0) p = "none";
			tprintf(0, "%.6s\n", p);
		}
		free(charbuf);
	} else {
		tprintf(0, "%.6s\n", dp->plane);
	}
	return 0;
}

	int
nusdir_element(struct nus_dims_t *dp,
		 struct nusdir_opts *op __attribute__((unused)))
{
	struct nus_type_t	*s;
	int		i, r;
	N_SI4		siz, nelem, nmember, nvalid, nplane;
	N_SI4           imember, ivalid, iplane;
	char		*elembuf, *mapbuf, *charbuf;
	N_SI4		*longbuf;
        int             btime;


	s = &(dp->type);
	siz = 1;
	r = nusdas_inq_def(s->type1, s->type2, s->type3,
		N_ELEMENT_NUM, &nelem, &siz);
	if (r < 1) {
	  eprintf(HTTP_ERR, "nusdas_inq_def: element_num: %d\n", r);
	  return r;
	}

	r = nusdas_inq_def(s->type1, s->type2, s->type3, N_PLANE_NUM,
		&nplane, &siz);
	if (r < 1) {
	  eprintf(HTTP_ERR, "nusdas_inq_def: plane_num: %d\n", r);
	  return r;
        }	
	if ((charbuf = malloc(nplane * 6)) == NULL)
		return -2;
	r = nusdas_inq_def(s->type1, s->type2, s->type3, N_PLANE_LIST,
		charbuf, &nplane);
	iplane = -1;
	for (i = 0; i < nplane; i++) {
	    if (memcmp(charbuf + 6 * i, dp->plane, 6) == 0) {
		iplane = i;
		break;
	    }
	}
	free(charbuf);
	if (iplane == -1) {
	    eprintf(HTTP_ERR, "plane %.6s not found\n", dp->plane);
	    return -3;
	}

	r = nusdas_inq_def(s->type1, s->type2, s->type3, N_MEMBER_NUM,
		&nmember, &siz);
	if (r < 1) {
	  eprintf(HTTP_ERR, "nusdas_inq_def: member_num: %d\n", r);
	  return r;
	}
	if ((charbuf = malloc(nmember * 4)) == NULL)
		return -2;
	r = nusdas_inq_def(s->type1, s->type2, s->type3, N_MEMBER_LIST,
		charbuf, &nmember);
	imember = -1;
	for (i = 0; i < nmember; i++) {
	    if (memcmp(charbuf + 4 * i, dp->member, 4) == 0) {
		imember = i;
		break;
	    }
	}
	free(charbuf);
	if (imember == -1) {
	    eprintf(HTTP_ERR, "member %.4s not found\n", dp->member);
	    return -3;
	}
#if 0
        if(dp->base == -1){ /* when wild card basetime */
            N_SI4 *btlist, *vtlist;
            nvalid = MAX_BASETIMES; 
            /* dirty sigsegv guard + 256 */
            if ((btlist = malloc(nvalid * 4 + 256)) == NULL)
                return -2;
            if ((vtlist = malloc(nvalid * 4 + 256)) == NULL)
                return -2;
            r = nusdas_inq_nrdvtime2(s->type1, s->type2, s->type3,
                                     btlist, vtlist, &nvalid, 
                                     &(dp->base), N_OFF);
            if (r < 1) {
                eprintf(HTTP_ERR, "nusdas_inq_nrdvtime: %d\n", r);
                return r;
            }
            nvalid = r;
            btime = -1;
            for (i = 0; i < nvalid; i++) {
                if (vtlist[i] == dp->valid) {
                    btime = btlist[i];
                    break;
                }
            }
            
            if (btime == -1) {
                eprintf(HTTP_ERR, "validtime2 %d not found\n", btime);
            }
            free(btlist);
            free(vtlist);
        }
        else{
            btime = dp->base;
        }
#endif
        btime = dp->base;

	r = nusdas_inq_def(s->type1, s->type2, s->type3,
	    N_VALIDTIME_NUM, &nvalid, &siz);
	if (r < 1) {
	  eprintf(HTTP_ERR, "nusdas_inq_def: validtime_num: %d\n", r);
	  return r;
	}
	/* dirty sigsegv guard + 256 */
	if ((longbuf = malloc(nvalid * 4 + 256)) == NULL)
		return -2;
        /*
	r = nusdas_inq_nrdvtime(s->type1, s->type2, s->type3,
	    longbuf, &nvalid, (N_SI4*)&btime, N_OFF);
        */

        r = nusdas_inq_cntl(s->type1, s->type2, s->type3,
                            &(dp->base), dp->member, &(dp->valid),
                            N_VALIDTIME_LIST, longbuf, &nvalid);

	if (r < 1) {
	  eprintf(HTTP_ERR, "nusdas_inq_nrdvtime: %d\n", r);
	  return r;
	}
	ivalid = -1;
	for (i = 0; i < nvalid; i++) {
	    if (longbuf[i] == dp->valid) {
		ivalid = i;
		break;
	    }
	}
	free(longbuf);
	if (ivalid == -1) {
	    eprintf(HTTP_ERR, "validtime %d not found\n", dp->valid);
            return r;
	}

	siz = nelem * nplane * nmember * nvalid;
	mapbuf = malloc(siz);
	if (mapbuf == NULL) return 1;
	r = nusdas_inq_def(s->type1, s->type2, s->type3, N_ELEMENT_MAP,
		mapbuf, &siz);
	if (r < 0) {
	  eprintf(HTTP_ERR, "nusdas_inq_def: n_element_map: %d\n", r);
	  return r;
	}

	elembuf = malloc(nelem * 6 + 1);
	if (elembuf == NULL) return 1;
	memset(elembuf, 0, nelem * 6 + 1);
	r = nusdas_inq_cntl(s->type1, s->type2, s->type3,
		&(dp->base), dp->member, &(dp->valid),
		N_ELEMENT_LIST, elembuf, &nelem);
	if (r < 0) {
	  eprintf(HTTP_ERR, "nusdas_inq_cntl: element_list: %d\n", r);
	  return r;
	} 
	elembuf[nelem * 6] = '\0';

	for (i = 0; i < nelem; i++) {
		char *p, *q;
		q = mapbuf + i + nelem * (iplane + nplane *
			(ivalid + nvalid * imember));
		if (!*q) continue;
		p = elembuf + (6 * i);
		if (strncmp(p, "      ", 6) == 0) p = "none";
		tprintf(0, "%.6s\n", p);
	}
	free(elembuf);
	free(mapbuf);
	return 0;
}
