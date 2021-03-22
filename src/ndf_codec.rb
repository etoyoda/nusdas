#!/usr/bin/ruby

DREC_HDR = 64

class C

	def self::sizeof(type)
		case type
		when /^(double|N_[US]I8)$/ then 8
		when /^(float|N_[US]I4)$/ then 4
		when /^N_[US]I2$/ then 2
		when /^N_[US]I1$/ then 1
		else raise "unknown type #{type}"
		end
	end

	def self::min(type)
		case type
		when /^double$/ then "-DBL_MAX"
		when /^float$/ then "-FLT_MAX"
		when /^N_UI8$/ then "long_to_si8(0)"
		when /^N_SI8$/ then "make_si8(0x80000000uL, 0uL)"
		when /^N_UI4$/ then "(N_SI4)0"
		when /^N_SI4$/ then "(N_SI4)0x80000000L"
		when /^N_UI2$/ then "(N_UI2)0"
		when /^N_SI2$/ then "(N_SI2)-0x8000"
		when /^N_UI1$/ then "(N_UI1)0"
		else raise "unknown type #{type}"
		end
	end

	def self::max(type)
		case type
		when /^double$/ then "DBL_MAX"
		when /^float$/ then "FLT_MAX"
		when /^N_UI8$/ then "make_si8(0xFFFFFFFFuL, 0xFFFFFFFFuL)"
		when /^N_SI8$/ then "make_si8(0x7FFFFFFFuL, 0xFFFFFFFFuL)"
		when /^N_UI4$/ then "0xFFFFFFFFuL"
		when /^N_SI4$/ then "0x7FFFFFFFL"
		when /^N_UI2$/ then "(N_UI2)0xFFFF"
		when /^N_SI2$/ then "(N_SI2)0x7FFF"
		when /^N_UI1$/ then "(N_UI1)0xFF"
		else raise "unknown type #{type}"
		end
	end

	def self::packmax(type)
		case type
		when /^N_UI4$/ then "0xFFFFFFFCuL"
		when /^N_SI4$/ then "0x7FFFFFFDL"
		when /^N_UI2$/ then "(N_UI2)0xFFFC"
		when /^N_SI2$/ then "(N_SI2)0x7FFD"
		when /^N_UI1$/ then "0xFC"
		else raise "unknown type #{type}"
		end
	end
	
	def self::udfvreplace(type)
		case type
		when /^N_SI4$/ then "0x80000001L"
		when /^N_SI2$/ then "0x8001"
		when /^N_UI2$/ then "0xFFFE"
		when /^N_UI1$/ then "0xFE"
		else raise "unknown type #{type}"
		end
	end
	
	def self::udfvmax(type)
		case type
		when /^N_SI4$/ then "0x7FFFFFFFL"
		when /^N_SI2$/ then "0x7FFF"
		when /^N_UI2$/ then "0xFFFE"
		when /^N_UI1$/ then "0xFE"
		else raise "unknown type #{type}"
		end
	end
	
	def self::udfvmin(type)
		case type
		when /^N_SI4$/ then "0x80000001L"
		when /^N_SI2$/ then "0x8001"
		when /^N_UI2$/ then "0"
		when /^N_UI1$/ then "0"
		else raise "unknown type #{type}"
		end
	end
	
	def self::udfv(type)
		case type
		when /^double$/ then "DBL_MAX"
		when /^float$/ then "FLT_MAX"
		when /^N_SI4$/ then "0x80000000L"
		when /^N_SI2$/ then "0x8000"
		when /^N_UI2$/ then "0xFFFF"
		when /^N_UI1$/ then "0xFF"
		else raise "unknown type #{type}"
		end
	end

	def self::int_of_same_width type
		case type
		when /^float$/ then 'N_UI4'
		when /^double$/ then 'N_UI8'
		else type
		end
	end

	def self::integral? type
		case type
		when /^N_[SU]I[124]$/ then true
		when /^(float|double)$/ then false
		else raise "unknown type <#{type}>"
		end
	end

	def self::floating? type
		case type
		when /^N_[SU]I[124]$/ then false
		when /^(float|double)$/ then true
		else raise
		end
	end

end

class PackType

	def initialize name, elemtype
		# パック方法、パック後の要素型
		@name, @elemtype = name, elemtype
		@scale = nil
	end

	def setscale val
		@scale = val
		self
	end

	def dup
		self.class.new(@name, @elemtype).setscale(@scale)
	end

	attr_reader :elemtype

	# パック方法名称を小文字にしたもの
	def lcname() @name.downcase.strip end
	# パック方法名称を大文字にしたもの
	def ucname() @name.upcase end

	# パック方法名称を大文字にして、４文字に満たない部分を
	# 下線で補ったもの
	def ucname4
		[@name].pack("A4").gsub(/ /, "_").upcase
	end

	def udfv
		C.udfv @elemtype
	end
        def udfvreplace
                C.udfvreplace @elemtype
        end
	def udfvmax
		C.udfvmax @elemtype
	end
	def udfvmin
		C.udfvmin @elemtype
	end

	# パック時に使用する作業変数の宣言
	def baseamp_decl
		"\t#{htype}\tbase, amp;\n"
	end

	# デコード時のパラメータ取得コード
	def baseamp_set(ofs)
		base_ofs = 0
		amp_ofs = base_ofs + C.sizeof(htype)
		(<<-EOC).gsub(/^\s*\|/, "")
		|        PEEK_#{htype}(&base, src + #{base_ofs} + #{ofs});
		|        PEEK_#{htype}(&amp, src + #{amp_ofs} + #{ofs});
		EOC
	end

	def elemint
		C.int_of_same_width @elemtype
	end

	def float_plain?
		case @name
		when /^R[48]/ then true
		else false
		end
	end

	def elemint_ntoh
		case elemint
		when /I8/ then 'NTOH8'
		when /SI4/ then '(N_SI4)NTOH4'
		when /I4/ then 'NTOH4'
		when /SI2/ then '(N_SI2)NTOH2'
		when /I2/ then 'NTOH2'
		when /I1/ then ''
		else raise
		end
	end

	def elemint_hton
		case elemint
		when /I8/ then 'HTON8'
		when /I4/ then 'HTON4'
		when /I2/ then 'HTON2'
		when /I1/ then ''
		else raise
		end
	end

	# パック要素のバイト数
	def elem_nbytes
		C.sizeof(@elemtype)
	end

	# 記録先頭からパック要素先頭までのバイト数
	def head_nbytes
		case @name
		when /^4(PAC|UPC)/ then 16
		when /^[12](PAC|UPC|UPJ|UPP)/ then 8
		when /^RLEN/ then 12
		else 0
		end
	end

	# 最大最小が必要か?
	def maxmin?
		/^([124](PAC|UPC|UPJ|UPP)|I1)/ =~ @name
	end

	# パックパラメタの型
	def htype
		case @name
		when /^4(PAC|UPC)/ then "double"
		when /^[12](PAC|UPC|UPJ|UPP)/ then "float"
		else raise
		end
	end

	# パックパラメタに対応する整数型
	def htype_int
		case @name
		when /^4(PAC|UPC)/ then "N_UI8"
		when /^[12](PAC|UPC|UPJ|UPP)/ then "N_UI4"
		else raise
		end
	end

	# パッキングアルゴリズム種別
	def algorithm
		case @name
		when /^[124](PAC|UPC|UPJ|UPP)/ then :PACK
		when /^RLEN/ then :RLEN
		else :PLAIN
		end
	end

	def maxval
		case @name
		when /^[12](PAC|UPC|UPJ|UPP)/ then C.max('float')
		when /^4(PAC|UPC)/ then C.max('double')
		else C.max(@elemtype)
		end
	end

	def minval
		case @name
		when /^[12](PAC|UPC|UPJ|UPP)/ then C.min('float')
		when /^4(PAC|UPC)/ then C.min('double')
		else C.min(@elemtype)
		end
	end

	# 最大最小からパッキング係数を計算するコード
	def getfactor
		case @name
		when /^[24]PAC/ then (<<-EOC).gsub(/^\s*\|/, "")
		|	base_d = (min0 + max0) * 0.5;
		|	if (min0 == max0) {
		|		amp_d = scale = 1.0;
		|	} else {
		|		#{htype} width;
		|		if ((max0 - base_d) > (base_d - min0)) {
		|			width = max0 - base_d;
		|		} else {
		|			width = base_d - min0;
		|		}
		|		amp_d = width / #{C.packmax @elemtype};
		|		scale = 1.0 / amp_d;
		|	}
		|	amp  = (#{htype})amp_d;
		|	base = (#{htype})base_d;
		EOC
		when /^([124]UPC|1PAC|2UPJ|2UPP)/ then (<<-EOC).gsub(/^\s*\|/, "")
		|	base_d = min0;
		|	if (min0 == max0) {
		|		amp_d = scale = 1.0;
		|	} else {
		|		amp_d = (max0 - base_d) / #{C.packmax @elemtype} ;
		|		scale = 1.0 / amp_d;
		|	}
		|	amp  = (#{htype})amp_d;
		|	base = (#{htype})base_d;
		EOC
		else raise
		end
	end

	# 各要素のパッキングを行うコード
	def get_packed
		case elemint
		when /^N_UI/ then
			"(source[i] - base_d) * scale + 0.5"
		when /^N_SI/ then
			"ROUND((source[i] - base_d) * scale)"
		else raise
		end
	end

	def get_plain
		if @scale
			"(source[i] * #{@scale})"
		else
			"source[i]"
		end
	end

	def ecd_formula
		case algorithm
		when :PACK then get_packed
		when :PLAIN then get_plain
		else raise
		end
	end

	# パック値から利用者値を得る式 (:PACK)
	def get_unpacked
		"(#{elemint})#{elemint_ntoh}(packed[i]) * amp + base"
	end

	def get_unplain
		packed = "packed[i]"
		packed = "#{elemint_ntoh}(packed[i])" if elemint_ntoh != ''
		if @scale
			"#{packed} * #{1.0 / @scale}"
		else
			packed
		end
	end

	# パック値から利用者値を得る式
	def dcd_formula
		case algorithm
		when :PACK then get_unpacked
		when :PLAIN then get_unplain
		else raise
		end
	end

end

class UserType

	def initialize name, ctype
		@name = name
		@ctype = ctype
	end
	attr_reader :ctype

	def lcname() @name.downcase.strip end
	def ucname() @name.upcase end

	def itype
		C.int_of_same_width @ctype
	end
	
	def integral?
		C.integral? @ctype
	end

	def rawdata?
		@name == 'NC'
	end

	def binary?
		@name == 'ND'
	end

	def lsname
		case @name
		when /^R[48]/ then @name.downcase
		when /^I4/ then 'si4'
		when /^I2/ then 'si2'
		when /^I1/ then 'ui1'
		when /^NC/ then 'ui1'
		when /^ND/ then 'ui1'
		else raise
		end
	end
	def maxminzero
		case @name
		when /^R[48]/ then return (<<-EOF).gsub(/^\s*\|/, "")
		|	if (0 == min0) min0 = 0;
		|	if (0 == max0) max0 = 0;
		EOF
		else return ""
		end
	end

	def maxminfunc(funcname)
		(<<-EOF).gsub(/^\s*\|/, "")
		|INLINE void
		|#{funcname}(struct obuffer_t *buf, #{@ctype} *maxp, #{@ctype} *minp@A@)
		|{
		|	N_UI4	i;
		|	const #{@ctype} *source = buf->ob_ptr;
		|#ifndef AVOID_PIPELINE_HACK
		|	#{@ctype} min0 = #{C.max @ctype};
		|	#{@ctype} min1 = #{C.max @ctype};
		|	#{@ctype} min2 = #{C.max @ctype};
		|	#{@ctype} min3 = #{C.max @ctype};
		|	#{@ctype} max0 = #{C.min @ctype};
		|	#{@ctype} max1 = #{C.min @ctype};
		|	#{@ctype} max2 = #{C.min @ctype};
		|	#{@ctype} max3 = #{C.min @ctype};
		|# ifdef USE_OMP
		|#  if USE_OMP >= 31
		|#  pragma omp parallel for private(i) reduction(min:min0,min1,min2,min3) reduction(max:max0,max1,max2,max3)
		|#  endif
		|# else
		|	/*poption parallel tlocal(i)
		|	 min(min0, min1, min2, min3) max(max0, max1, max2, max3) */
		|# endif
		|	for (i = 0; i < (buf->nelems & ~3u); i += 4) {
		|		if (source[i] > max0) max0 = source[i];
		|		if (source[i] < min0) min0 = source[i];
		|		if (source[i+1] > max1) max1 = source[i+1];
		|		if (source[i+1] < min1) min1 = source[i+1];
		|		if (source[i+2] > max2) max2 = source[i+2];
		|		if (source[i+2] < min2) min2 = source[i+2];
		|		if (source[i+3] > max3) max3 = source[i+3];
		|		if (source[i+3] < min3) min3 = source[i+3];
		|	}
		|	/*poption noparallel */
		|	for (i = (buf->nelems & ~3u) ; i < buf->nelems; i++) {
		|		if (source[i] > max0) max0 = source[i];
		|		if (source[i] < min0) min0 = source[i];
		|	}
		|	if (max1 > max0) max0 = max1; if (min1 < min0) min0 = min1;
		|	if (max2 > max0) max0 = max2; if (min2 < min0) min0 = min2;
		|	if (max3 > max0) max0 = max3; if (min3 < min0) min0 = min3;
		|#else
		|	#{@ctype} min0 = #{C.max @ctype};
		|	#{@ctype} max0 = #{C.min @ctype};
		|# ifdef USE_OMP
		|#  if USE_OMP >= 31
		|#  pragma omp parallel for private(i) reduction(min:min0) reduction(max:max0)
		|#  endif
		|# else
		|	/*poption parallel tlocal(i) min(min0) max(max0) */
		|# endif
		|	for (i = 0; i < buf->nelems; i++) {
		|		if (source[i] > V@max0@) max0 = source[i];
		|		if (source[i] < V@min0@) min0 = source[i];
		|	}
		|#endif
		|	*maxp = max0;
		|	*minp = min0;
		|}
		|
		EOF
	end

	def maxminfunc_mask(funcname)
		(<<-EOF).gsub(/^\s*\|/, "")
		|INLINE void
		|#{funcname}(struct obuffer_t *buf, #{@ctype} *maxp, #{@ctype} *minp)
		|{
		|	N_UI4	i;
		|	const #{@ctype} *source = buf->ob_ptr;
		|	const unsigned char *mask_ptr = buf->ob_mask;
		|	#{@ctype} min0 = #{C.max @ctype};
		|	#{@ctype} max0 = #{C.min @ctype};
		|	for (i = 0; i < buf->nelems; i++) {
		|		if (mask_ptr[i / 8] & (128u >> (i % 8))) {
		|			if (source[i] > max0) max0 = source[i];
		|			if (source[i] < min0) min0 = source[i];
		|		}
		|	}
		|	*maxp = max0;
		|	*minp = min0;
		|}
		|
		EOF
	end

end

class MissType

	def initialize name
		@name = name
		raise "bad name #{name}" unless /^\w+$/ === @name
		@sym = eval(":#{name}")
	end
	attr_reader :sym

	def none?() @sym == :NONE end
	def udfv?() @sym == :UDFV end
	def mask?() @sym == :MASK end

	def lcname() @name.downcase end
	def ucname() @name.upcase end

	def offset
		case @sym
		when :MASK then "mask_nbytes"
		when :UDFV then @sym
		else "0"
		end
	end

end

class Codec

	def initialize pack, miss, user
		@pack, @miss, @user = pack.dup, miss, user
		@pack.setscale(nil) if @user.integral?
	end
	attr_reader :user, :pack

	def p4
		$syms.push('-' + @pack.ucname)
		"SYM4_#{@pack.ucname}"
	end

	def m4
		$syms.push('-' + @miss.ucname)
		$syms.push @miss.ucname
		"SYM4_#{@miss.ucname}"
	end

	def u4
		$syms.push('-' + @user.ucname)
		"SYM4_#{@user.ucname}"
	end

	def fixed
		r = 68 + @pack.head_nbytes
		r += @pack.elem_nbytes if @miss.udfv?
		r += 1 if @miss.mask?
		r
	end

	def factor
		r = @pack.elem_nbytes * 8
		r *= 2 if /^(2UPP|2UPJ)$/ =~ @pack.ucname
		r += 1 if @miss.mask?
		r
	end

	def <=> other
		self.ident <=> other.ident
	end

	def ident
		"#{@pack.lcname}_#{@miss.lcname}_#{@user.lcname}"
	end

	def efunc
		"encode_#{ident}"
	end

	def dfunc
		return "NULL" if @user.binary?
		"decode_#{ident}"
	end

	def miss_offset
		ofs = @miss.offset
		ofs = @pack.elem_nbytes if ofs == :UDFV
		return ofs
	end

	def ecd_miss_decl
		if (@miss.none? or @miss.udfv?) then ""
		else
			s = (<<-EOF).gsub(/^\s*\|/, "")
			|	const unsigned char *mask_ptr = buf->ob_mask;
			|	size_t mask_nbytes = (buf->nelems - 1) / 8 + 1;
			|	N_UI4 j;
			EOF
			s.chomp
		end
	end

	def ecd_miss_set
		if @miss.none? then
			"/* missing = NONE */"
		elsif @miss.udfv? then
			"POKE_#{@pack.elemtype}(drec, #{@pack.udfv});"
		else
			(<<-EOF).gsub(/^\s*\|/, "")
			|memcpy(drec, mask_ptr, mask_nbytes);
			|	i = 0;
			EOF
		end
	end

	def undef_u
		"GlobalConfig(pc_missing_#{@user.lsname})"
	end

	def i
		@miss.mask? ? 'j' : 'i'
	end

	def ecd_formula
		formula = @pack.ecd_formula
		if @miss.none? || @miss.udfv? then
			formula
		else
			formula.gsub(/\bi\b/, 'j')
		end
	end

	def ecd_assign_core
		if @pack.float_plain? then
			if @miss.mask?
				simple = (<<-EOA).gsub(/^\s*\|/, "")
				|#{@pack.elemtype} pval;
				|			pval = #{ecd_formula};
				|			memcpy#{C.sizeof @pack.elemtype}((char *)(packed + i), (const char *)&pval);
				EOA
				for_pval = ecd_formula
			elsif @miss.udfv?
				for_pval = (<<-EOF).gsub(/^\s*\|/, "")
				|
				|				(source[i] == #{undef_u})
				|				? #{@pack.udfv}
				|				: (#{@pack.elemtype})(#{ecd_formula})
				EOF
				for_pval.chomp!
				simple = "packed[i] = (#{for_pval});"
			else
				simple = "packed[i] = (#{ecd_formula});"
				for_pval = ecd_formula
			end
			(<<-EOF).gsub(/^\s*\|/, "")
			|
			|#if WORDS_BIGENDIAN
			|			#{simple}
			|#else
			|			#{@pack.elemtype} pval;
			|			pval = (#{for_pval});
			|			POKE_#{@pack.elemtype}(&packed[i], pval);
			|#endif
			EOF
		else
            if ['N_SI2', 'N_UI2'].include?(@pack.elemtype) && @miss.mask?
			(<<-EOF).gsub(/^\s*\|/, "")
			|#{@pack.elemtype} pval;
			|			pval = (#{ecd_formula});
			|#if NEED_ALIGN & 2
			|			POKE_#{@pack.elemtype}(packed + i, pval);
			|#else
			|			packed[i] = #{@pack.elemint_hton}(pval);
			|#endif
			EOF
            elsif @pack.elemtype == 'N_SI4'&& @miss.mask?
			(<<-EOF).gsub(/^\s*\|/, "")
			|#{@pack.elemtype} pval;
			|			pval = (#{ecd_formula});
			|#if NEED_ALIGN & 4
			|			POKE_#{@pack.elemtype}(packed + i, pval);
			|#else
			|			packed[i] = #{@pack.elemint_hton}(pval);
			|#endif
			EOF
            elsif @miss.udfv?
                if :PACK != @pack.algorithm
			(<<-EOF).gsub(/^\s*\|/, "")
			|#{@pack.elemtype} pval;
			|			if ( source[i] == #{undef_u} ) {
			|				pval = (#{@pack.elemtype})#{@pack.udfv};
			|			} else {
			|				pval = (#{@pack.elemtype})(#{ecd_formula});
			|				if ( (#{@pack.elemtype})#{@pack.udfv} == pval ) pval = (#{@pack.elemtype})#{@pack.udfvreplace};
			|			}
			|			packed[i] = #{@pack.elemint_hton}(pval);
			EOF
                else
			(<<-EOF).gsub(/^\s*\|/, "")
			|#{@pack.elemtype} pval;
			|			if ( source[i] == #{undef_u} ) {
			|				pval = (#{@pack.elemtype})#{@pack.udfv};
			|			} else {
			|				double dval = #{ecd_formula};
			|				if ( dval > (#{@pack.elemtype})#{@pack.udfvmax} ) dval = (#{@pack.elemtype})#{@pack.udfvmax};
			|				else if ( dval < (#{@pack.elemtype})#{@pack.udfvmin} ) dval = (#{@pack.elemtype})#{@pack.udfvmin};
			|				pval = (#{@pack.elemtype})(dval);
			|			}
			|			packed[i] = #{@pack.elemint_hton}(pval);
			EOF
                end
            else
			(<<-EOF).gsub(/^\s*\|/, "")
			|#{@pack.elemtype} pval;
			|			pval = (#{ecd_formula});
			|			packed[i] = #{@pack.elemint_hton}(pval);
			EOF
            end
		end
	end

	def ecd_mask_core
		if @miss.mask? then 
			text = <<EOF
if (mask_ptr[j / 8] & (128 >> (j % 8))) {
			#{ecd_assign_core}
			i++;
		}
EOF
			text.sub(/^\s+#/, '#')
		else ecd_assign_core.gsub(/^\t/, '')
		end
	end

	def narrowing?
		case @pack.ucname
		when /^4(UPC|PAC)$/ then false
		when /^[12](UPC|PAC|UPJ|UPP)$/ then @user.ctype == 'double'
		when /^I4/ then /(float|double)/ === @user.ctype
		when /^(N1I2|I2)/ then /(N_SI4|float|double)/ === @user.ctype
		when /^(I1|RLEN)/ then /(N_SI[24]|float|double)/ === @user.ctype
		else raise "unknown packing=#{@pack.ucname}"
		end
	end

	def parallel
		if @miss.mask?
			"\t/*poption noparallel */"
		else
			(<<-EOF).gsub(/^\s*\|/, "")
			|#ifdef USE_OMP
			|#pragma omp parallel for
			|#else
			|	/*poption parallel */
			|#endif
			EOF
		end
	end
	def bufnelems
		if @miss.mask?
			"i"
		else
			"buf->nelems"
		end
	end

	def ecd_maxmincheck
		text = ''
		if @miss.mask?
			text += (<<-EOF).gsub(/^\s*\|/, "")
			|	if (buf->ob_mask == NULL) {
			|		return NUSERR_WR_MaskMissing;
			|	}
			EOF
		end
		unless @pack.float_plain?
			text += (<<-EOF).gsub(/^\s*\|/, "")
			|	#{maxmin_name}(buf, &max0, &min0);
			EOF
			if narrowing?
				text += (<<-EOF).gsub(/^\s*\|/, "")
				|	if ((max0 > #{@pack.maxval}) || (min0 < #{@pack.minval})) {
				|		return nus_err((NUSERR_WR_EncodeFail, "data (%g:%g) out of "
				|			"range of packing=#{@pack.ucname}",
				|			(double)min0, (double)max0));
				|	}
				EOF
			end
		end
		text
	end

	def ecd_pack
		return ecd_pack_raw if @user.rawdata?
		return ecd_binary if @user.binary?
		case @pack.lcname
		when '2upp' then return ecd_pack_cpsd('cpsd')
		when '2upj' then return ecd_pack_cpsd('jp2k')
		end
		base_ofs = 0
		amp_ofs = base_ofs + C.sizeof(@pack.htype)
		pack_ofs = base_ofs + C.sizeof(@pack.htype) * 2
		recl_ofs = pack_ofs + 4
		text = (<<-EOF).gsub(/^\s*\|/, "")
		|	N_UI4	i;
		|	const #{@user.ctype} *source = buf->ob_ptr;
		|	#{@pack.elemint} *packed;
		|	#{@pack.htype} base, amp;
		|	#{@user.ctype} max0;
		|	#{@user.ctype} min0;
		|	double scale, base_d, amp_d;
		|#{ecd_miss_decl}
		|	/* code */
		|#{ecd_maxmincheck}
		|#{@user.maxminzero}
		|#{@pack.getfactor}
		|	#{ecd_miss_set}
		|	POKE_#{@pack.htype}(drec + #{base_ofs} + #{miss_offset}, base);
		|	POKE_#{@pack.htype}(drec + #{amp_ofs} + #{miss_offset}, amp);
		|	packed = (#{@pack.elemint} *)(drec + #{pack_ofs} + #{miss_offset});
		|#{parallel}
		|	for (#{i} = 0; #{i} < buf->nelems; #{i}++) {
		|		#{ecd_mask_core}
		|	}
		|	return #{recl_ofs} + #{miss_offset} + #{bufnelems} * #{C.sizeof @pack.elemint};
		EOF
	end

	# complex packing with spatial differencing
	def ecd_pack_cpsd cpsd
		base_ofs = 0
		amp_ofs = base_ofs + C.sizeof(@pack.htype)
		pack_ofs = base_ofs + C.sizeof(@pack.htype) * 2
		recl_ofs = pack_ofs + 4
		x = "nxd"
		y = "nyd"
		nelems = "buf->nelems"
		if @miss.lcname == "mask"
			x = "i"
			y = "1"
			nelems = "i"
		end
		text = (<<-EOF).gsub(/^\s*\|/, "")
		|	N_UI4	i;
		|	const #{@user.ctype} *source = buf->ob_ptr;
		|	#{@pack.elemint} *packed;
		|	#{@pack.htype} base, amp;
		|	#{@user.ctype} max0;
		|	#{@user.ctype} min0;
		|	double scale, base_d, amp_d;
		|	unsigned char *dptr;
		|	long rlen;
		|#{ecd_miss_decl}
		|	/* code */
		|#{ecd_maxmincheck}
		|#{@user.maxminzero}
		|#{@pack.getfactor}
		|	#{ecd_miss_set}
		|	POKE_#{@pack.htype}(drec + #{base_ofs} + #{miss_offset}, base);
		|	POKE_#{@pack.htype}(drec + #{amp_ofs} + #{miss_offset}, amp);
		|	packed = (#{@pack.elemint} *)nus_malloc(sizeof(#{@pack.elemint})*buf->nelems);
		|#{parallel}
		|	for (#{i} = 0; #{i} < buf->nelems; #{i}++) {
		|		#{ecd_mask_core}
		|	}
		|	dptr = drec + #{pack_ofs} + #{miss_offset};
		|	rlen = nus_encode_#{cpsd}((const unsigned char *)packed, #{x}, #{y}, dptr, sizeof(#{@pack.elemint})*buf->nelems*2);
		|	nus_free(packed);
		|	if (rlen < 0) { return rlen; }
		|	return #{recl_ofs} + #{miss_offset} + rlen;
		EOF
	end

	def ecd_pack_raw
		base_ofs = 0
		amp_ofs = base_ofs + C.sizeof(@pack.htype)
		pack_ofs = base_ofs + C.sizeof(@pack.htype) * 2
		recl_ofs = pack_ofs + 4
                ecd = case @pack.lcname
			when '2upj' then 'nus_encode_jp2k'
			when '2upp' then 'nus_encode_cpsd'
			else nil
			end
		text = ""
		text += (<<-EOF).gsub(/^\s*\|/, "")
		|	N_UI4	i;
		|	const #{@pack.elemint} *source = buf->ob_ptr;
		|	const #{@pack.htype} *baseamp;
		|	#{@pack.elemint} *packed;
		EOF
		if ecd then
			text += (<<-EOF).gsub(/^\s*\|/, "")
			|	unsigned char *dptr;
			|	long rlen;
			EOF
		end
		if @miss.udfv?
			text += (<<-EOF).gsub(/^\s*\|/, "")
			|	POKE_N_UI2(drec, 0xFFFF);
			EOF
		end
		text += (<<-EOF).gsub(/^\s*\|/, "")
		|	/* code */
		|	baseamp = (#{@pack.htype} *)((N_UI1 *)buf->ob_ptr + #{miss_offset});
		|	POKE_#{@pack.htype}(drec + #{base_ofs} + #{miss_offset}, baseamp[0]);
		|	POKE_#{@pack.htype}(drec + #{amp_ofs} + #{miss_offset}, baseamp[1]);
		|	source = (const #{@pack.elemint} *)(baseamp + 2);
		EOF
		if ecd then
			text += (<<-EOF).gsub(/^\s*\|/, "")
			|	packed = (#{@pack.elemint} *)nus_malloc(sizeof(#{@pack.elemint})*buf->nelems);
			EOF
		else
			text += (<<-EOF).gsub(/^\s*\|/, "")
			|	packed = (#{@pack.elemint} *)(drec + #{pack_ofs} + #{miss_offset});
			EOF
		end
			text += (<<-EOF).gsub(/^\s*\|/, "")
			|#{parallel}
			|	for (#{i} = 0; #{i} < buf->nelems; #{i}++) {
			|		packed[#{i}] = NTOH#{C.sizeof @pack.elemint}(source[#{i}]);
			|	}
			EOF
		if ecd then
			text += (<<-EOF).gsub(/^\s*\|/, "")
			|	dptr = drec + #{pack_ofs} + #{miss_offset};
			|	rlen = #{ecd}((const unsigned char *)packed, nxd, nyd, dptr, sizeof(#{@pack.elemint})*buf->nelems*2);
			|	nus_free(packed);
			|	if (rlen < 0) { return rlen; }
			|	return #{recl_ofs} + #{miss_offset} + rlen;
			EOF
		else
			text += (<<-EOF).gsub(/^\s*\|/, "")
			|	return #{recl_ofs} + #{miss_offset} + buf->nelems * #{C.sizeof @pack.elemint};
			EOF
		end
		text
	end
=begin
	def ecd_raw
		base_ofs = 0
		amp_ofs = base_ofs + C.sizeof(@pack.htype)
		pack_ofs = base_ofs + C.sizeof(@pack.htype) * 2
		recl_ofs = pack_ofs + 4
		elemint = @pack.elemint
		(<<-EOF).gsub(/^\s*\|/, "")
		|	N_UI4	i;
		|	const #{elemint}	*pdata = (const #{elemint} *)((const char *)buf->ob_ptr + 8);
		|	const float	*hdata = buf->ob_ptr;
		|	#{elemint}	*packed = (#{elemint} *)(drec + #{pack_ofs} + #{miss_offset});
		|	POKE_float(drec + #{base_ofs} + #{miss_offset}, hdata[0]);
		|	POKE_float(drec + #{amp_ofs} + #{miss_offset}, hdata[1]);
		|	for (#{i} = 0; #{i} < buf->nelems; #{i}++) {
		|		POKE_#{@pack.elemint}(packed + #{i}, pdata[#{i}]);
		|	}
		|	return #{recl_ofs} + #{miss_offset} + 8 + buf->nelems * #{C.sizeof @pack.elemint};
		EOF
	end
=end
	def ecd_plain_maxmindecl
		if @pack.float_plain?
			""
		else
			(<<-EOF).gsub(/^\s*\|/, "")
			|	#{@user.ctype} max0;
			|	#{@user.ctype} min0;
			EOF
		end
	end

	def ecd_plain
		return ecd_plain_raw if @user.rawdata?
		return ecd_binary if @user.binary?
		pack_ofs = 0
		recl_ofs = pack_ofs + 4
		text = (<<-EOF).gsub(/^\s*\|/, "")
		|	N_UI4	i;
		|	const #{@user.ctype} *source = buf->ob_ptr;
		|	#{@pack.elemtype} *packed;
		|#{ecd_plain_maxmindecl}
		|#{ecd_miss_decl}
		|	/* code */
		|#{ecd_maxmincheck}
		|	#{ecd_miss_set}
		|	packed = (#{@pack.elemtype} *)(drec + #{pack_ofs} + #{miss_offset});
		|#{parallel}
		|	for (#{i} = 0; #{i} < buf->nelems; #{i}++) {
		|		#{ecd_mask_core}
		|	}
		|	return #{recl_ofs} + #{miss_offset} + #{bufnelems} * #{C.sizeof @pack.elemint};
		EOF
	end

	def ecd_rlen
		return ecd_rlen_raw if @user.rawdata?
		return ecd_binary if @user.binary?
		nbit_ofs = 0
		maxv_ofs = nbit_ofs + 4
		cbyte_ofs = maxv_ofs + 4
		pack_ofs = cbyte_ofs + 4
		recl_ofs = pack_ofs + 4
		(<<-EOF).gsub(/^\s*\|/, "")
		|	N_SI4	maxv;
		|	N_SI4 cmpr_nbytes;
		|	cmpr_nbytes = nus_compress_rlen_#{@user.lcname}(buf->ob_ptr, buf->nelems, &maxv, 
		|		drec + #{pack_ofs}, buf->nelems);
		|	if (cmpr_nbytes < 0) {
		|		return cmpr_nbytes;
		|	}
		|	POKE_N_UI4(drec + #{nbit_ofs}, 8u);
		|	POKE_N_UI4(drec + #{maxv_ofs}, maxv);
		|	POKE_N_UI4(drec + #{cbyte_ofs}, cmpr_nbytes);
		|	return #{recl_ofs} + cmpr_nbytes;
		EOF
	end

	def ecd_binary
		text = (<<-EOF).gsub(/^\s*\|/, "")
		|	N_UI4 dat_x, dat_y, dat_size, expect_size;
		|	char pack[5], miss[5];
		|	const unsigned char* src = (const unsigned char*)buf->ob_ptr;
		|	if ( 16 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size too small %d", buf->nelems));
		|	dat_x = PEEK_N_UI4(src);
		|	dat_y = PEEK_N_UI4(src + 4);
		|	memcpy(pack, src + 8, 4);
		|	memcpy(miss, src + 12, 4);
		|	pack[4] = miss[4] = 0;
		|	if ( dat_x != nxd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.x:%d != def.x:%d", dat_x, nxd));
		|	if ( dat_y != nyd ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.y:%d != def.y:%d", dat_y, nyd));
		|	if ( strcmp(pack, "#{@pack.ucname}") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.pack:%s != def.pack:#{@pack.ucname}", pack));
		|	if ( strcmp(miss, "#{@miss.ucname}") ) return nus_err((NUSERR_WR_EncodeFail, "ND invalid: data.miss:%s != def.miss:#{@miss.ucname}", miss));
		EOF
		if @miss.mask?
			text += (<<-EOF).gsub(/^\s*\|/, "")
			|	int i;
			|	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
			|	for (dat_size = i = 0; i < nxd * nyd; ++i) if (src[16 + i / 8] & (128 >> (i % 8))) ++dat_size;
			EOF
		else
			text += (<<-EOF).gsub(/^\s*\|/, "")
			|	dat_size = nxd * nyd;
			EOF
		end
		case @pack.ucname
		when "RLEN"
			text += (<<-EOF).gsub(/^\s*\|/, "")
			|	N_UI4 rlen_nbit = PEEK_N_UI4(src + 16 + #{miss_offset});
			|	N_UI4 rlen_num = PEEK_N_UI4(src + 16 + #{miss_offset} + 8);
			|	expect_size = 16 + #{miss_offset} + #{@pack.head_nbytes} + (rlen_nbit * rlen_num - 1) / 8 + 1;
			EOF
			text += ecd_binary_expect_size
		when "2UPP"
			text += (<<-EOF).gsub(/^\s*\|/, "")
			|	int j, width;
			|	expect_size = 16 + #{miss_offset} + #{@pack.head_nbytes};
			|	N_UI4 expect_grid = PEEK_N_UI4(src + expect_size);
			|	if (expect_grid != dat_size) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPP: data.size:%d != 2upp.size:%d", dat_size, expect_grid));
			|	N_UI4 n_g = 1 + (dat_size - 1) / 32;
			|	expect_size += 4 + 2 * n_g;
			|	N_UI4 packed_size = 0;
			|	for (j = 0; j < n_g; ++j) {
			|		width = 1 + ((src[expect_size + j / 2] >> (0 == j % 2 ? 4 : 0)) & 15);
			|		if (j == n_g - 1 && 0 != dat_size % 32) {
			|			packed_size += (((dat_size % 32) * width + 7) / 8 + 3) / 4 * 4;
			|		} else {
			|			packed_size += 4 * width;
			|		}
			|	}
			|	expect_size += (n_g - 1) / 2 + 1 + packed_size;
			EOF
			text += ecd_binary_expect_size
		when "2UPJ"
			text += (<<-EOF).gsub(/^\s*\|/, "")
			|	expect_size = buf->nelems;
			|	N_UI4 offset = 16 + #{miss_offset} + #{@pack.head_nbytes};
			|	if ( offset + 24 > buf->nelems ) return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size for 2UPJ too small %d", buf->nelems));
			|	if (memcmp("\\xff\\x4f\\xff\\x51", src + offset, 4)) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPJ: no JPEG2000 header"));
			|	if (memcmp("\\xff\\xd9", src + buf->nelems - 2, 2)) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPJ: no JPEG2000 footer"));
			|	N_UI4 jas_x = PEEK_N_UI4(src + offset + 8);
			|	N_UI4 jas_y = PEEK_N_UI4(src + offset + 12);
			|	N_UI4 jas_x0 = PEEK_N_UI4(src + offset + 16);
			|	N_UI4 jas_y0 = PEEK_N_UI4(src + offset + 20);
			|	N_SI8 expect_grid = (jas_x - jas_x0) * (jas_y - jas_y0);
			|	if (jas_x0 >= jas_x) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPJ: 2upj.x0:%d >= 2upj.x:%d", jas_x0, jas_x));
			|	if (jas_y0 >= jas_y) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPJ: 2upj.y0:%d >= 2upj.y:%d", jas_y0, jas_y));
			|	if (expect_grid != dat_size) return nus_err((NUSERR_WR_EncodeFail, "ND invalid 2UPJ: data.size:%d != 2upj.size:%d", dat_size, expect_grid));
			EOF
		else
			text += (<<-EOF).gsub(/^\s*\|/, "")
			|	expect_size = 16 + #{miss_offset} + #{@pack.head_nbytes} + #{C.sizeof(@pack.elemtype)} * dat_size;
			EOF
			text += ecd_binary_expect_size
		end
		text += (<<-EOF).gsub(/^\s*\|/, "")
		|	memcpy(drec, src + 16, expect_size - 16);
		|	return 4 + expect_size - 16;
		EOF
	end
	def ecd_binary_expect_size
		return (<<-EOF).gsub(/^\s*\|/, "")
		|	if (expect_size > buf->nelems) {
		|		return nus_err((NUSERR_WR_SmallBuf, "ND invalid: data.size:%d < expect_size:%d", buf->nelems, expect_size));
		|	} else if (expect_size < buf->nelems) {
		|		nus_warn(("ND invalid: data.size:%d > expect_size:%d", buf->nelems, expect_size));
		|		buf->nelems = expect_size;
		|	}
		EOF
	end

	def ecd_content
		case @pack.algorithm
		when :PACK then ecd_pack
		when :RLEN then ecd_rlen
		when :PLAIN then ecd_plain
		else raise
		end
	end

	def print_encoder io
		io.print (<<-EOC).gsub(/^\s*\|/, "")
		|static long
		|#{efunc}(unsigned char *drec, struct obuffer_t *buf, N_UI4 nxd, N_UI4 nyd)
		|{
		EOC
		text = ecd_content
		text.gsub!(/( \+ 0)+([^.])/, "\\2")
		io.print text.split(/\n/).grep(/\S/).join("\n") + "\n"
		io.print "}\n\n"
	end

	def dcd_missval_decl
		ofs = 0
		case @miss.sym
		when :UDFV then (<<-EOF).gsub(/^\s*\|/, "")
		|	#{@pack.elemtype} missval;
		EOF
		when :NONE then ""
		when :MASK then (<<-EOF).gsub(/^\s*\|/, "")
		|	N_UI4		j;
		|	const unsigned char *mask_ptr = src + #{ofs};
		|	size_t mask_nbytes = (nxd * nyd - 1) / 8 + 1;
		EOF
		else raise
		end
	end

	def dcd_missval_set
		case @miss.sym
		when :MASK then (<<-EOF).gsub(/^\s*\|/, "")
		|	i = 0;
		EOF
		when :UDFV then
			ofs = 0
			if @pack.float_plain? then (<<-EOF).gsub(/^\s*\|/, "")
			|	*(#{@pack.elemint} *)&missval = #{@pack.elemint_ntoh}(*(#{@pack.elemint} *)(src + #{ofs}));
			EOF
			else (<<-EOF).gsub(/^\s*\|/, "")
			|	missval = #{@pack.elemint_ntoh}(*(#{@pack.elemint} *)(src + #{ofs}));
			EOF
			end
		when :NONE then ""
		else raise
		end
	end

	def need_round?
		@user.integral? and \
		(C.floating?(@pack.elemtype) or (@pack.algorithm == :PACK))
	end

	def need_sign?
		/^(I2  |N1I2)$/ =~ @pack.ucname \
		and @user.ctype == 'N_SI4'
	end

	def dcd_formula
		formula = @pack.dcd_formula
		formula = "(#{@pack.elemtype})#{formula}" if need_sign?
		formula = "ROUND(#{formula})" if need_round?
		if @miss.none? then
			formula
		elsif @miss.udfv? then
			s = (<<-EOF).gsub(/^\s*\|/, "")
			|((#{@pack.elemint})#{@pack.elemint_ntoh}(packed[i]) == missval)
			|				? #{undef_u}
			|				: (#{@user.ctype})(#{formula})
			EOF
			s.chomp
		else
			formula
		end
	end

	def dcd_assign_core0 opts = {}
		if @pack.float_plain? then
			formula = dcd_formula
			formula.gsub!( \
				/(\(\w+\))?NTOH\d\(packed\[i\]\)/, 'pval');
			(<<-EOF).gsub(/^\s*\|/, "")
			|			#{@pack.elemtype} pval;
			|			PEEK_#{@pack.elemtype}(&pval, (unsigned char *)(&packed[i]));
			|			result[#{i}] = #{formula};
			EOF
		elsif opts[:hack2] and [2, 4].include?(@pack.elem_nbytes) then
			formula = dcd_formula
			formula.gsub!( \
				/(\(\w+\))?NTOH\d\(packed\[i\]\)/, 'pval');
			(<<-EOF).gsub(/^\s*\|/, "")
			|#if NEED_ALIGN & #{@pack.elem_nbytes}
			|			#{@pack.elemtype} pval;
			|			pval = PEEK_#{@pack.elemtype}((unsigned char *)(packed + i));
			|			result[#{i}] = #{formula};
			|#else
			|			result[#{i}] = #{dcd_formula};
			|#endif
			EOF
		else (<<-EOF).gsub(/^\s*\|/, "")
			|			result[#{i}] = #{dcd_formula};
			EOF
		end
	end

	def dcd_assign_core
		if @pack.float_plain? then
			formula = dcd_formula
			formula.gsub!( \
				/(\(\w+\))?NTOH\d\(packed\[i\]\)/, 'pval');
			(<<-EOF).gsub(/^\s*\|/, "")
			|	for (#{i} = 0; #{i} < nelems; #{i}++) {
			|			#{@pack.elemtype} pval;
			|			PEEK_#{@pack.elemtype}(&pval, (unsigned char *)(&packed[i]));
			|			result[#{i}] = #{formula};
			|		}
			EOF
		else 
			(<<-EOF).gsub(/^\s*\|/, "")
			|
			|#ifndef AVOID_PIPELINE_HACK
			|#{parallel}
			|		for (#{i} = 0; #{i} < (nelems & ~1u); #{i} += 2) {
			|			result[#{i}] = #{dcd_formula};
			|			result[#{i} + 1] = #{dcd_formula.gsub("\[#{i}\]", "\[#{i} + 1\]")};
			|		}
			|		if ((nelems & 1u) != 0) {
			|			result[nelems - 1] = #{dcd_formula.gsub("\[#{i}\]", "\[nelems - 1\]")};
			|		}
			|#else
			|#{parallel}
			|		for (#{i} = 0; #{i} < nelems; #{i}++) {
			|			result[#{i}] = #{dcd_formula};
			|		}
			|#endif
			EOF
		end
	end

	def dcd_mask_core opts = {}
		if @miss.mask? then (<<-EOF).gsub(/^\s*\|/, "")
			|#{parallel}
			|	for (#{i} = 0; #{i} < nelems; #{i}++) {
			|		if (mask_ptr[j / 8] & (0x80 >> (j % 8))) {
			|#{dcd_assign_core0(opts)}
			|			i++;
			|		} else {
			|			result[#{i}] = #{undef_u};
			|		}
			|	}
			EOF
		else
			dcd_assign_core.gsub(/^\t/, '')
		end
	end

	def dcd_pack_cpsd cpsd
		base_ofs = 0
		pack_ofs = base_ofs + C.sizeof(@pack.htype) * 2
		text = (<<-EOC).gsub(/^\s*\|/, "")
		|	const #{@pack.elemint}	*packed;
		|	#{@user.ctype}	*result;
		|	N_UI4		i, nelems;
		|	const unsigned char *sptr;
		|	long ret;
		|	unsigned char *dptr;
		|#{@pack.baseamp_decl}
		|#{dcd_missval_decl}
		|	/* code */
		|#{dcd_missval_set}
		|#{@pack.baseamp_set(miss_offset)}
		|	nelems = nxd * nyd;
		|	if (buf->nelems < nelems) {
		|		return nus_err((NUSERR_RD_SmallBuf,
		|			"buffer %Pu elements < record %Pu elements",
		|			buf->nelems, nelems));
		|	}
		|	dptr = (unsigned char *)nus_malloc(sizeof(#{@pack.elemint})*nelems);
		|	sptr = src + #{pack_ofs} + #{miss_offset};
		EOC
		if cpsd == "jp2k" || cpsd== "nus_decode_jp2k"
			text += (<<-EOC).gsub(/^\s*\|/, "")
			|	ret = nus_decode_#{cpsd}(sptr, dptr, PEEK_N_UI4(src - 64) -60 -#{pack_ofs} - #{miss_offset});
			EOC
		else
			text += (<<-EOC).gsub(/^\s*\|/, "")
			|	ret = nus_decode_#{cpsd}(sptr, dptr, sizeof(#{@pack.elemint})*nelems*2);
			EOC
		end
		text += (<<-EOC).gsub(/^\s*\|/, "")
		|	if (ret < 0) {
		|		nus_free(dptr);
		|		return ret;
		|	}
		|	packed = (#{@pack.elemint} *)dptr;
		|	result = (#{@user.ctype} *)(buf->ib_ptr);
		|	#{dcd_mask_core}
		|	nus_free(dptr);
		|	return nelems;
		|
		EOC
	end

	def dcd_pack
		return dcd_pack_raw if @user.rawdata?
		return dcd_binary if @user.binary?
		case @pack.lcname
		when '2upp' then return dcd_pack_cpsd('cpsd')
		when '2upj' then return dcd_pack_cpsd('jp2k')
		end
		base_ofs = 0
		pack_ofs = base_ofs + C.sizeof(@pack.htype) * 2
		text = (<<-EOC).gsub(/^\s*\|/, "")
		|#{dcd_packedptr_decl}
		|	#{@user.ctype}	*result;
		|	N_UI4		i, nelems;
		|#{@pack.baseamp_decl}
		|#{dcd_missval_decl}
		|	/* code */
		|#{dcd_missval_set}
		|#{@pack.baseamp_set(miss_offset)}
		|	nelems = nxd * nyd;
		|	if (buf->nelems < nelems) {
		|		return nus_err((NUSERR_RD_SmallBuf,
		|			"buffer %Pu elements < record %Pu elements",
		|			buf->nelems, nelems));
		|	}
		|#{dcd_packedptr_set(pack_ofs)}
		|	result = (#{@user.ctype} *)(buf->ib_ptr);
		|	#{dcd_mask_core(:hack2 => true)}
		|	return nelems;
		EOC
	end

	def dcd_pack_raw
		base_ofs = 0
		pack_ofs = base_ofs + C.sizeof(@pack.htype) * 2
		dcd = case @pack.lcname
			when '2upj' then 'nus_decode_jp2k'
			when '2upp' then 'nus_decode_cpsd'
			else nil
			end
		text = ""
		text += (<<-EOC).gsub(/^\s*\|/, "")
		|	#{@pack.elemint}	*result_packed;
		|	#{@pack.htype_int}	*result_baseamp;
		|	const #{@pack.elemint}	*packed;
		|	const #{@pack.htype_int} *baseamp;
		|	N_UI4		i, nelems;
		EOC
		if dcd then
			text += (<<-EOC).gsub(/^\s*\|/, "")
			|	unsigned char *dptr;
			|	const unsigned char *sptr;
			|	long ret;
			EOC
		end
		text += (<<-EOC).gsub(/^\s*\|/, "")
		|	/* code */
		|	nelems = nxd * nyd;
		|	if (buf->nelems < nelems) {
		|		return nus_err((NUSERR_RD_SmallBuf,
		|			"buffer %Pu elements < record %Pu elements",
		|			buf->nelems, nelems));
		|	}
		|	baseamp = (const #{@pack.htype_int} *)(src + #{base_ofs} + #{miss_offset});
		|	result_baseamp = (#{@pack.htype_int} *)((N_UI1 *)buf->ib_ptr + #{miss_offset});
		|#if NEED_ALIGN & 4
		|	memcpy(result_baseamp, baseamp, 8);
		|	endian_swab4(result_baseamp, 2);
		|#else
		|	result_baseamp[0] = NTOH4(baseamp[0]);
		|	result_baseamp[1] = NTOH4(baseamp[1]);
		|#endif
		EOC
		if dcd then
			text += (<<-EOC).gsub(/^\s*\|/, "")
			|	dptr = (unsigned char *)nus_malloc(sizeof(#{@pack.elemint})*nelems);
			|	sptr = src + #{pack_ofs} + #{miss_offset};
			EOC
			if dcd == 'nus_decode_jp2k'
				text += (<<-EOC).gsub(/^\s*\|/, "")
				|	ret = #{dcd}(sptr, dptr, PEEK_N_UI4(src - 64) -68);
				EOC
			else
				text += (<<-EOC).gsub(/^\s*\|/, "")
				|	ret = #{dcd}(sptr, dptr, sizeof(#{@pack.elemint})*nelems*2);
				EOC
			end
			text += (<<-EOC).gsub(/^\s*\|/, "")
			|	if (ret < 0) {
			|		nus_free(dptr);
			|		return ret;
			|	}
			|	packed = (#{@pack.elemint} *)dptr;
			EOC
		else
			text += (<<-EOC).gsub(/^\s*\|/, "")
			|	packed = (#{@pack.elemint} *)(src + #{pack_ofs} + #{miss_offset});
			EOC
		end
		text += (<<-EOC).gsub(/^\s*\|/, "")
		|	result_packed = (#{@pack.elemint} *)((N_UI1 *)buf->ib_ptr + #{2 * C.sizeof(@pack.htype)} + #{miss_offset});
		|#{parallel}
		|	for (#{i} = 0; #{i} < nelems; #{i}++) {
		|		result_packed[#{i}] = NTOH#{C.sizeof @pack.elemint}(packed[#{i}]);
		|	}
		EOC
		text += "\tnus_free(dptr);\n" if dcd
		text += "\treturn nelems;\n"
		text
	end

	def dcd_packedptr_decl
		(<<-EOC).gsub(/^\s*\|/, "")
		|	const #{@pack.elemint}	*packed;
		EOC
	end

	def dcd_packedptr_set(pack_ofs)
		(<<-EOC).gsub(/^\s*\|/, "")
		|	packed = (#{@pack.elemint} *)(src + #{pack_ofs} + #{miss_offset});
		EOC
	end

	def dcd_plain
		return dcd_plain_raw if @user.rawdata?
		return dcd_binary if @user.binary?
		pack_ofs = 0
		text = (<<-EOC).gsub(/^\s*\|/, "")
		|#{dcd_packedptr_decl}
		|	#{@user.ctype}	*result;
		|	N_UI4		i, nelems;
		|#{dcd_missval_decl}
		|	/* code */
		|#{dcd_missval_set}
		|	nelems = nxd * nyd;
		|	if (buf->nelems < nelems) {
		|		return nus_err((NUSERR_RD_SmallBuf,
		|			"buffer %Pu elements < record %Pu elements",
		|			buf->nelems, nelems));
		|	}
		|#{dcd_packedptr_set(pack_ofs)}
		|	result = (#{@user.ctype} *)(buf->ib_ptr);
		|	#{dcd_mask_core(:hack2 => true)}
		|	return nelems;
		EOC
	end

	def dcd_rlen
		return dcd_rlen_raw if @user.rawdata?
		nbit_ofs = 0
		maxv_ofs = nbit_ofs + 4
		cbyte_ofs = maxv_ofs + 4
		pack_ofs = cbyte_ofs + 4
		(<<-EOF).gsub(/^\s*\|/, "")
		|	N_UI4	maxv, cmpr_nbytes;
		|	N_SI4   nelems;
		|	if (buf->nelems < nxd * nyd) {
		|		return nus_err((NUSERR_RD_SmallBuf,
		|			"buffer %Pu elements < record %Pu elements",
		|			buf->nelems, nxd * nyd));
		|	}
		|	if (PEEK_N_UI4(src + #{nbit_ofs}) != 8u) {
		|		return nus_err((NUSERR_RD_NoCodec,
		|			"cannot uncompress %Pu-bit RLEN data",
		|			PEEK_N_UI4(src + #{nbit_ofs})));
		|	}
		|	maxv = PEEK_N_UI4(src + #{maxv_ofs});
		|	cmpr_nbytes = PEEK_N_UI4(src + #{cbyte_ofs});
		|	nelems = nus_decompress_rlen_#{@user.lcname}(src + #{pack_ofs}, maxv, cmpr_nbytes,
		|		buf->ib_ptr, buf->nelems);
		|	return nelems;
		EOF
	end

	def print_decoder io
		io.print (<<-EOC).gsub(/^\s*\|/, "")
		|static long
		|#{dfunc}(struct ibuffer_t *buf,
		|	const unsigned char *src, N_UI4 nxd, N_UI4 nyd)
		|{
		EOC
		text = case @pack.algorithm
		when :PACK then dcd_pack
		when :RLEN then dcd_rlen
		when :PLAIN then dcd_plain
		else raise
		end
		text = text.split(/\n/).grep(/^./).join("\n") + "\n"
		text.gsub!(/\(N_SI4\)\(N_SI4\)/, '(N_SI4)')
		text.gsub!(/\(N_SI2\)\(N_SI2\)/, '(N_SI2)')
		text.gsub!(/( \+ 0)+([^.])/, "\\2")
		io.print text
		io.print "}\n\n"
	end

	def maxmin_name
		"maxmin_#{@user.lcname}_#{@miss.lcname}"
	end

	def maxminfunc
		text = @user.maxminfunc(maxmin_name)
		if @miss.none?
			text.gsub(/@A@/, '').gsub(/V@(\w+)@/, "\\1")
		elsif @miss.udfv?
			text.gsub!(/@A@/, '')
			text.gsub!(/^#ifndef AVOID.*#else\n/m, '')
			text.gsub!(/^#endif\n/, '')
			text.gsub!(/V@(\w+)@/, "\\1\n\t\t\t&& source[i] != #{undef_u}\n\t\t")
			text
		else
			@user.maxminfunc_mask(maxmin_name)
		end
	end

end

class CodecAlias

	def initialize codec, pack, miss, user
		@codec, @pack, @miss, @user = codec, pack.dup, miss, user
	end

	attr_reader :user, :pack

	def ident
		"#{@pack.lcname}_#{@miss.lcname}_#{@user.lcname}"
	end


	def <=> other
		self.ident <=> other.ident
	end

	def efunc() @codec.efunc end
	def dfunc() @codec.dfunc end
	def maxmin_name() @codec.maxmin_name end
	def maxminfunc() @codec.maxminfunc end
	def fixed() @codec.fixed end
	def factor() @codec.factor end

	def print_encoder(io) end
	def print_decoder(io) end

	def p4
		$syms.push('-' + @pack.ucname)
		"SYM4_#{@pack.ucname}"
	end

	def m4
		$syms.push('-' + @miss.ucname)
		$syms.push @miss.ucname
		"SYM4_#{@miss.ucname}"
	end

	def u4
		$syms.push('-' + @user.ucname)
		"SYM4_#{@user.ucname}"
	end

end

class CodecTable

	def initialize
		@codec = []
		@index = {}
		@maxmin = {}
	end

	def add pack, miss, user
		c = Codec.new(pack, miss, user)
		return @index[c.ident] if @index[c.ident]
		@codec.push c
		@index[c.ident] = c
		c
	end

	def link codec, pack, miss, user
		c = CodecAlias.new(codec, pack, miss, user)
		return @index[c.ident] if @index[c.ident]
		@codec.push c
		@index[c.ident] = c
		c
	end

	def buildtable
		@codec.sort!
		for codec in @codec
			key = codec.maxmin_name
			next unless codec.pack.maxmin?
			next if codec.user.rawdata?
			next if codec.user.binary?
			next if @maxmin[key]
			@maxmin[key] = codec
		end
	end

	def printto io
		buildtable
		io.print (<<-EOC).gsub(/^\s*\|/, "")
		|/** @file
		| * @brief implementation of NDF binary encoder/decoder (codec).
		| *  [generated from #{__FILE__}]
		| */
		|#include "config.h"
		|#include "nusdas.h"
		|#include "internal_types.h"
		|#include "sys_kwd.h"
		|# define NEED_PEEK_N_SI2
		|# define NEED_PEEK_N_UI2
		|# define NEED_PEEK_N_SI4
		|# define NEED_PEEK_N_UI4
		|# define NEED_PEEK_FLOAT
		|# define NEED_PEEK_DOUBLE
		|# define NEED_POKE_FLOAT
		|# define NEED_POKE_DOUBLE
		|# define NEED_MAKE_UI8
		|#include "sys_endian.h"
		|#include "sys_int.h"
		|#include "glb.h"
		|
		|#include <string.h>
		|#include <math.h>
		|#include <stddef.h>
		|#include <stdlib.h>
		|#include "dset.h"
		|#include "dfile.h"
		|#include "sys_file.h"
		|#include "ndf_codec.h"
		|#include "sys_err.h"
		|# define NEED_MEMCPY4
		|# define NEED_MEMCPY8
		|#include "sys_string.h"
		|#include "sys_mem.h"
		|
		|#ifdef HAVE_LRINT
		|# define ROUND(x)	lrint(x)
		|#else
		|# define ROUND(x)	floor((x) + 0.5)
		|#endif
		|
		EOC
		for key in @maxmin.keys.sort
			io.print @maxmin[key].maxminfunc
		end
		for codec in @codec
			codec.print_encoder(io)
			codec.print_decoder(io) unless codec.user.binary?
		end

		io.print (<<-EOC).gsub(/^\s*\|/, "")
		|static struct ndf_codec_t codec_table[] = {
		EOC
		for c in @codec
			io.print (<<-EOC).gsub(/^\s*\|/, "")
			| { #{c.p4}, #{c.m4}, #{c.u4}, #{c.fixed}, #{c.factor}, #{c.efunc}, #{c.dfunc} },
			EOC
		end
		io.print (<<-EOC).gsub(/^\s*\|/, "")
		| { 0, 0, 0, -1, -1, NULL, NULL }
		|};
		|
		|struct ndf_codec_t *ndf_get_codec(sym4_t packing, sym4_t missing, sym4_t bffm)
		|{
		|	struct ndf_codec_t *codec;
		|	for (codec = codec_table; codec->packing; codec++) {
		|		if (packing == codec->packing
		|			&& missing == codec->missing
		|			&& bffm == codec->bffm)
		|			return codec;
		|	}
		|	return NULL;
		|}
		EOC
	end

end

$syms = []

pki1 = PackType.new("I1  ", "N_UI1")
rlen = PackType.new("RLEN", "N_UI1")
pki2 = PackType.new("I2  ", "N_SI2")
n1i2 = PackType.new("N1I2", "N_SI2").setscale(10)
pki4 = PackType.new("I4  ", "N_SI4")
pac1 = PackType.new("1PAC", "N_UI1")
upc2 = PackType.new("2UPC", "N_UI2")
upj2 = PackType.new("2UPJ", "N_UI2")
upp2 = PackType.new("2UPP", "N_UI2")
pac2 = PackType.new("2PAC", "N_SI2")
pkr4 = PackType.new("R4  ", "float")
pac4 = PackType.new("4PAC", "N_SI4")
pkr8 = PackType.new("R8  ", "double")
none = MissType.new("NONE")
udfv = MissType.new("UDFV")
mask = MissType.new("MASK")
r8 = UserType.new("R8", "double")
r4 = UserType.new("R4", "float")
i4 = UserType.new("I4", "N_SI4")
i2 = UserType.new("I2", "N_SI2")
i1 = UserType.new("I1", "N_UI1")
nc = UserType.new("NC", "N_UI1")
nd = UserType.new("ND", "N_UI1")

ct = CodecTable.new

ct.add rlen, none, i1
ct.add rlen, none, i2
ct.add rlen, none, i4
ct.add rlen, none, r4
ct.add rlen, none, r8
codec = ct.add(upc2, none, nc)
ct.link codec, pac2, none, nc
ct.add upj2, none, nc
ct.add upp2, none, nc
codec = ct.add(upc2, udfv, nc)
ct.link codec, pac2, udfv, nc
ct.add upj2, udfv, nc
ct.add upp2, udfv, nc
for missing in [none, udfv, mask]
	ct.add pki1, missing, i1
	ct.add pki1, missing, i2
	ct.add pki1, missing, i4
	ct.add pki1, missing, r4
	ct.add pki1, missing, r8
	ct.add pac1, missing, i2
	ct.add pac1, missing, i4
	ct.add pac1, missing, r4
	ct.add pac1, missing, r8
	ct.add n1i2, missing, i2
	ct.add n1i2, missing, i4
	ct.add n1i2, missing, r4
	ct.add n1i2, missing, r8
	ct.add pki2, missing, i2
	ct.add pki2, missing, i4
	ct.add pki2, missing, r4
	ct.add pki2, missing, r8
	ct.add upc2, missing, i4
	ct.add upc2, missing, r4
	ct.add upc2, missing, r8
	ct.add pac2, missing, i4
	ct.add pac2, missing, r4
	ct.add pac2, missing, r8
	ct.add pki4, missing, i4
	ct.add pki4, missing, r4
	ct.add pki4, missing, r8
	ct.add pkr4, missing, r4
	ct.add pkr4, missing, r8
	ct.add pac4, missing, r4
	ct.add pac4, missing, r8
	ct.add pkr8, missing, r4
	ct.add pkr8, missing, r8
	ct.add upj2, missing, i4
	ct.add upj2, missing, r4
	ct.add upj2, missing, r8
	ct.add upp2, missing, i4
	ct.add upp2, missing, r4
	ct.add upp2, missing, r8
end

[pki1, rlen, pki2, n1i2, pki4, pac1, upc2, upj2, upp2, pac2, pkr4, pac4, pkr8].each{|packing|
  [none, udfv, mask].each{|missing|
    ct.add packing, missing, nd
  }
}

ccode = File.open("ndf_codec.c", "w")
ct.printto(ccode)
ccode.close

symfile = File.open("ndf_codec.smb", "w")
for sym in $syms.sort.uniq
	symfile.puts sym
end
symfile.close
