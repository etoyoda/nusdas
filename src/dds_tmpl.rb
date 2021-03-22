#!/usr/bin/ruby

class Lex

	def initialize
		@fp = nil
		@syms = nil
		@chr2sym = Array.new(256)
		@sym2code = {}
		@code2sym = {}
		@state2code = { :End => -1, :Initial => 0 }
		@code2state = {}
		@acttab = {}
		@act2prog = {}
		@act2code = {}
		@code2act = {}
		@errors = []
		@default_act = nil
		setup_strcpy
		setup_plus
	end

	def rule state1, sym, state2, act, prog = nil
		if (state2 and prog)
			st2 = @state2code[state2]
			st2 = "@<#{state2}>" unless Integer === st2
			prog += "\t\tstate = #{st2}; /* #{state2.inspect} */\n"
		elsif (prog)
			prog += "\t\t/* state unchanged */\n"
		end
		if (state2 and not @state2code.include?(state2))
			@state2code[state2] = :dummy
		end
		@act2prog[act] = prog if act and prog
		raise "undefined action #{act}" if act and @act2prog[act].nil?
		@acttab[[state1, sym]] = [state2, act]
	end

	def define_symbol ichar, sym
		raise unless Integer === ichar
		raise unless String === sym
		@chr2sym[ichar] = sym
	end

	COPY = <<EOF
		dst[idst] = *src;
		idst++;
EOF

	MARK = <<EOC
		mark = src;
EOC

	REVERT = <<EOF
		memcpy(dst + idst, mark, src - mark + 1);
		idst += (src - mark + 1);
EOF

	ADV = <<EOC
EOC

	SLASH = <<EOC
		dst[idst++] = '/';
EOC

	def setup_strcpy
		define_symbol 0, 'NUL'
		for i in 1..255
			define_symbol i, 'ANY'
		end
		rule :Initial, 'NUL', :End, nil
		rule :Initial, 'ANY', nil, 'Copy', COPY
		@default_act = 'Copy'
	end

	def setup_plus
		#
		# 第一弾: スラッシュ以外の文字を読むと非初期状態になるように
		#
		define_symbol '/'[0], '///'
		rule :Initial,  '///', nil,       'Copy',   COPY
		rule :Initial,  'ANY', 'Default', 'Any',    COPY
		rule 'Default', 'NUL', :End,      nil
		rule 'Default', 'ANY', nil,       'Copy'
		rule 'Default', '///', :Initial,  'Slash',  COPY
		#
		# 第二弾: 下線シンボルを作る
		#
		define_symbol '_'[0], '___'
		rule :Initial,  '___', '_',      '_', MARK
		rule 'Default', '___', nil,	'Copy'
		rule '_',       '///', :Initial, 'Revert:SL', REVERT
		rule '_',       'NUL', :End,     'Revert:NUL', REVERT
		rule '_',       'ANY', 'Default', 'Revert', REVERT
		rule '_',       '___', 'Default', 'Revert'
		@default_act = 'Revert'
	end

	def register1 word, code
		for i in 1..(word.length - 2)
			sym = word[i, 1] * 3
			define_symbol word[i], sym
			head = word[0, i + 1]
			rule word[0, i], sym, head, head, ADV
			rule :Initial, sym, 'Default', 'Any'
			rule 'Default', sym, nil, 'Copy'
			rule head, '///', :Initial, 'Revert:SL'
			rule head, 'NUL', :End, 'Revert:NUL'
			rule head, 'ANY', 'Default', 'Revert'
			rule head, '___', 'Default', 'Revert'
		end
		sym = word[-1, 1] * 3
		define_symbol sym[0], sym
		rule :Initial, sym, 'Default', 'Any'
		rule 'Default', sym, nil, 'Copy'
		rule word[0..-2], sym, word, word, ADV
		rule word, '///', :Initial, word + ':SL', code + SLASH
		rule word, 'NUL', :End, word + ':NUL', code
		rule word, 'ANY', 'Default', 'Revert'
		rule word, '___', 'Default', 'Revert'
	end

	def register word, code
		register1 word.upcase, code
		register1 word.downcase, code
	end

	def compile
		@syms = @chr2sym.uniq.sort
		code = 0
		for sym in @syms
			@sym2code[sym] = code
			@code2sym[code] = sym
			code = code.succ
		end
		new_states = []
		for state in @state2code.keys
			next if Integer === @state2code[state]
			new_states.push state
		end
		new_states.sort!
		code = 1
		for state in new_states
			@state2code[state] = code
			code = code.succ
		end
		for state, istate in @state2code
			@code2state[istate] = state
		end
		code = 0
		for act in @act2prog.keys.sort
			@act2code[act] = code
			@code2act[code] = act
			code = code.succ
		end
		@actcode_eol = code
	end

	def actcode(state, isym)
		sym = @code2sym[isym]
		actrow = @acttab[[state, sym]]
		if actrow.nil? then
			act = @default_act
@errors.push "missing action [#{state.inspect}, #{sym}] defaults to #{act}\n"
		else
			act = actrow[1]
		end
		if act.nil? then
			@actcode_eol
		else
			@act2code[act]
		end
	end

	def print_tables
		puts "\tint symbol_table[256] = {"
		for i in 0..255
			symcode = @sym2code[@chr2sym[i]]
			cname = i.chr
			cname = format('0x%02x', i) unless /^\w/ =~ cname
			comma = (i == 255 ? ' ' : ',')
			puts "\t\t#{symcode}#{comma} /* #{cname} */"
		end
		puts "\t};"
		nstate = @state2code.keys.size - 1
		nsym = @syms.size
		puts "\tint action_table[#{nstate}][#{nsym}] = {"
		for istate in 0...nstate
			state = @code2state[istate]
			print "/* #{state.inspect} */\n{"
			a = (0...nsym).collect { |isym| actcode(state, isym) }
			print a.join(', ')
			comma = (istate == nstate - 1) ? '' : ','
			puts "}#{comma}"
		end
		puts "\t};"
	end

	def print_actcase
		for iact in 0...@actcode_eol
			act = @code2act[iact]
			puts "case #{iact}: /* #{act} */"
			print @act2prog[act].gsub(/@<(\w+)>/) {
				@state2code[$1]
			}
			puts "\t\tbreak;"
		end
		puts "default:"
		puts "\t\tstate = -1; /* :End */"
	end

	def dump filename = nil
		#$stdout = filename.nil? ? $stdout : File.open(filename, 'w')
		$stdout.reopen(filename, 'w') if filename
		for line in DATA
			case line
			when /\$TABLES\$/ then
				print_tables
				print "#line #{DATA.lineno} \"#{__FILE__}\"\n"
			when /\$ACTION_TABLE\$/ then
				print_acttab
				print "#line #{DATA.lineno} \"#{__FILE__}\"\n"
			when /\$ACTION_CASES\$/ then
				print_actcase
				print "#line #{DATA.lineno} \"#{__FILE__}\"\n"
			else
				print line
			end
		end
		$stdout.close if filename
		if @errors
			$stderr.print @errors if $VERBOSE
		end
	end

end

lex = Lex.new

lex.register '_model', <<EOT
		memcpy(dst + idst, (char *)&def->nustype.type1, 4);
		idst += 4;
EOT

lex.register '_space', <<EOT
		memcpy(dst + idst, (char *)&def->nustype.type1 + 4, 4);
		idst += 4;
EOT

lex.register '_2d', <<EOT
		memcpy(dst + idst, (char *)&def->nustype.type1 + 4, 2);
		idst += 2;
EOT

lex.register '_3d', <<EOT
		memcpy(dst + idst, (char *)&def->nustype.type1 + 6, 2);
		idst += 2;
EOT

lex.register '_attribute', <<EOT
		memcpy(dst + idst, (char *)&def->nustype.type2, 2);
		idst += 2;
EOT

lex.register '_time', <<EOT
		memcpy(dst + idst, (char *)&def->nustype.type2 + 2, 2);
		idst += 2;
EOT

lex.register '_name', <<EOT
		/* 4字に満たない場合詰めるため memcpy にはしない */
		idst += nusdas_snprintf(dst + idst, 5, "%Ps", def->nustype.type3);
EOT

lex.register '_basetime', <<EOT
		if (fill_basetime == -1) {
		    memcpy(dst + idst, "\\xFF.23456789ab", 12);
		    save = tmpl->b;
		    tmpl->b = idst + tmpl->rel;
		    if (save >= 0 && tmpl->b - save < 0xFF) {
			    dst[idst] = tmpl->b - save;
		    }
		} else {
		    time_to_chars(dst + idst, fill_basetime);
		}
		idst += 12;
EOT

lex.register '_member', <<EOT
		memcpy(dst + idst, "\\xFF.23", 4);
		save = tmpl->m;
		tmpl->m = idst + tmpl->rel;
		if (save >= 0 && tmpl->m - save < 0xFF) {
			dst[idst] = tmpl->m - save;
		}
		idst += 4;
EOT

lex.register '_validtime', <<EOT
		memcpy(dst + idst, "\\xFF.23456789ab", 12);
		save = tmpl->v;
		tmpl->v = idst + tmpl->rel;
		if (save >= 0 && tmpl->v - save < 0xFF) {
			dst[idst] = tmpl->v - save;
		}
		idst += 12;
EOT

lex.compile
lex.dump('dds_tmpl.c')

__END__
/** @file
 * @brief ファイル名テンプレート dds_template の構築
 */

#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include <stdlib.h>
#include <string.h>
#include "sys_mem.h"
# define NEED_CHARS_DUP
# define NEED_MEMCPY4
#include "sys_string.h"
#include "sys_time.h"
#include "sys_err.h"
#include "sys_kwd.h"
#include "dset.h"

	static int
expand_keywords(char * const dst, const char *src, nusdef_t *def,
	struct dds_template *tmpl, N_SI4 fill_basetime)
{
	$TABLES$
	int state = 0;
	int idst = 0;
	const char *mark = NULL;
	while (state >= 0) {
		int symbol, action;
		int save;
		symbol = symbol_table[*(unsigned char *)src];
		action = action_table[state][symbol];
		switch (action) {
			$ACTION_CASES$;
		}
		/* switch (action) の終わり */
		src++;
	}
	return idst + tmpl->rel;
}

	static int
expand_size_estimate(const char *str)
{
	int	n = 0;
	while (*str) {
		if (*str == '_') {
			switch (str[1]) {
				case 'b':
				case 'B':
					n += 4;
					break;
				case 'v':
				case 'V':
					n += 3;
					break;
				default:
					n++;
			}
		} else {
			n++;
		}
		str++;
	}
	return n;
}

/** @brief ファイル名テンプレート dds_template の構築
 *
 * 情報源は定義ファイル dds->def (うち path, filename) と dds->dirname
 * である。
 * @retval 0 正常終了
 * @retval 負 エラー (定義ファイルの解読エラーおよび nus_malloc)
 * @todo エラー通知法の確定
 */
	int
nusdds_build_template(struct dds_t *dds, /**< データセット */
	N_SI4 fill_basetime) /**< -1 でなければ基準時刻に置換 */
{
	nusdef_t *def = &dds->def;
	struct dds_template *tmpl = &dds->tmpl;
	char strbuf[sizeof(def->path) + sizeof(def->filename)];
	int status;

	/** @note 定義ファイルの解読が未完了ならば完成させる. */
	if ((status = dds_endparse(dds)) != 0)
		return status;

	/** @note 既に構築が完了していれば何もしないで正常終了. */
	if (tmpl->fullpath) {
		return 0;
	}
	tmpl->b = tmpl->m = tmpl->v = -1;

	/* --- strbuf に展開前のパスを構築する. --- */
	{
		char *p;
		size_t plen;
		strcpy(strbuf, def->path);
		p = strbuf;
		p += (plen = strlen(p));
		if ((plen > 0) && (p[-1] != '/') && def->filename[0]) {
			*p++ = '/';
			*p = '\0';
		}
		if ((def->filename[0] == '\0') && (p > strbuf)
				&& (p[-1] == '/')) {
			p[-1] = '\0';
		}
		strncat(strbuf, def->filename, sizeof(strbuf));
		if ((strbuf[0] == '\007') && (strbuf[1] == '4')) {
			nusdas_snprintf(strbuf, 32, "NUSD%ys", &(def->nustype));
		} else if (strbuf[0] == '\007') {
			int backup6;
			memcpy(strbuf, (char *)&def->nustype.type1 + 6, 2);
			/* 4 字に満たない場合右寄せするため
			   memcpy を用いない */
			backup6 = strbuf[6];
			nusdas_snprintf(strbuf + 2, 5, "%4Ps",
				def->nustype.type3);
			strbuf[6] = backup6;
		}
	}
	/* --- strbuf の文字列を tmpl->fullpath に展開する --- */
	{
		const char *dirname;
		size_t r;
		size_t allocsize;
		dirname = dds->dirname ? dds->dirname : "";
		if (*dirname == '\0') {
			dirname = ".";
		}
		r = strlen(dirname);
		allocsize = r + expand_size_estimate(strbuf) + 2;
		tmpl->fullpath = nus_malloc(allocsize);
		memcpy(tmpl->fullpath, dirname, r);
		if (tmpl->fullpath[r - 1] != '/') {
			tmpl->fullpath[r] = '/';
			r++;
		}
		tmpl->rel = r;
		tmpl->pathlen = expand_keywords(tmpl->fullpath + r,
			strbuf, def, tmpl, fill_basetime);
		tmpl->fullpath[tmpl->pathlen] = '\0';
	}
	
	return 0;
}

/* @brief パス名テンプレートに埋め込まれた置換の連鎖を数える/それぞれに
 *        ついてコールバック関数を呼ぶ
 * @param fullpath パス名テンプレート文字列
 * @param start_ofs 最初の置換対象のオフセット
 * @param callback コールバック関数
 * @param substr 置換対象の部分文字列を指すポインタ
 * @param arg コールバック関数に渡す引数
 * @return 置換の個数
 */
	unsigned
dds_tmpl_backscan(char *fullpath, int start_ofs,
		void (*callback)(char *substr, void *arg),
		void *arg)
{
	unsigned ofs, back, count;
	if (start_ofs < 0)
		return 0;
	ofs = start_ofs;
	count = 0;
	while (1) {
		count++;
		back = ((unsigned char *)fullpath)[ofs];
		if (callback) callback(fullpath + ofs, arg);
		if (back == 0xFF)
			break;
		ofs -= back;
	}
	return count;
}

	void
dds_tmpl_fill_time(char *substr, void *arg)
{
	time_to_chars(substr, *(N_SI4 *)arg);
}

	void
dds_tmpl_fill_member(char *substr, void *arg)
{
	memcpy4(substr, (char *)arg);
}

	char *
nusdds_tmpl_expand(struct dds_template *tmpl, const nusdims_t *dim)
{
	char *fullpath;
	unsigned i;
	/* 長さがわかっているので chars_dup */
	fullpath = chars_dup(tmpl->fullpath, tmpl->pathlen + 1);
	if (fullpath == NULL) {
		SETERR(NUSERR_MemShort);
		return NULL;
	}
	dds_tmpl_backscan(fullpath, tmpl->b, dds_tmpl_fill_time,
			(void *)&dim->basetime);
	dds_tmpl_backscan(fullpath, tmpl->v, dds_tmpl_fill_time,
			(void *)&dim->validtime1);
	dds_tmpl_backscan(fullpath, tmpl->m, dds_tmpl_fill_member,
			(void *)&dim->member);
	/* ファイル名に空白は許さない */
	for (i = 0; i < tmpl->pathlen; i++) {
		if (fullpath[i] == ' ') {
			fullpath[i] = '_';
		}
	}
	return fullpath;
}

struct ddstmpl_scanglob_info {
	char *fullpath;
	sym4_t dimchar;
	N_UI4 *entry;
};

	void
remember(char *substr, void *arg)
{
	struct ddstmpl_scanglob_info *info = arg;
	info->entry[0] = substr - info->fullpath;
	info->entry[1] = info->dimchar;
	info->entry += 2;
}

	static int
Ui4Compare(const void *va, const void *vb)
{
	const N_UI4 *a = va;
	const N_UI4 *b = vb;
	return (*a > *b) ? 1 : (*a < *b) ? -1 : 0;
}

/** @brief テンプレートから置換指定を抜き出す
 *
 * テンプレートが含む basetime, member, validtime の置換を抜き出し、
 * オフセット順にソートして返す。
 * @return 置換数の2倍 +1 の要素を含む int の配列で,
 * retval[0] = 最初の置換位置,
 * retval[1] = 最初の置換種別 ('B', 'M', 'V' のいずれか),
 * retval[2] = 次の置換位置,
 * retval[3] = 次の置換種別, ...,
 * retval[2n] = -1 (ターミネータ)
 * のごとくである。
 * @note 返却値は nus_malloc されたポインタであるので nus_free(3) すべきである。
 */
	N_UI4 *
nusdds_tmpl_scanglob(struct dds_template *tmpl)
{
	struct ddstmpl_scanglob_info info;
	unsigned count = 0;
	unsigned i;
	N_UI4 *globtable;
	count += dds_tmpl_backscan(tmpl->fullpath, tmpl->b, NULL, NULL);
	count += dds_tmpl_backscan(tmpl->fullpath, tmpl->m, NULL, NULL);
	count += dds_tmpl_backscan(tmpl->fullpath, tmpl->v, NULL, NULL);
	/* ファイル名に空白は許さない */
	for (i = 0; i < tmpl->pathlen; i++) {
		if (tmpl->fullpath[i] == ' ') {
			tmpl->fullpath[i] = '_';
		}
	}
	info.fullpath = tmpl->fullpath;
	info.entry = globtable = nus_malloc((count * 2 + 1) * sizeof(N_UI4));
	info.dimchar = SYM4_BTLS;
	dds_tmpl_backscan(tmpl->fullpath, tmpl->b, remember, &info);
	info.dimchar = SYM4_VTLS;
	dds_tmpl_backscan(tmpl->fullpath, tmpl->v, remember, &info);
	info.dimchar = SYM4_MBLS;
	dds_tmpl_backscan(tmpl->fullpath, tmpl->m, remember, &info);
	qsort(globtable, count, 2 * sizeof(int), Ui4Compare);
	globtable[count * 2] = ~(N_UI4)0;
	return globtable;
}
