#pragma once

#include "error/error.h"
#include "instruction/instruction.h"
#include "tokenizer/token.h"

#include <vector>
#include <optional>
#include <utility>
#include <map>
#include <cstdint>
#include <cstddef> // for std::size_t

namespace miniplc0 {
	enum SymbolKind {
		CONST_KIND,
		VAR_KIND,
		FUNCTION_KIND,
	};
	enum SymbolType {
		INT_TYPE,
		VOID_TYPE,
		DOUBLE_TYPE,

	};

	enum ConstType {
		STRING_Const_Type,
		INT_Const_Type,
		DOUBLE_Const_Type,
	};

	struct oneConst {
		ConstType type;
		std::string name;
		std::string value;
	};
	//符号表的一项
	struct _one_symbol {
		std::string name;
		int32_t index;
		SymbolKind kind;
		SymbolType type;
		int32_t level;
		bool isInitial;
		
	};
	//索引表一项的数据结构
	struct _index_function {
		int32_t outern;
		int32_t ecount;
		int32_t pointer;
	};

	struct OneFunction {
		int32_t index;
		std::string name;
		int32_t params_size;
		int32_t level;
		int32_t params_num;
		std::vector<SymbolType> type;
		SymbolType retype;
	};



	class Analyser final {
	private:
		using uint64_t = std::uint64_t;
		using int64_t = std::int64_t;
		using uint32_t = std::uint32_t;
		using int32_t = std::int32_t;
	public:
		Analyser(std::vector<Token> v)
			: _tokens(std::move(v)), _offset(0), _instructions({}), _current_pos(0, 0),
			_current_level(0),_nextFunction(0),_current_sp(0),_current_ss(0),_current_function_name("0_begin"),
			_symbol_table({}), _symbol_vector_map({}), _index_functions({}), _nextTokenIndex({ 0 }) {}
		Analyser(Analyser&&) = delete;
		Analyser(const Analyser&) = delete;
		Analyser& operator=(Analyser) = delete;

		// 唯一接口
		std::pair<std::vector<Instruction>, std::optional<CompilationError>> Analyse();
		//获取常量表
		std::vector<oneConst> getConsttable() { return _const_table; }
		//获取指令集
		std::map<std::string, std::vector<Instruction>> getInstructions() { return _instructions; }
		//获取函数表
		std::vector<OneFunction> getFunctionsTable() { return _functions_table; }
	private:
		// 所有的递归子程序

		// <程序>
		std::optional<CompilationError> analyseProgram();
		//函数声明
		std::optional<CompilationError> analyseFunctionDefinition();
		//<variable - declaration> :: = {<variable - declaration - statement>}
		//<variable - declaration - statement> ::=[<const-qualifier>]<type-specifier><init-declarator-list>';'
		//由于只写了基础c0，所以这里的<type-specifier>实际上应该是<simple-type-specifier>
		std::optional<CompilationError> analyseVariableDeclaration();

		//<init-declarator-list>
		std::optional<CompilationError> analyseInitList(SymbolKind kind, SymbolType type);
		//<init-declarator> ::= <identifier>[<initializer>]
		std::optional<CompilationError> analyseInitDeclarator(SymbolKind kind, SymbolType type);

		//<initializer> ::= '=' < expression >
		std::optional<CompilationError> analyseInitializer(SymbolKind kind, SymbolType type);

		//<parameter - clause> :: ='('[<parameter - declaration - list>] ')'
		std::optional<CompilationError> analyseParameterClause(int32_t& params_size, int32_t& ,std::vector<SymbolType>&);

		//<parameter-declaration-list> ::= <parameter - declaration>{',' < parameter - declaration > }
		std::optional<CompilationError> analyseParameterDeclarationList(int32_t& params_size, int32_t&, std::vector<SymbolType>&);

		//<parameter-declaration> ::= [<const - qualifier>]<type - specifier><identifier>
		std::optional<CompilationError> analyseParameterDeclaration(int32_t& params_size, int32_t&, std::vector<SymbolType>&);

		//<function-call> ::= <identifier> '('[<expression - list>] ')'
		std::optional<CompilationError> analyseFunctionCall();

		//<compound-statement> ::= '{' {<variable - declaration>} < statement - seq> '}'
		std::optional<CompilationError> analyseCompoundStatement();

		//<statement-seq> ::= {<statement>}
		//<statement> :: =
		//	'{' < statement - seq > '}'
		//	| <condition - statement>
		//	| <loop - statement>
		//	| <jump - statement>
		//	| <print - statement>
		//	| <scan - statement>
		//	| < assignment - expression>';'
		//	| < function - call>';'
		//	| ';'
		std::optional<CompilationError> analyseStatementSeq();
		std::optional<CompilationError> analyseStatement();
		//<jump-statement> ::= <return-statement>
		std::optional<CompilationError> analyseJumpStatement();
		//<return-statement> ::= 'return' [<expression>] ';'
		std::optional<CompilationError> analyseReturnStatement();
		// Token 缓冲区相关操作
			//<condition-statement> ::= 'if' '(' < condition > ')' < statement > ['else' < statement > ]
		std::optional<CompilationError> analyseConditionStatement();
		//<loop-statement> ::= 'while' '(' < condition > ')' < statement >
		std::optional<CompilationError> analyseLoopStatement();

		//<scan - statement>  :: = 'scan' '(' < identifier > ')' ';'
		std::optional<CompilationError> analyseScanStatement();

		//< print - statement > :: = 'print' '('[<printable - list>] ')' ';'
		std::optional<CompilationError> analysePrintStatement();

		//< printable - list > :: = <printable>{ ',' < printable > }
		std::optional<CompilationError> analysePrintableList(int32_t& printable_num);
		//<printable> :: = <expression>
		std::optional<CompilationError> analysePrintable(SymbolKind kind, SymbolType type);

		//<expression-list> ::= <expression>{',' < expression > }
		std::optional<CompilationError> analyseExpressionList(const std::string );
		//<expression> :: =<additive - expression>
		std::optional<CompilationError> analyseExpression(SymbolKind kind, SymbolType type);

		std::optional<CompilationError> analyseAdditiveExpression(SymbolKind kind, SymbolType type);

		//	<multiplicative - expression> :: =<cast - expression>{ <multiplicative - operator><cast - expression> }
		std::optional<CompilationError> analyseMultiplicativeExpression(SymbolKind kind, SymbolType type);
		//	<cast - expression> :: ={ '(' < type - specifier > ')' }<unary - expression>
		//std::optional<CompilationError> Analyser::analyseCastExpression();
		//	<unary - expression> :: =[<unary - operator>]<primary - expression>
		std::optional<CompilationError> analyseUnaryExpression(SymbolKind kind, SymbolType type);
		//	<primary - expression> :: =
		//	'(' < expression > ')'
		//	| <identifier>
		//	| <integer - literal>
		//	| <char - literal>
		//	| <floating - literal>
		//	| <function - call>
		//std::optional<CompilationError> Analyser::analysePrimaryExpression();
		//<assignment - expression> :: = <identifier><assignment - operator><expression>
		std::optional<CompilationError> analyseAssignmentExpression();
		//	<condition> :: = <expression>[<relational - operator><expression>]
		std::optional<CompilationError> analyseCondition(std::string& op);

		// 返回下一个 token
		std::optional<Token> nextToken();
		// 回退一个 token

		void addFunction(OneFunction);

		void unreadToken();

		// 下面是符号表相关操作

		// helper function
		void _add(const Token&,const _one_symbol&);
		// 添加变量、常量、未初始化的变量
		void addInit(const Token&,SymbolKind kind, SymbolType type);
		void addUninitialized(const Token&, SymbolKind kind, SymbolType type);

		void addConsttable(oneConst );
		void backLevel(int32_t);
		// 是否被声明过
		bool isDeclaredCurrentLevel(int32_t level, const std::string& s);
		bool isDeclared(const std::string& s);
		bool isDeclaredAndInit(const std::string& s);
		// 是否是未初始化的变量
		bool isUninitializedVariable(int32_t,const std::string&);
		//// 是否是已初始化的变量
		bool isInitializedVariable(int32_t,const std::string&);
		//// 是否是常量
		bool isConstant(int32_t,const std::string&);
		bool isFunctionInit(const std::string& s);
		_one_symbol getOneSymbol(std::string s);
		// 获得 {变量，常量} 在栈上的偏移
		int32_t getIndex(const std::string&);
		//获取在哪个栈里
		int32_t getIndexLevel(const std::string& s);
		//获取层级差
		int32_t getLevelDiff(const std::string& s);
		//获取函数在函数表里的位置
		int32_t getFunctionIndex(const std::string s);
		/*int32_t getFunctionSymbolTableIndex(const std::string s);*/
		bool numInRange(std::string, int32_t&,TokenType);
		bool isDeclaredAndConstant(const std::string& s);
	private:
		std::vector<Token> _tokens;
		std::size_t _offset;
		//std::vector<Instruction> _instructions;
		std::map<std::string, std::vector<Instruction>> _instructions;
		std::pair<uint64_t, uint64_t> _current_pos;
		
		
		// 为了简单处理，我们直接把符号表耦合在语法分析里
		//其实我是不太会语法翻译制导的，所以只能耦合了orz
		//使用栈式符号表

		
		
		//分程序索引表
		std::vector<_index_function> _index_functions;
		
		//栈式符号表
		std::vector<_one_symbol> _symbol_table;		
		std::vector<std::map<std::string, _one_symbol>> _symbol_vector_map;

		//常量表
		std::vector<oneConst> _const_table;
		//函数表
		std::vector<OneFunction> _functions_table;

		//当前函数名
		std::string _current_function_name = "0_begin";

		//这个是用来存放当前层级的
		int32_t _current_level = 0;

		//这个是用来存当前栈顶的
		int32_t _current_ss;
		//存放当前栈顶
		int32_t _current_sp;
		// 变量                   示例
		// _uninitialized_vars    int a;
		// _vars                  int a=1;
		// _consts                const a=1;
		/*std::map<std::string, int32_t> _uninitialized_vars;
		std::map<std::string, int32_t> _vars;
		std::map<std::string, int32_t> _consts;*/
		// 下一个 token 在栈的偏移,就是在符号表当前的栈顶
		std::vector<int32_t> _nextTokenIndex;
		int32_t _nextFunction;
	};
}