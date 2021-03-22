#!/usr/bin/ruby
#
# sys_int.exe の出力を検証するスクリプト

def si8_add(words)
	a = Integer("0x" + words[0])
	b = Integer("0x" + words[1])
	c = Integer("0x" + words[2])
	c2 = (a + b) % (1 << 64)
	if c != c2
		print "#{a} + #{b} => #{c} (#{c2} expected)\n"
	end
end

def si8_sub(words)
	a = Integer("0x" + words[0])
	b = Integer("0x" + words[1])
	c = Integer("0x" + words[2])
	c2 = (a - b) % (1 << 64)
	if c != c2
		print "#{a} - #{b} => #{c} (#{c2} expected)\n"
	end
end

def ui8_mod_ui2(words)
	a = Integer("0x" + words[0])
	b = Integer("0x" + words[1])
	c = Integer("0x" + words[2])
	c2 = a % b
	if c != c2
		print "#{a} % #{b} => #{c} (#{c2} expected)\n"
	end
end

def ui8_div_ui2(words)
	a = Integer("0x" + words[0])
	b = Integer("0x" + words[1])
	c = Integer("0x" + words[2])
	c2 = a / b
	if c != c2
		print "#{a} / #{b} => #{c} (#{c2} expected)\n"
	end
end

for line in ARGF
	line.chomp!
	words = line.split(/\s+/)
	first = words.shift
	case first
	when /^si8_add$/ then si8_add(words);
	when /^si8_sub$/ then si8_sub(words);
	when /^ui8_mod_ui2$/ then ui8_mod_ui2(words);
	when /^ui8_div_ui2$/ then ui8_div_ui2(words);
	else raise "unknown line <#{line}>"
	end
end
