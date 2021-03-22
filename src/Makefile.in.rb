#!/usr/bin/ruby

def clist() Dir['*.c'].sort end
def hlist() Dir['*.h'].sort end
def olist()
	ol = clist.collect {|file| file.sub(/\.c$/, '.o') }
	ol.sort!
	ol -= %w(pandora_lib.o)
	ol
end
def list head, args
	out = [head]
	for item in args
		if out.last.length > 64 then
			out.last << "\\\n"
			out << "    "
		end
		out.last << "#{item} "
	end
	out.join
end

$stdout = File.open("Makefile.in", "w")

print <<END_OF_MAKEFILE_IN
# edit Makefile.in.rb instead of Makefile or Makefile.in

CC =		@CC@ $(CCFlags)
CFLAGS = 	$(CFlags) -I../nwplib8
F90 =		@F90@
AR =            @ARCHIVER@
ARFLAGS =	@ARFLAGS@
prefix =        @prefix@
exec_prefix =   @exec_prefix@
LIBDIR =        @libdir@
INCDIR =        @includedir@
BINDIR =        @bindir@
INSTALLD =      mkdir -p
INSTALLF =      cp
RUBY = ruby

.SUFFIXES:
.SUFFIXES: .c .o .f

.c.o:
	$(CC) @CFLAGS@ @CPPFLAGS@ $(CFLAGS) -c $<

.f.o:
	$(F90) @FFLAGS@ $(FFLAGS) -c $<

#{list "OBJS_CORE= ", olist}
#{list "HDRS= ", hlist}
#{list "SRCS= ", clist}
OBJS_Fyes   = dummy.o
OBJS_Fno    =
OBJS_NETyes = pandora_lib.o
OBJS_NETno  =
OBJS        = $(OBJS_CORE) $(OBJS_NET@NET_AVAILABLE@) $(OBJS_F@F90_AVAILABLE@)
LIBNUS      = libnusdas.a

all: $(LIBNUS)

$(LIBNUS): $(OBJS)
	$(AR) $(ARFLAGS) $(LIBNUS) $(OBJS)

echo-objs-nusdas:
	@echo OBJS=$(OBJS)

LIBS_to_INSTALL = $(LIBNUS)
HEADERS_to_INSTALL = nusdas.h nusdas_fort.h

install: all
	test -d $(LIBDIR) || $(INSTALLD) $(LIBDIR)
	$(INSTALLF) $(LIBS_to_INSTALL) $(LIBDIR)/
	test -d $(INCDIR) || $(INSTALLD) $(INCDIR)
	$(INSTALLF) $(HEADERS_to_INSTALL) $(INCDIR)/
	test -d $(BINDIR) || $(INSTALLD) $(BINDIR)
	for exe in nusdas-config ; do \
		$(INSTALLF) $$exe $(BINDIR)/ ;\
		chmod ugo+rx $(BINDIR)/$$exe ;\
	done

refresh:
	$(MAKE) -f Refresh.mk

doc-c-ref: cparse.rb
	$(RUBY) cprodoc.rb -a nusdas.h \
	-m s/nusdas_swab/endian_swab/i \
	-m s/nusdas_encode/n_encode/i \
	-m s/nusdas_decode/n_decode/i \
	-m s/nusdas_bf/bf/i \
	-m downcase \
	-m s/_i1/_I1/ \
	-o ../doc/capi_@.tex api_*.c

doc-f-ref: cparse.rb
	$(RUBY) cprodoc.rb -a nusdas.h --lang Fortran \
	-x NuSDaS_bf -x NuSDaS_snprintf \
	-m s/nusdas_encode/n_encode/i \
	-m s/nusdas_decode/n_decode/i \
	-m s/nusdas_swab/endian_swab/i -m upcase \
	-o ../doc/fapi_@.tex api_*.c

doc: ../html/index.html

cparse.rb:
	racc -o cparse.rb cparse.y

../html/index.html: $(SRCS) $(HDRS)
	doxygen

charset: @CHARSET@

decharset: euc-jp

shift_jis: $(SRCS) $(HDRS) Pbfmaker.rb ../InstNaps8.rb
	if [ ! -f shift_jis ] ;\\
	then \\
		for file in $(SRCS) $(HDRS) Pbfmaker.rb ../InstNaps8.rb ;\\
		do \\
			@EUCTOSJIS@ < $$file > xconv.tmp && mv -f xconv.tmp $$file ;\\
		done; \\
	fi
	-rm -f euc-jp utf8
	touch shift_jis

euc-jp: $(SRCS) $(HDRS) Pbfmaker.rb ../InstNaps8.rb
	if [ ! -f euc-jp ] ;\\
	then \\
		for file in $(SRCS) $(HDRS) Pbfmaker.rb ../InstNaps8.rb ;\\
		do \\
			@SJISTOEUC@ < $$file > xconv.tmp && mv -f xconv.tmp $$file ;\\
		done; \\
	fi
	-rm -f shift_jis utf8
	touch euc-jp

utf8: $(SRCS) $(HDRS) Pbfmaker.rb ../InstNaps8.rb
	if [ ! -f utf8 ] ;\\
	then \\
		for file in $(SRCS) $(HDRS) Pbfmaker.rb ../InstNaps8.rb ;\\
		do \\
			nkf -w < $$file > xconv.tmp && mv -f xconv.tmp $$file ;\\
		done; \\
	fi
	-rm -f shift_jis euc-jp
	touch utf8

tags: $(SRCS) $(HDRS)
	ctags *.c *.h

clean: decharset
	-rm -f *~ *.bak *.o $(LIBNUS)

distclean: clean
	-rm -f Makefile Testbin.mk config.h nusdas.h nusdas-config utf8 euc-jp shift_jis

END_OF_MAKEFILE_IN

print "# following are made by \"gcc -MM -I../nwplib8 *.c\"\n"
$stderr.print "gcc -MM -I../nwplib8 ...\n"
print `gcc -MM -I../nwplib8 #{clist.join(' ')}`
