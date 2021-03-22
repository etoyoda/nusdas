struct nsd_subc_s {
        int     nsd;
        int     bt;             /* basetime */
        char   *types;
    };

int proc_subc(struct nsd_subc_s *, struct nsd_subc_s *, char *, int, char *, char *, char *);
