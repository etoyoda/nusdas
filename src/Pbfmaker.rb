#!/usr/bin/env ruby

require 'optparse'

=begin
こんなのを出力する。
@pbf
  USRDIR /dvlK2/npd1/npd_pg/suuchi43/usrdir
  CLASS Comm
  INCMK ${RTNDIR}/Comm/Mk/Nwpmake/nwpmake.mk
  @lib Nwp/libnusdas13.a
    @aopt -X64
    #@ipath Nwp/Incnusdas13:Nwp/Inc
    @dir Nwp/Incnusdas13
      @mod nusdas.h
    @src
      @dir Nwp/Nusdas13
      @option -64 -O4
        api_read.c: nusdas.h
        nusdas.h:
    @end src
  @end lib
@end pbf
=end

class PLib

	def initialize hdr, dep, pbf, dir
		@hdr, @dep, @pbf, @dir = hdr, dep, pbf, dir
	end

	def basename
		@hdr
	end

	def depname
		@hdr
	end

	def pbfline
		@pbf ? "# #{@hdr} comes from #{File.basename @pbf}": nil
	end
	
	def ref_dir
		@dir
	end
	
	def ref_src
		@pbf ? nil: @dep ? "#{@hdr}: #{@dep}" : @hdr
	end
end

class SourceInfo

	def include_C line
		return $1 if /^\s*#\s*include\s*"([^"]+)"/ =~ line
		return $1 if /^\s*#\s*include\s*<([^>]+)>/ =~ line
		false
	end

	def include_Fortran line
		return $1 if /^\s*INCLUDE\s*'([^']+)'/i =~ line
		return $1 if /^\s*INCLUDE\s*"([^"]+)"/i =~ line
		return $1.downcase + '.f90' if /^\s*USE\s+(\w+)/i =~ line
		false
	end

	def include_CppFortran line
		return $1 if /^\s*#\s*include\s*"([^"]+)"/ =~ line
		return $1 if /^\s*#\s*include\s*<([^>]+)>/ =~ line
		return $1 if /^\s*INCLUDE\s*'([^']+)'/i =~ line
		return $1 if /^\s*INCLUDE\s*"([^"]+)"/i =~ line
		return $1.downcase + '.f90' if /^\s*USE\s+(\w+)/i =~ line
		false
	end

	def initialize filename, pick
		@pick = pick
		@picked = pick ? false : true
		@filename = filename
		case @filename
		when /\.[ch]$/
			@inc = :include_C
		when /\.(f|f90)$/
			@inc = :include_Fortran
		when /\.(F|F90)$/
			@inc = :include_CppFortran
		when /\.o$/
			@filename.sub!(/\.o$/, '.f90')
			@inc = :include_Fortran
		else
			raise "#{@filename}: unknown language"
		end
		@incs = []
	end

	def scan
		File.open(@filename) { |fp|
			for line in fp
				@picked = true if @pick and @pick === line
				inc = send(@inc, line)
				inc = yield(inc) if inc
				@incs.push inc if inc
			end
			@incs.uniq!
		}
		self
	end

	def picked?
		@picked
	end

	def basename
		File.basename @filename
	end

	def depname
		b = basename
		case b
		when /\.f90$/ then b.sub(/\.f90$/, '.o')
		else b
		end
	end

	def src
		@filename
	end

	def pbfline
		a = ([basename + ':'] + @incs.collect{|fs| fs.depname})
		a.join(' ')
	end

end

class PbfMaker

	def initialize
		@entry = nil
		@USRDIR = @CLASS = @SRCCLASS = @LIBCLASS = @INCDIR = @SRCDIR = nil
		@TARGET = @CFLAGS = @FFLAGS = @ARFLAGS = @SPC = @CC = nil
		@LOAD = @ABST = @DETAIL = nil
		@INCMK = 'nwpmake.mk'
		@sources = {}
		@incpath = []
		@syshdrs = {}
		@apihdrs = []
		@plibs = []
		@ref_srcs = []
		@picky_sys = false
	end

	def help neededflag
		puts "missing config: specify #{neededflag} " \
			+ "(try -h for more info)"
		exit 1
	end

	def mandatory_check
		help '-t or --target' if @TARGET.nil?
		help '-u or --usrdir' if @USRDIR.nil?
		help '-c or --class' if @CLASS.nil?
		help '-s or --srcclass' if @SRCCLASS.nil?
		help '-l or --libclass' if @LIBCLASS.nil?
	end

	def config opts
		opts.banner = "Usage: #{$0} [flags] sources ..."
		opts.separator ""
		opts.separator "Mandatory Flags:"
		opts.on('-t', '--target file', 'ex. libnusdas.a or program') {
			|arg| @TARGET = arg
		}
		opts.on('-u', '--usrdir directory') { |arg| @USRDIR = arg }
		opts.on('-c', '--class class') { |arg| @CLASS = arg }
		opts.on('-C', '--c_compiler name') { |arg| @CC = arg }
		opts.on('-s', '--srcclass name') { |arg| @SRCCLASS = arg }
		opts.on('-l', '--libclass name') { |arg| @LIBCLASS = arg }
		opts.separator "Options:"
		opts.on('-I', '--incpath dir', 'add include directory') {
			|arg| @incpath.push arg }
		opts.on('-i', '--incdir name',
			'where headers/mods installed') { |arg| @INCDIR = arg }
		opts.on('-r', '--srcdir name') { |arg| @SRCDIR = arg }
		opts.on('-f', '--cflags string') { |arg| @CFLAGS = arg }
		opts.on('-g', '--fflags string') { |arg| @FFLAGS = arg }
		opts.on('-a', '--arflags string') { |arg| @ARFLAGS = arg }
		opts.on('-A', '--abstract string') { |arg| @ABST = arg }
		opts.on('-d', '--dtail string') { |arg| @DETAIL = arg }
		opts.on('-L', '--module load string') { |arg| @LOAD = arg }
		opts.on('-p', '--supercede path') { |arg| @SPC = arg }
		opts.on('-h', 'this help') { puts opts; exit 1 }
		opts.on('--incmk nwpmake.mk', 'change INKMK line') {
			|arg| @INCMK = arg }
		opts.on('--plib PBF:foo.h,bar.mod',
			'inserts @plib into PBF') { |arg|
			pbf, ref_dir, hdrs = arg.split(/;/)
			pbf =  nil if pbf.empty?
			@plibs.push pbf if pbf
			for hs in hdrs.to_s.split(/,/)
				hdr = hs.split(":")
				if hdr.length > 1
					@ref_srcs << @sources[hdr[0]] = PLib.new(hdr[0], hdr[1], pbf, ref_dir)
				else
					@ref_srcs << @sources[hdr[0]] = PLib.new(hdr[0], nil, pbf, ref_dir)
				end
			end
		}
		opts.on('-m', '--hdrmods foo.h,bar.mod',
			'comma-separated list of headers/mods') { |arg|
			@apihdrs += arg.split(/,/)
		}
		opts.on('-S', '--syshdr foo.h,bar.mod',
			'comma-separated list of system headers/mods') { |arg|
			@picky_sys = true
			for hdr in arg.split(/,/)
				@syshdrs[hdr] = true
			end
		}
	end

	def incdir
		@INCDIR ? "#{@LIBCLASS}/#{@INCDIR}" : @LIBCLASS
	end

	def srcdir
		@SRCDIR ? "#{@SRCCLASS}/#{@SRCDIR}" : @SRCCLASS
	end

	def ipathline
		@SPC ? "@ipath #{incdir}:#{@SPC}" : ''
	end

	def register_hdr
	end

	def register fs
		basename = fs.basename
		if @sources.include? basename
			raise "duplicated file #{basename}"
		end
		@sources[basename] = fs
	end

	def findhdr hdrname, srcdir
		for dir in @incpath + [srcdir]
			try = File.join(dir, hdrname)
			return try if File.exist?(try)
		end
		nil
	end

	def register_header hdrname, srcdir
		return @sources[hdrname] if @sources.include? hdrname
		return false if @syshdrs.include? hdrname
		unless filename = findhdr(hdrname, srcdir)
			@syshdrs[hdrname] = true
			msg = "<#{hdrname}> considered system header"
			raise msg if @picky_sys
			puts msg
			return false
		end
		fs = SourceInfo.new(filename, nil)
		@sources[hdrname] = nil
		fs.scan do |subhdr|
			register_header subhdr, srcdir
		end
		@sources[hdrname] = fs
	end

	def search_source2 file, pick
		fs = SourceInfo.new(file, pick)
		fs.scan do |hdrname|
			register_header hdrname, File.dirname(file)
		end
		return unless fs.picked?
		register fs
	end

	def search_source1 file, pick
		if /\.o$/ =~ file
			basename = $`
			for suffix in %w(.c .f90 .f .F90 .F)
				file = basename + suffix
				next unless File.exist?(file)
				return search_source2(file, pick)
			end
			raise "source for #{basename}.o missing"
		end
		search_source2 file, pick
	end

	def search_source0 pat, pick
		if /[*?]/ =~ pat
			for file in Dir[pat]
				search_source1 file, pick
			end
		else
			search_source1 pat, pick
		end
	end

	def search_apihdrs
		for basename in @apihdrs.sort
			next if @sources.include? basename
			next if /\.mod$/ =~ basename
			path = findhdr(basename, '.')
			raise "missing #{basename}: try adding -I" unless path
			fs = SourceInfo.new(path, nil)
			fs.scan do |hdrname|
				register_header hdrname, File.dirname(path)
			end
			@sources[basename] = fs
		end
	end

	def search_sources argv
		pick = nil
		for arg in argv
			case arg
			when /^PICK=$/
				pick = nil
			when /^PICK=/
				pick = Regexp.new($')
			else
				search_source0(arg, pick)
			end
		end
	end

	def srcs mode
		text = []
		for basename in @sources.keys.sort
			dep = @sources[basename]
			line = dep.pbfline
			case mode
			when :c
			  next if /\.[fF](?:90)?\z/ =~ basename
			when :f
			  next unless /\.[fF](?:90)?\z/ =~ basename
			end
			text.push line if line
		end
		"\t" + text.join("\n\t")
	end
	
	def detail
		@DETAIL.gsub(/^"/, "").gsub(/"$/, "").split(",").map{|line| "#%   #{line}"}.join("\n")
	end
	
	def module_load
		block = ''
		if @LOAD && !@LOAD.empty?
			block = @LOAD.split(";").map{|line| "    #{line}"}.join("\n")
			block = "  @env\n#{block}\n  @end"
		end
		block
	end
	
	def compiler
		@CC ? "  CC #{@CC}" : ""
	end
	
	def ref_dirs
		lines = [];
		@ref_srcs.map{|src| src.ref_dir}.uniq.sort.each{|ref_dir|
			lines << "      "
			lines << "      @dir #{ref_dir}"
			@ref_srcs.each{|src|
				next if src.ref_dir != ref_dir
				lines << "        #{src.ref_src}" if src.ref_src
			}
		}
		lines.join("\n")
	end

	def apiblock
		block = ''
		for pbf in @plibs
			block += "    @plib #{pbf}\n"
		end
		for file in @apihdrs
			block += "    @mod #{file}\n"
		end
		unless @apihdrs.empty?
			block = "    #{ipathline}\n" + block
			block = "    @dir #{incdir}\n" + block
		end
		block
	end

	def each
		srcdirF = "#@USRDIR/#@CLASS/Src/#{srcdir}"
		incdirF = "#@USRDIR/#@CLASS/Lib/#{incdir}"
		for basename in @sources.keys.sort
			fs = @sources[basename]
			next if PLib === fs
			yield File.join(srcdirF, basename), fs.src
			if @apihdrs.include? basename
				yield File.join(incdirF, basename), fs.src
			end
		end
	end

	def pbf_path
		"#{@LIBCLASS}/#{@TARGET}".sub(/(\.a)?$/, '.pbf')
	end

	def pbf
		r = <<EOF
#-----------------------------------------------#
# PBF DOCUMENT MAINPAGE PART                    #
#-----------------------------------------------#
#%
#% @mainpage
#% @par メインルーチン:
#%
#% @author
#%
#% @par 概要:
#%   #{@ABST}
#% @par 詳細:
#{detail}
#% @par リターンコード情報:
#%
#% @par 入力データ:
#%
#% @par 出力データ:
#%
#% @par その他備考:
#%
#-----------------------------------------------#

@pbf
  CLASS #{@CLASS}
  INCMK ${RTNDIR}/Comm/Mk/Nwpmake/#{@INCMK}
#{module_load}
#{compiler}
  @lib #{@LIBCLASS}/#{@TARGET}
    @aopt #{@ARFLAGS}
#{apiblock}
    @src
      @dir #{srcdir}
      @option #{@CFLAGS}
#{srcs(:c)}
#{ref_dirs}

## fortran
      @dir #{srcdir}
      @option #{@FFLAGS}
#{srcs(:f)}
    @end src
  @end lib
@end pbf
EOF
		r
	end

	def compile argv
		opts = OptionParser.new(& method(:config))
		opts.parse!(argv)
		mandatory_check
		search_sources(argv)
		search_apihdrs
		self
	end

	def run argv
		compile argv
		print pbf
	end

end

if $0 == __FILE__
	PbfMaker.new.run(ARGV)
end
