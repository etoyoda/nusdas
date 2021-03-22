#!/usr/bin/env ruby
# vi:set sw=4:
require 'cparse'
require 'optparse'

class CFortran

    @@inout = 'I/O'

    def self::frttype str
	case str
	when /^(const )?char (\[\d*\])*\[(\d+)\]$/ then "CHARACTER(#{$3})"
	when /^(const )?(unsigned )?char \[\]$/ then "CHARACTER"
	when /^(const )?char \*$/ then "CHARACTER(*)"
	when /^(const )?N_[SU]I4( \*| \[\d*\])?$/ then "INTEGER(4)"
	when /^int$/ then "INTEGER(4)"
	when /^(const )?N_[SU]I2( \*| \[\d*\])?$/ then "INTEGER(2)"
	when /^(const )?float( \*| \[\])?$/ then "REAL(4)"
	when /^(const )?void \*$/ then "\\AnyType"
	when /^(const )?(SRF_AMD_SINFO) \*$/ then "type(#{$2})"
	else raise "unsupported <#{str}>"
	end
    end

    def self::arysize str
	case str
	when /^(const )?char \[(\d+)\]$/ then ""
	when /^(const )?(unsigned )?char \[\](\[\d+\])*$/ then "\\AnySize"
	when /^(const )?char \*$/ then "\\AnySize"
	when /^(const )?(N_[SU]I[24]|int)( \*)?$/ then ""
	when /^(const )?N_[SU]I[24] \[(\d+)\]?$/ then "#{$2}"
	when /^(const )?N_[SU]I[24] \[\]$/ then "\\AnySize"
	when /^(const )?float( \*)?$/ then ""
	when /^(const )?float \[\]$/ then "\\AnySize"
	when /^(const )?void \*$/ then "\\AnySize"
	when /^(const )?(SRF_AMD_SINFO) \*$/ then "\\AnySize"
	else raise "unsupported <#{str}>"
	end
    end

    def self::intent str
	case str
	when /^\w+$/ then "IN"
	when /^const \w+( \*| \[\d*\](\[\d+\])*)?$/ then "IN"
	when /^const unsigned \w+( \*| \[\d*\](\[\d+\])*)?$/ then "IN"
	when /^((unsigned )?\w+) (\*|\[\d*\])$/ then @@inout
	else raise "unsupported <#{str}>"
	end
    end

    def self::intent_out_hack
	@@inout = 'OUT'
    end

end

class CProDoc

    Options = Struct.new(:funcs, :api, :cpp, :cppflags, :out, :lang, :mangle,
    	:exclude)

    def initialize
	@options = Options.new
	@options.funcs = {}
	@options.api = []
	@options.cpp = "cpp"
	@options.cppflags = "-C"
	@options.out = nil
	@options.lang = :doc_C
	@options.mangle = []
	@options.exclude = []
	@funcs = {}
    end

    def getopts argv
	opts = OptionParser.new do |opts|
	    opts.banner = "Usage: #{$0} [options]"
	    opts.separator ""
	    opts.separator "Options:"
	    opts.on("-f", "--functions f1,f2,f3,...",
		"functions to document") do |funcs|
		    for func in funcs.split(/,/)
			@options.funcs[func] = true
		    end
	    end
	    opts.on("-a", "--api header.h,...",
		"headers where public functions declared") do |hdrs|
		    @options.api += hdrs.split(/,/)
	    end
	    opts.on('-x', '--exclude regexp',
	    	'functions to exclude (used with -a)') do |re|
		    @options.exclude.push Regexp.new(re)
		end
	    opts.on("-E", "--cpp cpp", "command to invoke") do |cpp|
		@options.cpp = cpp
	    end
	    opts.on("-F", "--cppflags flags", "flags to cpp") do |cpf|
		@options.cppflags = cpf
	    end
	    opts.on("-O", "--output file",
		"@ is replaced to function name") do |out|
		    @options.out = out
	    end
	    opts.on("-L", "--lang Fortran",
	    	"create documents for Fortran (default: C)") do |lang|
		    @options.lang = ('doc_' + lang).intern
		end
	    opts.on("-m", "--mangle {downcase|upcase|s/re/str/}",
	    	"specify how functions are renamed") do |mangle|
		    @options.mangle << mangle
		end
	    opts.on('--intent-out-hack') do
		CFortran::intent_out_hack
	    end
	    opts.on("-d", "debug using Ruby's") do $DEBUG = true end
	    opts.on("-w", "--verbose", "using Ruby's") do $VERBOSE = true end
	    opts.on_tail("-h", "--help", "Show this message") do
		puts opts
		exit
	    end
	end
	opts.parse!(argv)
    end

    def on_extdecl extdecl
	return unless extdecl.function?
	name = extdecl.name
	for re in @options.exclude
	    return if re === name
	end
	if @options.api.include?(File.basename(extdecl.filename))
	    puts "#{name} <= #{extdecl.filename}" if $DEBUG
	    @options.funcs[name] = true
	elsif @options.funcs[name]
	    puts "#{name} accepted" if $DEBUG
	    @funcs[name] = extdecl
	end
    end

    def scansource filename
	parser = CParser.new
	handler = method(:on_extdecl)
	begin
	    cppcmd = "|#{@options.cpp} #{@options.cppflags} #{filename}"
	    open(cppcmd, 'r') do |fp|
		parser.parse(fp, :on_extdecl => handler)
	    end
	rescue Racc::ParseError
	    exit 1
	end
    end

    def tex str
	text = str.dup
	text.gsub!(/\w+\(\)/) do |funcref|
	    xname = ''
	    funcref.each_byte do |b|
		case b
		when 40 then break
		when 95 then xname << '.'
		else xname << b.chr
		end
	    end
	    "\\APILink{#{xname}}{#{funcref.sub(%r<\(\)>, '')}}"
	end
	text.gsub!(/_/, '\_')
	text.gsub!(/#/, '\#')
	text.gsub!(/%/, '\%')
	text.gsub!(/\*/, '$\ast$')
	text.gsub!(/(\\[a-z]+)\$\\ast\$/, '\1*')
	text.gsub!(/@p (\S+)\b/, '{\it \1}')
	text.gsub!(/\[\]/, '[\,]')
	text
    end

    def cfunc_mangle string
	str = string.dup
	for op in @options.mangle
	    case op
	    when %r<^upcase$> then str.upcase!
	    when %r<^downcase$> then str.downcase!
	    when %r<^s/([^/]*)/([^/]*)/$> then
		subto = $2
		re = Regexp.new($1)
		str.sub!(re, subto)
	    when %r<^s/([^/]*)/([^/]*)/i$> then
		subto = $2
		re = Regexp.new($1, Regexp::IGNORECASE)
		str.sub!(re, subto)
	    end
	end
	str
    end

    def doc_C name, fp
	xname = tex(mname = cfunc_mangle(name))
	proto = (func = @funcs[name]).texstr
	proto.gsub!(name, mname)
	proto = tex(proto)
	puts "undocumented #{name}" if func.brief.nil?
	text = <<TEX
\\subsection{#{xname}: #{func.brief}}
\\APILabel{#{xname.gsub(/\\?_/, '.')}}

\\Prototype
\\begin{quote}
#{proto};
\\end{quote}

\\begin{tabular}{l|rp{20em}}
\\hline
\\ArgName & \\ArgType & \\ArgRole \\\\
\\hline
TEX
	func.each_arg do |arg|
	    aname = tex(arg.name)
	    tname = tex(arg.typename)
	    com = tex(arg.comment.to_s)
	    puts "undocumented #{name}/#{aname}" if /^\s*$/ =~ com
	    text << "{\\it #{aname}} & #{tname} & #{com} \\\\\n"
	end
	text << <<TEX
\\hline
\\end{tabular}
TEX
	text << tex(func.docs)
        fp.print text
    end

    def doc_Fortran name, fp
	xname = tex(mname = cfunc_mangle(name))
	proto = (func = @funcs[name]).frtcall
	proto.gsub!(name, mname)
	proto = tex(proto)
	text = <<TEX
\\subsection{#{xname}: #{func.brief}}
\\APILabel{#{xname.gsub(/\\?_/, '.').downcase}}

\\Prototype
\\begin{quote}
#{proto}
\\end{quote}

\\begin{tabular}{l|rllp{16em}}
\\hline
\\ArgName & \\ArgType & \\ArrayDim & I/O & \\ArgRole \\\\
\\hline
TEX
	func.each_farg do |arg|
	    name = tex(arg.frtname)
	    tn = tex(arg.frt_typename)
	    dn = tex(arg.frt_diminfo)
	    it = tex(arg.frt_intentinfo)
	    com = tex(arg.comment.to_s)
	    text << "{\\it #{name}} & #{tn} & #{dn} & #{it} & #{com} \\\\\n"
	end
	text << <<TEX
\\hline
\\end{tabular}
TEX
	text << tex(func.docs)
	fp.print text
    end

    def savefunc name
	if @options.out
	    xname = cfunc_mangle(name).downcase
	    fnam = @options.out.gsub(/@/, xname)
	    File.open(fnam, 'w') do |fp|
		self.send @options.lang, name, fp
	    end
	    puts "#{fnam} written"
	else
	    self.send @options.lang, name, STDOUT
	end
    end

    def run argv
	getopts(argv)
	for file in argv
	    puts "scanning #{file}" if @options.out
	    scansource(file)
	end
	for name in @funcs.keys.sort
	    savefunc name
	end
    end

end

module CPTree
    class SymDecl
	def texstr
	    "#{declspec.cstr} #{sym.texstr}"
	end
	def frtcall
	    "CALL #{sym.frtcall(declspec)}"
	end
	def each_farg
	    sym.each_farg(declspec) do |arg|
		yield arg
	    end
	end
	TEXTRANS = {
	    /<DT>([^<]+)<DD>/i	=> '\item[{\bf \1}] ',
	    /<DL>/i		=> '\begin{quote}\begin{description}',
	    /<\/DL>/i		=> '\end{description}\end{quote}',
	    /<UL>/i		=> '\begin{itemize}',
	    /<LI>/i		=> '\item ',
	    /<BR>/i		=> '\newline ',
	    /<\/UL>/i		=> '\end{itemize}',
	    /<H3>([^<]+)<\/H3>/i => '\paragraph{\1}',
	    /<TT>([^<]+)<\/TT>/i => '{\tt \1}',
	    /<B>([^<]+)<\/B>/i => '{\bf \1}',
	    /@bug/		=> '\paragraph{\Bug}'
	}
	RETVAL = "\\end{description}\\end{quote}\n"
	def docs
	    text = ''
	    block = nil
	    for line in @comment.to_s.split(/\n/)
		case line
		when /@retval\s+(\S+)\s+/
		    item, arg = $1, $'
		    if block != RETVAL
			text << <<EOF
\\paragraph{\\ResultCode}
\\begin{quote}
\\begin{description}
EOF
			block = RETVAL
		    end
		    text << "\\item[{\\bf #{item}}] #{arg}\n"
		else
		    if block
			text << block
			block = false
		    end
		    for pat, subst in TEXTRANS
			line.gsub!(pat, subst)
		    end
		    text << line + "\n"
		end
	    end
	    text = '\paragraph{\FuncDesc}' + text unless text.empty?
	    text
	end
    end
    class DeclSpec
	def frtname
	    "result"
	end
	def frtcall
	    "{\\it #{frtname}}"
	end
	undef comment
	def comment
	    "\\ResultCode"
	end
	def frt_typename() CFortran::frttype(typename) end
	def frt_diminfo() CFortran::arysize(typename) end
	def frt_intentinfo() 'OUT' end
    end
    class Func
	def bf_subj
	    if Identifier === subject then "{\\bf #{subject.cstr}}"
	    else subject.texstr
	    end
	end
	def texstr
	    "#{bf_subj}(#{object ? object.texstr : ''})"
	end
	def frtcall(declspec)
	    args = []
	    each_farg(declspec) do |arg|
		args.push arg.frtcall
	    end
	    "#{bf_subj}(#{args.join(', ')})"
	end
	def each_farg(declspec)
	    each_arg do |arg|
		yield arg
	    end
	    yield declspec if declspec.cstr != 'void'
	end
    end
    class CArray
	def texstr
	    obj = object ? object.cstr : ''
	    "{\\it #{subject.cstr}}[#{obj}]"
	end
	def frtcall
	    "{\\it #{subject.name}}"
	end
    end
    class Pointer
	def texstr
	    "*#{object.cstr}{\\it #{subject.cstr}}"
	end
	def frtcall
	    "{\\it #{subject.cstr}}"
	end
    end
    class Param
	def texstr
	    if Identifier === object then obj = "{\\it #{object.cstr}}"
	    else obj = object.texstr
	    end
	    "#{subject.cstr} #{obj}"
	end
	def frtcall
	    if Identifier === object then "{\\it #{object.cstr}}"
	    else object.frtcall
	    end
	end
	def frtname
	    name
	end
	def frt_typename() CFortran::frttype(typename) end
	def frt_diminfo() CFortran::arysize(typename) end
	def frt_intentinfo()
	    intent_out? ? 'OUT' : CFortran::intent(typename)
	end
    end
    class VarArgs
	def texstr() cstr end
    end
    class List
	def texstr
	    @a.collect{|elem| elem.texstr}.join(', ')
	end
    end
end

CProDoc.new.run(ARGV)
