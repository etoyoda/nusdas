all: nusdas.pdf

TEXS = api2.tex capi.tex capi_endian_swab4.tex capi_nusdas_allfile_close.tex \
       capi_nusdas_cut.tex capi_nusdas_cut2.tex capi_nusdas_cut2_raw.tex \
       capi_nusdas_cut_raw.tex capi_nusdas_esf_flush.tex \
       capi_nusdas_grid.tex capi_nusdas_grid2.tex capi_nusdas_info.tex \
       capi_nusdas_info2.tex capi_nusdas_inq_cntl.tex \
       capi_nusdas_inq_cntl2.tex capi_nusdas_inq_data.tex \
       capi_nusdas_inq_data2.tex capi_nusdas_inq_def.tex \
       capi_nusdas_inq_nrdbtime.tex capi_nusdas_inq_nrdvtime.tex \
       capi_nusdas_iocntl.tex capi_nusdas_make_mask.tex \
       capi_nusdas_onefile_close.tex capi_nusdas_onefile_close2.tex \
       capi_nusdas_parameter_change.tex capi_nusdas_parameter_reset.tex \
       capi_nusdas_read.tex capi_nusdas_read2.tex \
       capi_nusdas_read2_raw.tex capi_nusdas_read_3d.tex \
       capi_nusdas_subc_delt.tex capi_nusdas_subc_delt2.tex \
       capi_nusdas_subc_delt_preset1.tex capi_nusdas_subc_eta.tex \
       capi_nusdas_subc_eta2.tex capi_nusdas_subc_eta_inq_nz.tex \
       capi_nusdas_subc_eta_inq_nz2.tex capi_nusdas_subc_preset1.tex \
       capi_nusdas_subc_rgau.tex capi_nusdas_subc_rgau2.tex \
       capi_nusdas_subc_rgau_inq_jn.tex \
       capi_nusdas_subc_rgau_inq_jn2.tex \
       capi_nusdas_subc_rgau_preset1.tex capi_nusdas_subc_sigm.tex \
       capi_nusdas_subc_sigm2.tex capi_nusdas_subc_srf.tex \
       capi_nusdas_subc_srf2.tex capi_nusdas_subc_srf_ship.tex \
       capi_nusdas_subc_srf_ship2.tex capi_nusdas_subc_tdif.tex \
       capi_nusdas_subc_tdif2.tex capi_nusdas_subc_zhyb.tex \
       capi_nusdas_subc_zhyb2.tex capi_nusdas_subc_zhyb_preset1.tex \
       capi_nusdas_write.tex capi_nusdas_write2.tex \
       capi_nusdas_write_3d.tex changes.tex fapi.tex \
       fapi_endian_swab4.tex fapi_nusdas_allfile_close.tex \
       fapi_nusdas_cut.tex fapi_nusdas_cut2.tex fapi_nusdas_cut2_raw.tex \
       fapi_nusdas_cut_raw.tex fapi_nusdas_esf_flush.tex \
       fapi_nusdas_grid.tex fapi_nusdas_grid2.tex fapi_nusdas_info.tex \
       fapi_nusdas_info2.tex fapi_nusdas_inq_cntl.tex \
       fapi_nusdas_inq_cntl2.tex fapi_nusdas_inq_data.tex \
       fapi_nusdas_inq_data2.tex fapi_nusdas_inq_def.tex \
       fapi_nusdas_inq_nrdbtime.tex fapi_nusdas_inq_nrdvtime.tex \
       fapi_nusdas_iocntl.tex fapi_nusdas_make_mask.tex \
       fapi_nusdas_onefile_close.tex fapi_nusdas_onefile_close2.tex \
       fapi_nusdas_parameter_change.tex fapi_nusdas_parameter_reset.tex \
       fapi_nusdas_read.tex fapi_nusdas_read2.tex \
       fapi_nusdas_read2_raw.tex fapi_nusdas_read_3d.tex \
       fapi_nusdas_subc_delt.tex fapi_nusdas_subc_delt2.tex \
       fapi_nusdas_subc_delt_preset1.tex fapi_nusdas_subc_eta.tex \
       fapi_nusdas_subc_eta2.tex fapi_nusdas_subc_eta_inq_nz.tex \
       fapi_nusdas_subc_eta_inq_nz2.tex fapi_nusdas_subc_preset1.tex \
       fapi_nusdas_subc_rgau.tex fapi_nusdas_subc_rgau2.tex \
       fapi_nusdas_subc_rgau_inq_jn.tex \
       fapi_nusdas_subc_rgau_inq_jn2.tex \
       fapi_nusdas_subc_rgau_preset1.tex fapi_nusdas_subc_sigm.tex \
       fapi_nusdas_subc_sigm2.tex fapi_nusdas_subc_srf.tex \
       fapi_nusdas_subc_srf2.tex fapi_nusdas_subc_srf_ship.tex \
       fapi_nusdas_subc_srf_ship2.tex fapi_nusdas_subc_tdif.tex \
       fapi_nusdas_subc_tdif2.tex fapi_nusdas_subc_zhyb.tex \
       fapi_nusdas_subc_zhyb2.tex fapi_nusdas_subc_zhyb_preset1.tex \
       fapi_nusdas_write.tex fapi_nusdas_write2.tex \
       fapi_nusdas_write_3d.tex general.tex nusdas.tex nusdas_net.tex \
       pandora_driver_for_nusdas.tex pandora_driver_lib.tex \
       pandora_lib.tex \
capi_nusdas_scan_ds.tex fapi_nusdas_scan_ds.tex \
capi_n_decode_rlen_nbit_i1.tex capi_n_encode_rlen_8bit.tex \
capi_n_encode_rlen_8bit_i1.tex fapi_n_decode_rlen_nbit_i1.tex \
fapi_n_encode_rlen_8bit.tex fapi_n_encode_rlen_8bit_i1.tex \
capi_bfclose.tex capi_bfgetpos.tex capi_bfopen.tex capi_bfread.tex capi_bfread_native.tex capi_bfseek.tex capi_bfsetpos.tex capi_bfwrite.tex capi_bfwrite_native.tex \
capi_nusdas_snprintf.tex \
capi_nusdas_inq_subcinfo.tex fapi_nusdas_inq_subcinfo.tex \
capi_nusdas_inq_parameter.tex fapi_nusdas_inq_parameter.tex \
capi_rdr_lv_trans.tex capi_srf_amd_aqc.tex capi_srf_amd_rdic.tex capi_srf_amd_slct.tex capi_srf_lv_set.tex capi_srf_lv_trans.tex capi_srf_rd_rdic.tex capi_srf_search_amdstn.tex \
capi_endian_swab_fmt.tex fapi_endian_swab_fmt.tex \
capi_nusdas_gunzip.tex capi_nusdas_gunzip_nbytes.tex capi_nusdas_gzip.tex fapi_nusdas_gunzip.tex fapi_nusdas_gunzip_nbytes.tex fapi_nusdas_gzip.tex \
capi_nusdas_unpack.tex fapi_nusdas_unpack.tex \
capi_nusdas_uncpsd.tex fapi_nusdas_uncpsd.tex \
capi_nusdas_uncpsd_nbytes.tex fapi_nusdas_uncpsd_nbytes.tex \
	readers.tex writers.tex \
	usage.tex deffile.tex \
	datamodel.tex \
	nuselems.tex \
       tables.tex format.tex proj.tex opts.tex nusref.bib

nusdas.pdf: nusdas.dvi
	dvipdfmx -d 5 nusdas

nusdas.dvi: $(TEXS) nusdas.bbl
	for i in 1 2 3 ;\
	do \
		platex nusdas ;\
		grep -q Rerun nusdas.log || break ;\
	done

nusdas.bbl: nusref.bib
	platex nusdas
	pbibtex nusdas # update: jbibtex to pbibtex (NAPS10:20180830)

nuselems.tex: nuselems.csv nuselems.rb
	ruby nuselems.rb nuselems.csv > nuselems.tex

clean:
	-rm -f *.dvi *.pdf *.aux *.idx *.toc *.bak *~ *.log *.out *.blg

sjis:
	for tex in *.tex *.bib *.bbl; do echo -n "$${tex} "; nkf -s $${tex} > z.tmp && mv z.tmp $${tex}; done
	:

euc:
	for tex in *.tex *.bib *.bbl; do echo -n "$${tex} "; nkf -e $${tex} > z.tmp && mv z.tmp $${tex}; done
	:

utf8:
	for tex in *.tex *.bib *.bbl; do echo -n "$${tex} "; nkf -w $${tex} > z.tmp && mv z.tmp $${tex}; done
	:
