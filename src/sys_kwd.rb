#!/usr/bin/ruby

sym8 = {}
sym4 = {}

for line in ARGF
	word = line.chomp.sub(/^\s+/, '').sub(/\s+$/, '')

	shortp = word.sub!(/^-/, '')

	binary = [word.upcase].pack('A8')
	up = word.upcase[0..7].gsub(/ /, "_")
	be = format('%08X%08X', *binary.unpack('NN'))
	le = format('%08X%08X', *binary.unpack('VV').reverse)
	defbe = "#  define\tSYM8_#{up}\t0x#{be}uLL\n"
	defle = "#  define\tSYM8_#{up}\t0x#{le}uLL\n"
	xstr = binary.dump
	def32 = "#  define\tSYM8_#{up}\t(*(N_SI8 *)#{xstr})\n"
	sym8[up] = [defbe, defle, def32] unless shortp

	binary = [word.upcase].pack('A4')
	up = word.upcase[0..3]
	be = format('%08X', *binary.unpack('N'))
	le = format('%08X', *binary.unpack('V').reverse)
	defbe = "#  define\tSYM4_#{up}\t0x#{be}uL\n"
	defle = "#  define\tSYM4_#{up}\t0x#{le}uL\n"
	sym4[up] = [defbe, defle]

end

# special
sym4['ALLSPACE'] = ["#  define\tSYM4_ALLSPACE\t0x20202020uL\n"] * 2
sym8['ALLSPACE'] = ["#  define\tSYM8_ALLSPACE\t0x2020202020202020uLL\n"] * 2 \
	+ ["#  define\tSYM8_ALLSPACE\tsym8_fillspace()\n"]
#

header = File.open("sys_kwd.h", "w")
header.print <<EOF
/** @file
 * @brief [auto-generated by sys_kwd.rb] predefined symbol constants.
 *
 * Do not edit this file.
 * Edit sys_kwd.rb instead.
 */
#ifndef SYS_KWD_H
#define SYS_KWD_H

#ifndef HAVE_SI8_TYPE
# error "please include nusdas.h before sys_kwd.h"
#endif

#if !(HAVE_SI8_TYPE)
INLINE
sym8_t
sym8_fillspace(void)
{
  sym8_t buf;
  char *ptr = (char *)&buf;
  ptr[0] = ptr[1] = ptr[2] = ptr[3] = ' ';
  ptr[4] = ptr[5] = ptr[6] = ptr[7] = ' ';
  return buf;
}
#endif

#ifdef WORDS_BIGENDIAN
# if HAVE_SI8_TYPE
EOF

for up in sym8.keys.sort
	header.print sym8[up][0]
end

header.print "# endif\n"

for up in sym4.keys.sort
	header.print sym4[up][0]
end

header.print "#else\n"
header.print "# if HAVE_SI8_TYPE\n"

for up in sym8.keys.sort
	header.print sym8[up][1]
end

header.print "# endif\n"

for up in sym4.keys.sort
	header.print sym4[up][1]
end

header.print "#endif /* NUS_IS_BIG_ENDIAN */\n"
header.print "#if !(HAVE_SI8_TYPE)\n"

for up in sym8.keys.sort
	header.print sym8[up][2]
end

header.print "#endif /* ! HAVE_SI8_TYPE */\n"
header.print "#endif /* SYS_KWD_H */\n"
header.close
