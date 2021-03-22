#!/usr/bin/make -f
RUBY =          ruby
FLEX =          flex

all: def_phase.h def_phase.smb sys_kwd.h ndf_codec.c ndf_rlen.c dds_tmpl.c \
	sys_ary4.c sys_ary4v.c sys_ary8.c sys_aryp.c glb_typetab.c \
	dds_dftab.c ndf_auxtab.c pds_cfgscan.c cparse.rb


def_phase.h: def_phase.rb
	$(RUBY) def_phase.rb
def_phase.smb: def_phase.rb
	$(RUBY) def_phase.rb
sys_kwd.h: sys_kwd.rb def_phase.smb ndf_codec.smb hardcode.smb
	$(RUBY) sys_kwd.rb def_phase.smb ndf_codec.smb hardcode.smb
ndf_codec.c: ndf_codec.rb
	$(RUBY) ndf_codec.rb
ndf_codec.smb: ndf_codec.rb
	$(RUBY) ndf_codec.rb
ndf_rlen.c: ndf_rlen.rb
	$(RUBY) ndf_rlen.rb
dds_tmpl.c: dds_tmpl.rb
	$(RUBY) dds_tmpl.rb
sys_ary4.c: sys_ary.rb sys_ary.txt
	$(RUBY) sys_ary.rb
sys_ary4v.c: sys_ary.rb sys_ary.txt
	$(RUBY) sys_ary.rb
sys_ary8.c: sys_ary.rb sys_ary.txt
	$(RUBY) sys_ary.rb
sys_aryp.c: sys_ary.rb sys_ary.txt
	$(RUBY) sys_ary.rb
glb_typetab.c: sys_hash.rb sys_hash.txt
	$(RUBY) sys_hash.rb
dds_dftab.c: sys_hash.rb sys_hash.txt
	$(RUBY) sys_hash.rb
ndf_auxtab.c: sys_hash.rb sys_hash.txt
	$(RUBY) sys_hash.rb
pds_cfgscan.c: pds_cfgscan.l flex.skl
	$(FLEX) -Sflex.skl -opds_cfgscan.c pds_cfgscan.l

cparse.rb: cparse.y
	racc -o cparse.rb cparse.y
