class CParser

prechigh
  nonassoc UNARY
  left '*' '/' '%'
  left '+' '-'
  left '<<' '>>'
  left '<'  '>'  '>='  '<='
  left '==' '!='
  left '&'
  left '^'
  left '|'
  left '&&'
  left '||'
  nonassoc 'else'
  nonassoc WITHOUT_ELSE
preclow

token IDENTIFIER TYPEDEF_NAME STRING INTEGER FLOAT CHARACTER
  ENUM_CONSTANT BadToken PreMarkupLeader PostMarkupLeader COMMENT
  UNARY WITHOUT_ELSE LexBug

expect 1

rule

  real_program: /* empty program */
  	| translation_unit
	;

  translation_unit: external_declaration
	{ extdecl(val[0]) }
  	| translation_unit external_declaration
	{ extdecl(val[1]) }
	;

  external_declaration: core_external_declaration
  	| pre_comment_list core_external_declaration
  	{
		result = val[1]
		result.first.comment = val[0]
	}
  	| core_external_declaration post_comment
  	{
		result = val[0]
		result.last.comment = val[1]
	}
	;

  core_external_declaration: function_definition
  	| declaration
	;

  function_definition: declarator compound_statement
  	{ raise "function should be declared as int" }
  	| declaration_specifiers declarator compound_statement
  	{ result = List.new.push(SymDecl.new(val[0], val[1])) }
  	| declaration_specifiers declarator post_comment compound_statement
  	{
	  result = List.new.push(SymDecl.new(val[0], val[1]))
	  val[1].object.last.comment = val[2]
	}
	| declarator declaration_list compound_statement
  	{ raise "pre-ANSI function" }
  	| declaration_specifiers declarator declaration_list compound_statement
  	{ raise "pre-ANSI function" }
  	; 

  declaration:
  	pure_declaration
	| '__extension__' pure_declaration
	{ result = val[1] }
	;

  pure_declaration:
	declaration_specifiers ';'
        {
	  # 構造体・共用体・列挙型の宣言
	  result = List.new.push(val[0])
	}
  	| declaration_specifiers attributegroups ';'
	{
	  result = List.new.push(val[0])
	}
  	| declaration_specifiers init_declarator_list ';'
	{
	  result = List.new
	  for variable in val[1]
	    # バグ: スコープを考えていない
	    @typedeftab[variable.name] = val[0] if val[0].typedef?
	    result.push SymDecl.new(val[0], variable)
	  end
	}
  	| declaration_specifiers init_declarator_list attributegroups ';'
	{
	  result = List.new
	  for variable in val[1]
	    # バグ: スコープを考えていない
	    @typedeftab[variable.name] = val[0] if val[0].typedef?
	    result.push SymDecl.new(val[0], variable)
	  end
	}
	;

  attributegroups: attributegroup
  	| attributegroups attributegroup
	;

  attributegroup: '__attribute__' '(' '(' attributes ')' ')'
  	| '__asm__' '(' STRING STRING ')'
  	;

  attributes: attribute
  	| attributes ',' attribute
	;

  attribute: IDENTIFIER
  	| '__mode__' '(' IDENTIFIER ')'
  	| '__nonnull__' '(' integer_list ')'
	| '__const'
  	| IDENTIFIER '(' IDENTIFIER ','  integer_list ')'
	;

  integer_list: INTEGER
  	| integer_list ',' INTEGER
	;

  declaration_list: declaration
  	| declaration_list declaration
	;
  
  declaration_specifiers: storage_class_specifier
  	{ result = DeclSpec.new.add_storage_class(val[0]) }
  	| storage_class_specifier declaration_specifiers
  	{ result = val[1].add_storage_class(val[0]) }
  	| type_specifier
  	{ result = DeclSpec.new.add_type(val[0]) }
  	| type_specifier declaration_specifiers
  	{ result = val[1].add_type(val[0]) }
  	| type_qualifier
  	{ result = DeclSpec.new.add_type_qual(val[0]) }
  	| type_qualifier declaration_specifiers
  	{ result = val[1].add_type_qual(val[0]) }
  	;

  storage_class_specifier: 'auto'
  	| 'register'
	| 'static'
	| 'extern'
	| 'typedef'
	| 'inline'
	;

  type_specifier: 'void'
  	| 'char'
  	| 'short'
  	| 'int'
  	| 'long'
  	| 'float'
  	| 'double'
  	| 'signed'
  	| 'unsigned'
	| struct_or_union_specifier
	| enum_specifier
	| TYPEDEF_NAME
	| '__builtin_va_list'
	;

  type_qualifier: 'const'
  	| 'volatile'
	| '__restrict'
	| '__const'
	;

  struct_or_union_specifier:
          struct_or_union identifier
  	| struct_or_union '{' struct_declaration_list '}'
  	| struct_or_union identifier '{' struct_declaration_list '}'
	;

  struct_or_union: 'struct'
  	| 'union'
	;

  struct_declaration_list: struct_declaration
	| struct_declaration_list struct_declaration
  	;

  init_declarator_list: init_declarator
  	{ result = List.new; result.push(val[0]) }
  	| init_declarator_list ',' init_declarator
	{ result = val[0]; result.push(val[2]) }
	;

  init_declarator: declarator
  	{ result = val[0] }
  	| declarator '=' initializer
  	{ result = Initialize.new(val[0], val[2]) }
	;

  struct_declaration:
  	  specifier_qualifier_list struct_declarator_list ';'
  	| pre_comment specifier_qualifier_list struct_declarator_list ';'
  	| specifier_qualifier_list struct_declarator_list ';' post_comment
  	| '__extension__' specifier_qualifier_list struct_declarator_list ';'
	# causes one shift/reduce conflict
	| '__extension__' 'union' '{' struct_declaration_list '}' ';'
	;

  specifier_qualifier_list:
          type_specifier
  	| type_specifier specifier_qualifier_list
  	| type_qualifier
  	| type_qualifier specifier_qualifier_list
  	;

  struct_declarator_list: struct_declarator
  	| struct_declarator_list ',' struct_declarator
	;

  struct_declarator: declarator
  	| declarator ':' constant_expression
	# C99 extension
  	| ':' constant_expression
	;

  enum_specifier: 'enum' identifier
  	| 'enum'  '{' enumerator_list '}'
  	| 'enum' identifier '{' enumerator_list '}'
	;

  enumerator_list: enumerator_commented
  	| enumerator_list ',' enumerator_commented
	;

  enumerator_commented: enumerator
  	| pre_comment enumerator
	| enumerator post_comment
	;

  enumerator: IDENTIFIER
  	| IDENTIFIER '=' constant_expression
	;

  declarator: direct_declarator
  	| pointer direct_declarator
	{ result = Pointer.new(val[1], val[0]) }
	;

  direct_declarator: IDENTIFIER
  	{ result = Identifier.new(val[0]) }
  	| '(' declarator ')'
	{ result = val[1] }
	| direct_declarator '[' ']'
	{ result = CArray.new(val[0], nil) }
	| direct_declarator '[' constant_expression ']'
	{ result = CArray.new(val[0], val[2]) }
	| direct_declarator '('  ')'
	{ result = Func.new(val[0], nil) }
	| direct_declarator '(' parameter_type_list ')'
	{ result = Func.new(val[0], val[2]) }
	;

  pointer: '*'
  	{ result = DeclSpec.new }
  	| '*' type_qualifier_list
	{ result = val[1] }
  	| '*' pointer
  	{ result = Pointer.new(DeclSpec.new, val[1]) }
  	| '*' type_qualifier_list pointer
  	{ result = Pointer.new(val[1], val[2]) }
	;

  type_qualifier_list: type_qualifier
  	{ result = DeclSpec.new.add_type_qual(val[0]) }
  	| type_qualifier_list type_qualifier
  	{ result = val[0].add_type_qual(val[1]) }
	;

  parameter_type_list: parameter_list
  	| parameter_list ',' '...'
	{ result = val[0]; result.push(VarArgs.new) }
  	| parameter_list ',' '...' post_comment
	{
	  result = val[0]
	  result.push(VarArgs.new)
	  result.last.comment = val[3]
	}
  	| parameter_list ',' post_comment '...' post_comment
	{
	  result = val[0]
	  result.last.comment = val[2]
	  result.push(VarArgs.new)
	  result.last.comment = val[4]
	}
	;

  parameter_list: parameter_declaration
  	{ result = List.new.push(val) }
  	| parameter_list ',' parameter_declaration
  	{ result = val[0]; result.push val[2] }
  	| parameter_list ',' post_comment parameter_declaration
  	{
	  result = val[0]
	  result.last.comment = val[2]
	  result.push val[3]
	}
  	| parameter_list attributegroup ',' post_comment parameter_declaration
  	{
	  result = val[0]
	  result.last.comment = val[3]
	  result.push val[4]
	}
	;

  parameter_declaration: declaration_specifiers declarator
  	{ result = Param.new(val[0], val[1]) }
	| declaration_specifiers declarator attributegroup
	{ result = Param.new(val[0], val[1]) }
	| declaration_specifiers declarator post_comment
  	{
	  result = Param.new(val[0], val[1])
	  result.comment = val[2]
	}
	| declaration_specifiers declarator attributegroup post_comment
	{
	  result = Param.new(val[0], val[1])
	  result.comment = val[3]
	}
  	| declaration_specifiers
  	| declaration_specifiers abstract_declarator
  	{ result = Param.new(val[0], val[1]) }
	;

  initializer: assignment_expression
  	| '{' initializer_list '}'
  	| '{' initializer_list ','  '}'
	;

  initializer_list: initializer
  	| initializer_list ',' initializer
	;

  type_name: specifier_qualifier_list abstract_declarator
  	| specifier_qualifier_list
	;

  abstract_declarator: pointer
  	| direct_abstract_declarator
  	| pointer direct_abstract_declarator
	;

  direct_abstract_declarator: '(' abstract_declarator ')'
	| '[' ']'
	| '[' constant_expression ']'
	| direct_abstract_declarator '[' ']'
	| direct_abstract_declarator '[' constant_expression ']'
	| '(' ')'
	| '(' parameter_type_list ')'
	| direct_abstract_declarator '(' ')'
	| direct_abstract_declarator '(' parameter_type_list ')'
	;

  statement: c_statement
	| pre_comment c_statement
	;

  c_statement: labeled_statement
  	| expression_statement
	| compound_statement
	| selection_statement
	| iteration_statement
	| jump_statement
	;

  labeled_statement: IDENTIFIER ':' statement
  	| 'case' constant_expression ':' statement
	| 'default'  ':' statement
	;

  expression_statement: ';'
  	| expression ';'
	;

  compound_statement: '{' '}'
  	| '{' declaration_list '}'
  	| '{' statement_list '}'
  	| '{' declaration_list statement_list '}'
	;

  statement_list: statement
  	| statement_list statement
	;

  selection_statement: 'if'  '(' expression ')' statement  = WITHOUT_ELSE
  	| 'if'  '(' expression ')' statement 'else' statement
	| 'switch'  '(' expression ')' statement
	;

  iteration_statement: 'while'  '(' expression ')' statement
  	| 'do' statement 'while'  '(' expression ')' ';'
	| 'for'  '(' ';' ';' ')' statement
	| 'for'  '(' ';' ';' expression ')' statement
	| 'for'  '(' ';' expression ';' ')' statement
	| 'for'  '(' ';' expression ';' expression ')' statement
	| 'for'  '(' expression ';' ';' ')' statement
	| 'for'  '(' expression ';' ';' expression ')' statement
	| 'for'  '(' expression ';' expression ';' ')' statement
	| 'for'  '(' expression ';' expression ';' expression ')' statement
	;

  jump_statement: 'goto' IDENTIFIER ';'
  	| 'continue'  ';'
  	| 'break'  ';'
  	| 'return'  ';'
  	| 'return' expression ';'
	;

  expression: assignment_expression
  	| expression ',' assignment_expression
	;

  assignment_expression: conditional_expression
 	| unary_expression assignment_operator assignment_expression
	;

  assignment_operator: '='
  	| '*='
	| '/='
	| '%='
	| '+='
	| '-='
	| '<<='
	| '>>='
	| '&='
	| '^='
	| '|='
	;

  conditional_expression: logical_OR_expression
  	| logical_OR_expression '?' expression ':' conditional_expression
	;

  constant_expression: conditional_expression
  	;

  logical_OR_expression: exp
  	;

  exp: exp '||' exp
  	| exp '&&' exp
  	| exp '|' exp
  	| exp '^' exp
  	| exp '&' exp
  	| exp '==' exp | exp '!=' exp
  	| exp '<' exp | exp '>' exp | exp '<=' exp | exp '>=' exp
  	| exp '<<' exp | exp '>>' exp
  	| exp '+' exp | exp '-' exp
  	| exp '*' exp | exp '/' exp | exp '%' exp
	| cast_expression
	;

  cast_expression: unary_expression
	| '(' type_name ')' cast_expression
	;

  unary_expression: postfix_expression
  	| '++' unary_expression
  	| '--' unary_expression
  	| unary_operator cast_expression  = UNARY
	| 'sizeof' unary_expression
	| 'sizeof'  '(' type_name ')'
	;

  unary_operator: '&' | '*' | '+' | '-' | '~' | '!' ;


  postfix_expression: primary_expression
  	| postfix_expression '[' expression ']'
	{ result = CArray.new(val[0], val[2]) }
  	| postfix_expression '(' ')'
  	| postfix_expression '(' argument_expression_list ')'
  	| postfix_expression '.' identifier
  	| postfix_expression '->' identifier
  	| postfix_expression '++'
  	| postfix_expression '--'
	;

  primary_expression: IDENTIFIER
  	{ result = Identifier.new(val[0]) }
  	| constant
  	{ result = Constant.new(val[0]) }
	| string
	| '(' expression ')'
  	{ result = val[1] }
	;

  string: STRING
  	{ result = Constant.new(val[0]) }
	| string STRING
  	{ result = StrCat.new(val[0], Constant.new(val[1])) }
	;

  argument_expression_list: assignment_expression
  	| argument_expression_list ',' assignment_expression
	;

  constant: INTEGER
	| FLOAT
	| CHARACTER
	| ENUM_CONSTANT
	| BadToken { puts "broken token `#{val}`"; yyerror }
	;

  identifier: IDENTIFIER
  	| TYPEDEF_NAME
	;

  pre_comment_list: pre_comment
	| pre_comment_list pre_comment { result = val[1] } # discard former
  	;

  pre_comment: PreMarkupLeader { result = [] }
  	| pre_comment COMMENT { result = (val[0].is_a?(Array) ? val[0] : [val[0]]) + (val[1].is_a?(Array) ? val[1] : [val[1]]) }
	;

  post_comment: PostMarkupLeader { result = [] }
  	| post_comment COMMENT { result = (val[0].is_a?(Array) ? val[0] : [val[0]]) + (val[1].is_a?(Array) ? val[1] : [val[1]]) }
	;

end

---- header ----

require 'strscan'
require_relative './cptree.rb'

---- inner ----

  include CPTree

  C_KEYWORDS = /(auto|break|case|char|const|continue|default|do(uble)?\
|else|enum|extern|float|for|goto|if|int|long|register|return|short|signed\
|sizeof|static|struct|switch|typedef|union|unsigned|void|volatile|while\
|inline|__builtin_va_list|__attribute__|__mode__|__extension__\
|__restrict|__const|__asm__|__nonnull__)\b/

  PUNCTS = /(\.\.\.|->|[-+*%\/&^|=!<>]=|\
>>=|<<=|\+\+|--|&&|\|\||<<|>>|<:|>:|<%|%>|\
[-\[\](){}.&*%+~!\/<>^\|?:=,])/

  def parse(io, flags)
    @fp = io
    @line = nil
    @filename = @fp.respond_to?(:path) ? @fp.path : "(stdin)"
    @lineno = 0
    @doxygenp = false # whether in doxygen comment or not
    @commentp = false
    @yydebug = $DEBUG
    @typedeftab = {}
    @parseflags = flags
    @last_pos = [nil, nil]
    do_parse
  end

unless $DEBUG
  def show(tos)
    if Array === tos
      return '[' + tos.collect{ |elem| show(elem) }.join(', ') + ']'
    elsif tos.respond_to?(:cstr)
      return '<' + tos.cstr + '>'
    else
      return tos.inspect
    end
  end

  def on_error token, val, stack
    puts "syntax error on #{@filename}:#{@lineno} " +
      "near token #{val.inspect} (#{token_to_str token})"
    puts '<' + @line.string.chomp + '>' if @line
    puts show(stack)
    raise Racc::ParseError, "syntax error"
  end
end

  def getline
    cpp_cont = false
    while true
      line = @fp.gets
      throw(:EOF, nil) if line.nil?
      line.chomp!
      @lineno = @lineno.succ
      if cpp_cont
	unless /\\$/ =~ line
	  cpp_cont = false
	end
      else
	case line
	when /^\s*#.*\\$/
	  cpp_cont = true
	when /^# (\d+) "([^\x22]+)"/
	  @filename = $2
	  @lineno = $1.to_i - 1
	when /^\s*#/
	  # do_nothing
	else
	  return @line = StringScanner.new(line)
	end
      end
    end
  end

  def next_token
    t = next_token_core
    t
  end

  def next_token_core
    while true
      while @line.nil? || @line.eos?
        t = catch(:EOF) { getline }
        return [false, '$EOF'] if t.nil?
      end
      if @commentp
	if @line.skip(%r<([^*]|\*[^/])*\*\/\s*>)
	  @commentp = false
	else
	  @line = nil
	  next
	end
      end
      if @doxygenp
	if @line.scan(/(\s+)/)
	  return [:COMMENT, @line[0]]
	elsif @line.scan(%r<(.*)\*\/\s*>)
	  @doxygenp = false
	  return [:COMMENT, @line[1]]
	elsif @line.scan(/(.*)/)
	  return [:COMMENT, @line[0]]
	end
      else
        @line.skip(/\A\s+/)
	if @line.scan(/(\/\*\*<)/)
	  @doxygenp = true
	  return [:PostMarkupLeader, @line[0]]
	elsif @line.scan(/(\/\*\*)/)
	  @doxygenp = true
	  return [:PreMarkupLeader, @line[0]]
	end
        @line.skip(%r!\A\/\*([^\*]|\*[^\/])*\*\/\s*!)
	next if @line.eos?
	if @line.scan(%r<\/\*>)
	  @commentp = true
	  next
	elsif token = @line.scan(/;/)
	  @last_pos = [@filename, @lineno]
	  return [token, token]
        elsif token = @line.scan(PUNCTS)
	  return [token, token]
	elsif token = @line.scan(/[+-]?0x([0-9a-f]+\.[0-9a-f]*|\.[0-9a-f]+)p[+-]?\d+[FL]?\b/i)
	  return [:FLOAT, token]
	elsif token = @line.scan(/[+-]?(\d+\.\d*|\.\d+)(e[+-]?\d+)?[FL]?\b/i)
	  return [:FLOAT, token]
	elsif token = @line.scan(/0[0-7]*(u|ul|ull|l|ll|lu|llu)?\b/i)
	  return [:INTEGER, token]
	elsif token = @line.scan(/0x[0-9a-f]+(u|ul|ull|l|ll|lu|llu)?\b/i)
	  return [:INTEGER, token]
	elsif token = @line.scan(/[+-]?\d+(u|ul|ull|l|ll|lu|llu)?\b/i)
	  return [:INTEGER, token]
	elsif token = @line.scan(/'([^\\\']|\\([^0-7x]|[0-7]{1,3}|x[0-9A-Fa-f]+))'/)
	  return [:CHARACTER, token]
	elsif token = @line.scan(/"([^\\\x22]|\\.)*"/)
	  return [:STRING, token]
	elsif token = @line.scan(C_KEYWORDS)
	  return [token, token]
	elsif token = @line.scan(/([_a-zA-Z]\w*)\b/)
	  if @typedeftab[token] then
	    return [:TYPEDEF_NAME, token]
	  else
	    return [:IDENTIFIER, token]
	  end
	elsif token = @line.scan(/\S+/)
	  return [:BadToken, token]
	end
      end
    end
  end

  def extdecl(decl)
    if handler = @parseflags[:on_extdecl]
      for variable in decl
        next if variable.name.nil?
	variable.pos = @last_pos
        if handler.respond_to?(:call)
	  handler.call(variable)
	else
	  handler.push(variable)
	end
      end
    end
  end

---- footer ----

if $0 == __FILE__
  parser = CParser.new

  handler = proc { |extdecl|
    p extdecl.cstr
  }
  if ARGV.empty? then
    parser.parse(STDIN, :on_extdecl => handler)
  else
    for file in ARGV
      fp = File.open(file)
      parser.parse(fp, :on_extdecl => handler)
      fp.close
    end
  end
end
