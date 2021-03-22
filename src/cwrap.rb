#!/usr/bin/env ruby
# vi:set sw=4:
require_relative './cparse.rb'
require 'optparse'

class File
    def self::touch filename
	File.open(filename, 'a') { |fp|
	    fp.truncate(fp.pos)
	}
    end
end

class Counter
    def initialize
	@c = {}
    end
    def count key
	@c[key] = 0 unless @c.include?(key)
	@c[key] += 1
    end
    def to_hash
	@c.dup
    end
    def report
	for key in @c.keys.sort
	    printf "\t%30s%4d\n", key, @c[key]
	end
    end
end

class NameSpace

    def initialize
	@table = {}
    end

    def push payload
	while @table.include?(payload.name)
	    payload.rename
	end
	@table[payload.name] = payload
    end

    def to_hash
	@table
    end

    def each
	for name in @table.keys.sort
	    yield name, @table[name]
	end
    end

end

class Variable

    def initialize cname, ctype
	@cname, @ctype = cname, ctype
	@name0 = @name = @cname.nil? ? 'arg' : @cname
	@parent = nil
    end
    
    attr_reader :name

    def rename
	if @name.equal? @name0
	    @name = @name + '0'
	else
	    @name = @name.succ
	end
    end

end

class ShieldVar < Variable

    def attach_parent(parent)
	@parent = parent
	self
    end

    MAX = 1024

    def copyin
	case @ctype
	when /\bchar\b/
	    len = "(#{@parent.len.name} >= #{MAX})" +
		" ? #{MAX - 1} : #{@parent.len.name}"
	    "memcpy(#{@name}, #{@parent.name}, #{len});\n\t" +
		"#{@name}[#{len}] = '\\0';"
	else nil
	end
    end

    def decl
	case @ctype
	when /\bint\b/
	    "int #{@name} = *#{@parent.name};"
	when /\bchar\b/
	    "char #{@name}[#{MAX}];"
	else raise "<#@ctype> unsupported"
	end
    end

    def copyout
	case @ctype
	when /\bconst\b/ then nil
	when /\bint\b/ then "*#{@parent.name} = #{@name};"
	when /\bchar\b/ then
	    len = "(#{@parent.len.name} >= #{MAX})" +
		" ? #{MAX - 1} : #{@parent.len.name}"
	    "memcpy(#{@parent.name}, #{@name}, #{len});"
	else nil
	end
    end

    def ref
	case @ctype
	when /\bint\b/ then "&#{@name}"
	when /\bchar\b/ then @name
	else raise
	end
    end

    def int?() /\bint\b/ === @ctype end

end

class ArgInfo < Variable

    def initialize *args
	super
	@len = @shield = nil
    end

    attr_reader :len

    def argdecl
	case @ctype
	when /^(const )?(double|float|N_[US]I[24]|char|void)$/
	    "const #{$2} *#{@name}"
	when /^(const )?(double|float|N_[US]I[24]|char|void|SRF_AMD_SINFO)( [\*&]| \[\d*\])$/
	    "#{$1}#{$2} *#{@name}"
	when /^(const )?(int)( [\*&]| \[\d*\])?$/
	    "#{$1}N_SI4 *#{@name}"
	when /^(const )?char \[\]\[\d+\]$/
	    "#{$1}char *#{@name}"
	when /^unsigned$/
	    "#@ctype #@name"
	when /^(const )?unsigned char \[\]$/
	    "#{$1}unsigned char *#{@name}"
	else
	    raise "<#{@ctype}> unsupported"
	end
    end

    def array2d?
	/\]\[/ =~ @ctype
    end

    def fallbacktype
	@ctype.sub(/\[\d*\]/, '(*)')
    end

    def pointer?
	/[\*\[]/ =~ @ctype
    end

    def ref0
	pointer? ? @name : "*#{@name}"
    end

    def ref
	r = @shield ? @shield.ref : ref0
	r = "(#{fallbacktype})#{r}" if array2d?
	r
    end

    def add_shield(var) @shield = var end

    def shield_var
	case @ctype
	when /^(const )?int( \*| \[\d*\])$/, /^(const )?char \*$/
	    add_shield(ShieldVar.new(@cname, @ctype).attach_parent(self))
	else
	    nil
	end
    end

    def needcopy?
	case @ctype
	when /^(const )?char \*$/
	    true
	else
	    nil
	end
    end

    def add_len(var) @len = var end

    def len_var
	case @ctype
	when /\bchar\b/
	    add_len(self.class.new("L_#{@cname}", 'unsigned'))
	else
	    nil
	end
    end

end

class Wrapper

    @@iftypes = Counter.new
    @@iatypes = Counter.new

    def initialize funcname, functype
	@funcname, @functype = funcname, functype
	@@iftypes.count(functype)
	@args = []
	@argNameSpace = NameSpace.new
	@ret = @fargs = @shields = nil
	@frt = :frt_gnu
	@namepos = 0
	@incs = []
	@mangle = []
	@si4 = nil
	rename
    end

    attr_reader :funcname, :name

    NAMELEN = 10
    MINILEN = 8

    def rename
	case @namepos
	when 0
	    @name = @funcname.downcase.sub(/^(nusdas|nwp.?)_/, '').gsub(/_/, '')
	    @name = @name[-NAMELEN,NAMELEN] if @name.size > NAMELEN
	when 1
	    @name = @funcname.downcase.gsub(/_/, '')
	    @name = @name[-MINILEN,MINILEN] if @name.size > MINILEN
	    @name = @name + '00'
	else
	    @name = @name.succ
	end
	@namepos = @namepos.succ
    end

    def add_arg argname, argtype
	return if /^void$/ === argtype
	@@iatypes.count(argtype)
	arg = ArgInfo.new(argname, argtype)
	@args << arg
	@argNameSpace.push arg
    end

    def add_inc(inc) @incs += inc end
    def add_mangle(inc) @mangle += inc end

    def add_si4 type
	@si4 = type
    end

    def compile
	return unless @fargs.nil?
	unless @functype == 'void'
	    @ret = ArgInfo.new('result', "#{@functype} &")
	end
	@fargs = (@args + [@ret]).compact
	if (@fargs.collect{|lv| lv.needcopy?}.any?) then
	    @incs.push 'string.h' unless @incs.include?('string.h')
	    lenvars = @fargs.collect{|arg| arg.len_var}.compact
	    @wargs = (@fargs + lenvars).compact
	else
	    @wargs = @fargs
	end
	@shields = @fargs.collect{|arg| arg.shield_var}.compact
	for shield in @shields
	    @argNameSpace.push shield
	end
    end

    def frt_gnu string
	if /_/ =~ string
	    string.downcase.sub(/$/, '__')
	else
	    string.downcase.sub(/$/, '_')
	end
    end

    def frt_ifc string
	string.downcase.sub(/$/, '_')
    end

    def frt_ibm string
	string.downcase
    end

    def frt_win string
	string.upcase
    end

    def frt= compiler
	@frt = compiler
    end

    COMPILERS = [:frt_gnu, :frt_ifc, :frt_ibm, :frt_win]

    def frt
	string = @funcname.dup
	for directive in @mangle
	    string.send(*directive)
	end
	string = send(@frt, string)
	if string == @funcname
	    raise "wrapper name collision for <#{string}>: try -m option"
	end
	string
    end

    def wrapargs
	@wargs
    end

    def main(flags = {})
	if flags[:inthack]
	    cargs = @args.collect{|arg| arg.ref0}.join(",\n\t\t");
	else
	    cargs = @args.collect{|arg| arg.ref}.join(",\n\t\t");
	end
	text = "\t"
	text << (@ret ? @ret.ref + ' = ' : '');
	text << "#{@funcname}(#{cargs});\n"
    end

    def shield_pre
	return "" if @shields.empty?
	pre = (@shields.collect{|var| var.decl} +
	    @shields.collect{|var| var.copyin}).compact
	pre.empty? ? '' : "\t" + pre.join("\n\t") + "\n"
    end

    def shield_post
	post = @shields.collect{|var| var.copyout}.compact
	post.empty? ? '' : "\t" + post.join("\n\t") + "\n"
    end

    def wrapper_body0
	shield_pre + main + shield_post
    end

    def no_int_branch
	return true if @shields.empty?
	not @shields.collect{|v| v.int?}.all?
    end

    def wrapper_body
	return wrapper_body0 if no_int_branch
	"    if (sizeof(int) == sizeof(N_SI4)) {\n" +
	main(:inthack => true) +
	"    } else {\n" +
	wrapper_body0 +
	"    }\n"
    end

    def wrapper
	compile
	text = ""
	for inc in @incs
		text << "#include <#{inc}>\n"
	end
	if @si4
	    text << <<EOF
#ifndef N_SI4
# define N_SI4	#{@si4}
#endif
EOF
	end
	fortran = frt
	text << "\n"
	text << "#undef #{fortran}\n"
	text << "\tvoid\n#{fortran}("
	text << wrapargs.collect{|arg| arg.argdecl}.join(",\n\t")
	text << ")\n{\n"
	text << wrapper_body
	text << "}\n"
    end

    def self::report
	puts '--- function types ---'
	@@iftypes.report
	puts '--- argument types ---'
	@@iatypes.report
    end

end

class CWrap

    Options = Struct.new(:out, :si4, :inc, :mangle, :cflink, :exclude, :void)

    def initialize
	@options = Options.new
	@options.si4 = nil
	@options.out = nil
	@options.inc = []
	@options.mangle = []
	@options.cflink = Wrapper::COMPILERS
	@options.exclude = []
	@options.void = []
	@wrappers = NameSpace.new
    end

    def getopts argv
	opts = OptionParser.new do |opts|
	    opts.banner = "Usage: #{$0} [options]"
	    opts.separator ""
	    opts.separator "Options:"
	    opts.on("-O", "--output file",
		"@ is replaced to function name") do |out|
		    @options.out = out
	    end
	    opts.on("-i", "--include nusdas.h,nwpl_capi.h,...",
		"#include's for wrappers") do |arg|
		@options.inc = arg.split(/,/)
	    end
	    opts.on('--si4 type', 'define 32bit integer N_SI4 [int]') do |si4|
		@options.si4 = si4
	    end
	    opts.on('--void regexp',
		'functions whose return code is ignored') do |re|
		    @options.void.push Regexp.new(re)
		end
	    opts.on('-x', '--exclude regexp', 'functions to omit') do |re|
		@options.exclude.push Regexp.new(re)
	    end
	    opts.on("-m", "--mangle 's/nusdas_swab/endian_swab/i'",
	    	"exceptional Fortran names") do |directive|
		    case directive
		    when %r<^s/([^/]*)/([^/]*)/$>
			@options.mangle.push [:sub!, Regexp.new($1), $2]
		    when %r<^s/([^/]*)/([^/]*)/i$>
			re = Regexp.new($1, Regexp::IGNORECASE)
			@options.mangle.push [:sub!, re, $2]
		    else raise "unrecognized rename rule '#{directive}'"
		    end
		end
	    opts.on("-f", "--cflink gnu,ifc,ibm,win",
		"linkage conventions") do |cflink|
		    @options.cflink = cflink.split(/,/).collect{ |compiler|
			('frt_' + compiler).intern
		    }
	    end
	    opts.on("-d", "--debug", "Ruby's $DEBUG=true") do
		$DEBUG = true
	    end
	    opts.on("-w", "--verbose", "Ruby's $VERBOSE=true") do
		$VERBOSE = true
	    end
	    opts.on_tail("-h", "--help", "Show this message") do
		puts opts
		exit
	    end
	end
	opts.parse!(argv)
    end

    def makewrap func
	return unless func.function?
	name = func.name
	for re in @options.exclude
	    return if re === name
	end
	ftype = func.functype
	for re in @options.void
	    ftype = 'void' if re === name
	end
	f = Wrapper.new(name, ftype)
	f.add_si4(@options.si4) if @options.si4
	f.add_mangle @options.mangle
	f.add_inc @options.inc
	func.each_arg do |arg|
	    f.add_arg arg.name, arg.typename
	end
	@wrappers.push f
    end

    def scansource filename
	parser = CParser.new
	#cppcmd = "|#{@options.cpp} #{@options.cppflags} #{filename}"
	File.open(filename, 'r') do |fp|
	    parser.parse(fp, :on_extdecl => method(:makewrap))
	end
    end

    def tex str
	str.gsub(/_/, '\_').gsub(/\*/, '$\ast$')
    end

    def run argv
	getopts(argv)
	for file in argv
	    scansource(file)
	end
	finalize
    end

    def savefile name, text
	unless @options.out
	    print text
	else
	    if /@/ =~ @options.out then filename = @options.out.sub(/@/, name)
	    else filename = @options.out + name
	    end
	    filename += '.c' unless /\.\w+$/ =~ filename
	    begin
		current = File.read(filename)
	    rescue Errno::ENOENT
		current = nil
	    end
	    if (text == current) 
		puts "touch #{filename}"
		File.touch filename
	    else
		File.open(filename, 'w') {|fp| fp.write text}
		puts "written #{filename}"
	    end
	end
    end

    def finalize
	for name, f in @wrappers
	    n = 0
	    for compiler in @options.cflink
		f.frt = compiler
		subname = "#{n}#{name}"
		savefile subname, f.wrapper
		n = n.succ
	    end
	end
	Wrapper.report if $DEBUG
    end

end

CWrap.new.run(ARGV)
