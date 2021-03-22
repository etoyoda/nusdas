#!/usr/bin/ruby 

CFG = Struct.new(:htype, :ktype, :vtype,
	:ktype_eq, :use_each, :use_reject,
	:hdrs,
	:kptr, :ktype_dup)

settings = {
	"glb_typetab.c" => CFG.new("glb_typetab", "nustype_t",
		"struct nustype_dstab",
		"nustype_p_eq", false, false,
		[],
		"", "*"),
	"dds_dftab.c" => CFG.new("dds_dftab", "char",
		"union nusdfile_t",
		"!strcmp", true, :use_reject,
		%w(string.h sys_string.h dset.h),
		"*", "string_dup"),
	"ndf_auxtab.c" => CFG.new("ndf_auxtab", "sym4_t",
		"struct ndf_aux_t",
		"nusndf_aux_eq", :use_each, false,
		%w(dfile.h ndf.h),
		"", "*"),
	"pds_cntltab.c" => CFG.new("pds_cntltab", "char",
		"char",
		"!strcmp", false, false,
		%w(string.h sys_string.h),
		"*", "string_dup")
}

for fnam, c in settings
	puts "writing #{fnam}"
	fp = File.open(fnam, "w")
	template = File.open("sys_hash.txt")
	r = <<END_OF_RUBY
fp.print <<EOL
#line 1 "sys_hash.txt"
EOL
END_OF_RUBY
	for line in template
		case line
		when /^%/ then
			r << $'
			r << <<END_OF_RUBY
fp.print <<EOL
#line #{template.lineno + 1} "sys_hash.txt"
EOL
END_OF_RUBY
		else r << <<END_OF_RUBY
fp.puts "#{line.chomp.gsub(/"/, '\"')}"
END_OF_RUBY
		end
	end
	template.close
	eval(r)
	fp.close
end
