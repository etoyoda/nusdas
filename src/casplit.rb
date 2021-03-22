#!/usr/bin/ruby
# ex: set sw=4:

class File

    def self::touch filenames
	now = Time.now
	begin
	    utime(now, now, *filenames)
	rescue Errno::ENOENT
	end
    end

end

class CASplit

    def initialize(input)
	@input = input
	@common = []
	@part = {}
	@code = {}
    end

    def collect
	part = nil
	for line in @input
	    case line
	    when /^\s*#\s*pragma\s+casplit\s+common\b/
		part = nil
		@common = []
	    when /^\s*#\s*pragma\s+casplit\s+part\s+([.\w]+)/
		part = $1
		raise "duplicated part #{part}" if @part[part]
	    else
		if part
		    if @part[part].nil?
			@part[part] = @common.dup
			@part[part].push "#line #{@input.lineno} \"#{@input.filename}\"\n"
		    end
		    @part[part].push line
		else
		    @common.push line
		end
	    end
	end
    end

    def putfile fnam, text
	unless File.exist?(fnam)
	    File.open(fnam, "w") { |fp| fp.write text }
	    puts "#{fnam} created"
	    return
	end
	orig = File.read(fnam)
	if orig.gsub(/^#line .*$/, '') == text.gsub(/^#line .*$/, '')
	    puts "#{fnam} unchanged" if $DEBUG
	    File.touch([fnam, fnam.sub(/\.c$/, '.o')])
	else
	    File.open(fnam, "w") { |fp| fp.write text }
	    puts "#{fnam} updated"
	end
    end

    def convert part, infix, setcase, suffix
	code = @part[part].join
	fnam = part.sub(/^./) { $& + infix }
	code.sub!(/Fortran_(\w+)/) { $1.send(setcase) + suffix }
	putfile(fnam, code)
    end

    def output
	for part in @part.keys.sort
	    convert part, '0', :downcase, ''
	    convert part, '1', :upcase, ''
	    convert part, '2', :downcase, '_'
	    convert part, '3', :upcase, '_'
	    convert part, '4', :downcase, '__'
	    convert part, '5', :upcase, '__'
	end
    end

    def run
	collect
	output
    end

end

CASplit.new(ARGF).run
