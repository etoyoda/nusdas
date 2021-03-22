#!/usr/bin/ruby 

CFG = Struct.new(:atype, :ctype, :modfunc, :eqfunc,
	:use_hash, :use_sort, :flexible, :use_movehead)

settings = {
	"sys_ary8.c" => CFG.new("array8", "N_UI8", "ui8_mod_ui2", "ui8_eq",
		:use_hash, false, false, false),
	"sys_ary4.c" => CFG.new("array4", "N_UI4", "%", "==", 
		:use_hash, false, false, false),
	"sys_ary4v.c" => CFG.new("array4v", "N_UI4", "%", "==", 
		false, :use_sort, :flexible, false),
	"sys_aryp.c" => CFG.new("arrayp", "void *", nil, "==",
		false, :use_sort, :flexible, :use_movehead),
}

def operator(a, sym, b)
	if /^\w+$/ === sym
		"#{sym}(#{a}, #{b})"
	else
		"#{a} #{sym} #{b}"
	end
end

for fnam, c in settings
	fp = File.open(fnam, "w")
	template = File.open("sys_ary.txt")
	r = ""
	for line in template
		case line
		when /^%/ then
			r << $'
			r << <<END_OF_RUBY
fp.print <<EOL
#line #{template.lineno + 1} "sys_ary.txt"
EOL
END_OF_RUBY
		else r << <<END_OF_RUBY
fp.print <<EOL
#{line.chomp}
EOL
END_OF_RUBY
		end
	end
	template.close
	eval(r)
	fp.close
end
