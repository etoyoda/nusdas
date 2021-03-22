#!/usr/bin/ruby

def clist() Dir['*.c'] end
def hlist() Dir['*.h'] end
def olist() clist.collect {|file| file.sub(/\.c$/, '.o') } end
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
CFLAGS = 	-I../src -I../nwplib8 -I../libsrf @CFLAGS@ @CPPFLAGS@ $(CFlags)
AR =            @ARCHIVER@
ARFLAGS =	@ARFLAGS@
RUBY =          ruby
prefix =        @prefix@
exec_prefix =   @exec_prefix@
LIBDIR =        @libdir@
INCDIR =        @includedir@
INSTALLD =      mkdir -p
INSTALLF =      cp

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) -c $<

#{list "OBJS_N= ", olist.grep(/^n/)}
#{list "OBJS_S= ", olist.grep(/^s/)}
#{list "HDRS= ", hlist}
#{list "SRCS_N= ", clist.grep(/^n/)}
#{list "SRCS_S= ", clist.grep(/^s/)}
SRCS = $(SRCS_N)
LIBNUS = libnusdas.a
LIBNUSf = ../src/$(LIBNUS)
LIBNWP = libnwp.a
LIBSRFf = ../libsrf/libsrf.a

all: $(LIBNUSf) $(LIBSRFf)

$(LIBNUSf): $(OBJS_N)
	$(AR) $(ARFLAGS) $(LIBNUSf) $(OBJS_N)

$(LIBSRFf): $(OBJS_S)
	$(AR) $(ARFLAGS) $(LIBSRFf) $(OBJS_S)

echo-objs-nusdas:
	@echo OBJS=$(OBJS_N)

echo-objs-srf:
	@echo OBJS=$(OBJS_S)

tags: $(SRCS) $(HDRS)
	ctags *.c *.h

clean:
	-rm -f *~ *.bak *.o

distclean: clean
	-rm -f Makefile Testbin.mk config.h nusdas.h euc-jp shift_jis

WRAPTOOL = ../src/cwrap.rb ../src/cparse.rb  ../src/cptree.rb
WRAPGEN = $(RUBY) -I../src ../src/cwrap.rb

wrapper: wrap_n wrap_s

wrap_n: $(WRAPTOOL) ../src/nusdas.h
	$(WRAPGEN) -i nusdas.h -m s/nusdas_swab/endian_swab/i \
		-m s/nusdas_encode/n_encode/i \
		-m s/nusdas_decode/n_decode/i \
		-x NuSDaS_bf -x NuSDaS_snprintf \
		-o n@.c ../src/nusdas.h
wrap_s: $(WRAPTOOL) ../libsrf/libsrf.h
	$(WRAPGEN) -i libsrf.h --si4 @SI4_TYPE@ \
		-f gnu,ifc,win \
		-o s@.c ../libsrf/libsrf.h

$(SRCS_N): $(WRAPTOOL) ../src/nusdas.h
	touch $(SRCS_N)
$(SRCS_S): $(WRAPTOOL) ../libsrf/libsrf.h
	touch $(SRCS_S)

END_OF_MAKEFILE_IN

print "# following are made by \"gcc -MM -I../nwplib8 -I../src -I. *.c\"\n"
$stderr.print "gcc -MM -I../nwplib8 -I../src -I../libsrf ...\n"
print `gcc -MM -I../nwplib8 -I../src -I../libsrf #{clist.join(' ')}`
