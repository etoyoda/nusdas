module CPTree

	class TreeNode
		def inspect
			self.class.to_s.sub(/.*::/, '') + '{' + cstr + '}'
		end
		def check
		end
	end

	class Primary < TreeNode
		def initialize name
			@name = name.dup
			@name.freeze
			check
		end
		attr_reader :name
		def cstr() name end
		def function?() false end
	end

	class Identifier < Primary
		def ptrinfo() '' end
	end
	class Constant < Primary
	end

	class DeclSpec < TreeNode
		def initialize
			@storage_class = {}
			@type_qual = {}
			@type = []
			@comment = nil
			check
		end
		attr_reader :comment
		def comment= text
			@comment = text
		end
		def add_storage_class name
			@storage_class[name] = true
			self
		end
		def add_type name
			@type.unshift name
			self
		end
		def add_type_qual name
			@type_qual[name] = true
			self
		end
		def typedef?() @storage_class['typedef'] end
		def cstr
			a = @type_qual.keys.sort
			a += @storage_class.keys.sort
			a += @type
			a.join(' ')
		end
		def typename() cstr end
		def ptrstr() nil end
		def name() nil end
	end

	class Triplet < TreeNode
		def initialize subject, object
			@tag, @subject, @object = tag, subject, object
			@comment = nil
			@flags = {}
			check
		end
		attr_reader :tag, :subject, :object, :comment
		def name() subject.name; end
		def cstr() raise("missing in #{self.class}") end
	end

	class CArray < Triplet
		def tag() :CArray end
		def cstr()
			if object then "#{subject.cstr}[#{object.cstr}]"
			else "#{subject.cstr}[]"
			end
		end
		def ptrstr()
			bra = object ? "[#{object.cstr}]" : "[]"
			if Identifier === subject
				bra
			else
				"#{subject.ptrstr}#{bra}"
			end
		end
		def function?() false end
	end
	class Func < Triplet
		def tag() :Func end
		def cstr()
			if object then "#{subject.cstr}(#{object.cstr})"
			else "#{subject.cstr}()"
			end
		end
		def function?() true end
		def ptrinfo() subject.ptrinfo end
		def each_arg &callback
			return if object.nil?
			object.each(&callback)
		end
	end
	class Pointer < Triplet
		def tag() :Pointer end
		def cstr()
			"*#{object.cstr}#{subject.cstr}"
		end
		def ptrstr()
			if Identifier === subject
				"*#{object.cstr}"
			else
				"*#{object.cstr} #{subject.cstr}"
			end
		end
		def function?() subject.function? end
		def each_arg &callback
			return unless function?
			subject.each_arg(&callback)
		end
	end

	class Initialize < Triplet
		def tag() :Initialize end
		def cstr() subject.cstr end
		def function?() false end
	end

	class StrCat < Triplet
		def tag() :StrCat end
		def cstr() subject.cstr + ' ' + object.cstr end
		def function?() false end
	end

	class Param < Triplet
		def tag() :Param end
		def cstr()
			object_cstr = object.cstr
			object_cstr = "(#{object_cstr})" if / / =~ object_cstr
			"#{subject.cstr} #{object_cstr}".strip
		end
		def comment= text
			@comment = text
			for str in @comment
				if str.sub!(/\s*INTENT\(OUT\)\s*/, '')
					@flags[:intent_out] = true
				end
			end
		end
		def intent_out?() @flags.include?(:intent_out) end
		def function?() false end
		def typename()
			if Identifier === object
				subject.cstr
			else
				"#{subject.cstr} #{object.ptrstr}".strip
			end
		end
		def name() object.name end
		def check
		end
	end

	class SymDecl < TreeNode
		def initialize(declspec, sym)
			raise "BUG" unless DeclSpec === declspec
			raise "BUG" unless sym.respond_to?(:cstr) || sym.nil? 
			@declspec, @sym = declspec, sym
			@comment = nil
			@brief = nil
			@pos = nil
			check
		end
		attr_reader :declspec, :sym, :comment
		attr_accessor :pos
		def comment= text
			@comment = ""
			@brief = ""
			for line in text
				case line
				when /@brief\s*/
					@brief = $'
				when /^\s*\*\s*$/
					@comment << "\n"
				when /^\s+$/
				else
					line.sub!(/\s*\*\s*/, '')
					@comment << "#{line}\n"
				end
			end
		end
		def brief
			@brief
		end
		def cstr0()
			if sym.nil? then declspec.cstr
			else "#{declspec.cstr} #{sym.cstr}"
			end
		end
		def cstr()
			if comment.nil? then cstr0
			else "#{cstr0} /* #{comment.to_s} */"
			end
		end
		def name() sym.name end
		def filename() @pos ? @pos.first : nil end
		def function?() sym.function? end
		def functype
			"#{declspec.cstr} #{sym.ptrinfo}".strip
		end
		def each_arg &callback
			sym.each_arg(&callback)
		end
	end

	class List < TreeNode
		def initialize() @a = [] end
		def push(arg) @a.push(*arg); self end
		def inspect() @a.inspect end
		def each(&callback) @a.each{|elem| callback.call(elem) } end
		def first() @a.first end
		def last() @a.last end
		def cstr()
			@a.collect{|elem| elem.cstr}.join(', ')
		end
		def name()
			@a.collect{|elem| elem.name}
		end
	end

	class VarArgs < TreeNode
		def initialize
			@comment = nil
		end
		attr_reader :comment
		def cstr() "..." end
		def name() '...' end
		def typename() '\AnyType' end
		def comment=(text)
			@comment = ""
			for line in text
				case line
				when /^\s*\*\s*$/
					@comment << "\n"
				when /^\s+$/
				else
					line.sub!(/\s*\*\s*/, '')
					@comment << "#{line}\n"
				end
			end
		end
	end

end
