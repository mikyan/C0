#include "analyser.h"
#include <sstream>
#include <climits>

namespace miniplc0 {
	std::pair<std::vector<Instruction>, std::optional<CompilationError>> Analyser::Analyse() {
		auto err = analyseProgram();
		if (err.has_value())
			return std::make_pair(std::vector<Instruction>(), err);
		//这要改
		else
			return std::make_pair(_instructions[_current_function_name], std::optional<CompilationError>());
	}

	// <C0-program> ::= {<variable - declaration>}{<function - definition>}
	//	这里做了一个等价改写
	// <C0-program> ::= <variable - declaration> <function - definition>
	//<variable - declaration> :: = {<variable - declaration - statement>}
	//<variable - declaration - statement> ::=[<const-qualifier>]<type-specifier><init-declarator-list>';'
	//<function - definition> ::= {<function - definition- statement>}
	//<function - definition- statement> ::= <type-specifier><identifier><parameter-clause><compound-statement>
	//这样做的目的主要是为了让入口函数清晰些，不过这样会增加递归深度
	//<C0 - program> :: = <variable - declaration> <function - definition>
	std::optional<CompilationError> Analyser::analyseProgram() {

		_index_function index_function;
		index_function.ecount = 0;
		index_function.outern = 0;
		index_function.pointer = 0;
		_index_functions.emplace_back(index_function);
		_nextTokenIndex.emplace_back(0);
		std::map<std::string, _one_symbol> initmap;
		_symbol_vector_map.emplace_back(initmap);
		OneFunction onefunction;
		onefunction.level = 1;
		onefunction.index = _nextFunction;
		onefunction.name = "0_begin";
		onefunction.params_size = 0;
		onefunction.params_num = 0;
		onefunction.type = {};
		onefunction.retype = SymbolType::VOID_TYPE;
		addFunction(onefunction);
		//std::cout << _current_level << std::endl;
		auto err = analyseVariableDeclaration();
		if (err.has_value())
			return err;
		//std::cout << "variblefinish" << std::endl;

		err = analyseFunctionDefinition();
		if (err.has_value())
			return err;
		return {};
	}
	

	//<variable - declaration> :: = {<variable - declaration - statement>}
	//<variable - declaration - statement> ::=[<const-qualifier>]<type-specifier><init-declarator-list>';'
	//由于只写了基础c0，所以这里的<type-specifier>实际上应该是<simple-type-specifier>
	std::optional<CompilationError> Analyser::analyseVariableDeclaration() {
		int i = 0;
		while (true) {
			//std::cout << "times" << std::endl;
			//std::cout << i << std::endl;
			i++;
			bool isConstant = false;
			SymbolKind kind=SymbolKind::VAR_KIND;
			SymbolType type=SymbolType::VOID_TYPE;
			auto next = nextToken();
			//std::cout << next.value().GetValueString() << std::endl;
			//std::cout << "run here10" << std::endl;
			if (!next.has_value())
				return {};

			//如果无法处理
			if (next.value().GetType() != TokenType::CONST
				&& next.value().GetType() != TokenType::INT
				&& next.value().GetType() != TokenType::VOID) {
				unreadToken();
				return {};
			}
			//如果有const修饰符
			if (next.value().GetType() == TokenType::CONST) {
				isConstant = true;
				next = nextToken();
				next = nextToken();
				next = nextToken();
				unreadToken();
				unreadToken();
				unreadToken();
				if (next.value().GetType() == TokenType::LEFT_BRACKET) {
					return {};
				}
				//std::cout << next.value().GetValueString() << std::endl;
				next = nextToken();
				kind = SymbolKind::CONST_KIND;
				if (!next.has_value()) {
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedType);
				}
				if (next.value().GetType() != TokenType::INT &&
					next.value().GetType() != TokenType::VOID) {
					
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedType);
				}
				if (next.value().GetType() == TokenType::VOID) {
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrVoidType);
				}
				type = SymbolType::INT_TYPE;

			}
			else if(next.value().GetType() == TokenType::INT){
				next = nextToken();
				next = nextToken();
				unreadToken();
				unreadToken();
				if (next.value().GetType() == TokenType::LEFT_BRACKET) {
					unreadToken();
					return {};
				}
				kind = SymbolKind::VAR_KIND;
				type = SymbolType::INT_TYPE;
			}
			else if (next.value().GetType() == TokenType::VOID) {
				next = nextToken();
				next = nextToken();
				unreadToken();
				unreadToken();
				if (next.value().GetType() == TokenType::LEFT_BRACKET) {
					unreadToken();
					return {};
				}
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrVoidType);
			}
			//这个地方我怎么生成符号表？？？
			//这里不生成符号表，通过参数传递继承属性，在之后的函数里生成符号表
			//std::cout << "crazy" << std::endl;
			auto err = analyseInitList(kind,type);
			if (err.has_value()) {
				return err;
			}
			//std::cout << "crazy" << std::endl;
			next = nextToken();
			std::string s = next.value().GetValueString();
			//std::cout << s << std::endl;
			if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
			}
			//std::cout << "crazy" << std::endl;


		}

		return {};
	}
	//<init-declarator-list> ::= <init - declarator>{',' < init - declarator > }
	std::optional<CompilationError> Analyser::analyseInitList(SymbolKind kind,SymbolType type) {

		auto next = nextToken();
		next = nextToken();
		unreadToken();
		unreadToken();
		if (next.value().GetType() == TokenType::LEFT_BRACKET) {
			return {};
		}
		//std::cout << "assdsadasd" << std::endl;
		auto err = analyseInitDeclarator(kind,type);
		if (err.has_value()) {
			return err;
		}
		
		while (true) {
			//std::cout << "dasdadasdsadasdddddd" << std::endl;
			auto next = nextToken();
			if (!next.has_value()) {
				return {};
			}
			if (next.value().GetType() != TokenType::COMMA_STATE_SIGN) {
				unreadToken();
				return {};
			}

			next = nextToken();
			next = nextToken();
			unreadToken();
			unreadToken();
			if (next.value().GetType() == TokenType::LEFT_BRACKET) {
				return {};
			}
			//std::cout << "dasdadasdsadasdddddd" << std::endl;
			err = analyseInitDeclarator(kind, type);
			if (err.has_value()) {
				return err;
			}
		}
		return {};
	}
	//<init-declarator> ::= <identifier>[<initializer>]
	std::optional<CompilationError> Analyser::analyseInitDeclarator(SymbolKind kind, SymbolType type) {
		auto next = nextToken();
		if (!next.has_value()) {
			return {};
		}
		if (next.value().GetType() != TokenType::IDENTIFIER) {
			return {};
		}
		auto store = next;
		next = nextToken();
		if (!next.has_value()) {
			return {};
		}
		for(auto i:_nextTokenIndex)
			//std::cout << i << std::endl;

		if (isDeclaredCurrentLevel(_current_level, store.value().GetValueString())) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
		}
		//std::cout << "assdsadasd" << std::endl;
		addUninitialized(store.value(), kind, type);
		//std::cout << "assdsadasd" << std::endl;
		
		//std::cout << "now,level is:" << std::endl;
		//std::cout << _current_level << std::endl;
		_instructions[_current_function_name].emplace_back(Operation::SNEW, 1);
		_instructions[_current_function_name].emplace_back(Operation::LOADA, getLevelDiff(store.value().GetValueString()), getIndex(store.value().GetValueString()));

		unreadToken();
		if (next.value().GetType() == TokenType::EQUAL_SIGN) {
			
			
			auto err = analyseInitializer(kind, type);
			if (err.has_value()) {
				return err;
			}

			_symbol_vector_map[_current_level][store.value().GetValueString()].isInitial = true;
			//std::cout << "which is init" << std::endl;
			//std::cout << store.value().GetValueString() << std::endl;
			//std::cout << _current_level << std::endl;
			_instructions[_current_function_name].emplace_back(Operation::ISTORE, 0);

		}
		else {
			
			if (kind == SymbolKind::CONST_KIND) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConstantNeedValue);
			}
			else if(kind == SymbolKind::VAR_KIND) {
				_instructions[_current_function_name].emplace_back(Operation::IPUSH,0);
				_instructions[_current_function_name].emplace_back(Operation::ISTORE, 0);
	
			}
		}

		

		return {};
	}
	//<initializer> ::= '=' < expression >
	std::optional<CompilationError> Analyser::analyseInitializer(SymbolKind kind, SymbolType type) {
		auto next = nextToken();
		if (!next.has_value()) {
			return {};
		}

		if (next.value().GetType() != TokenType::EQUAL_SIGN) {
			unreadToken();
			return {};
		}

		auto err = analyseExpression(kind, type);
		if (err.has_value()) {
			return err;
		}

		return {};
	}

	//<function - definition> ::= {<function - definition- statement>}
	//<function-definition> ::= <type - specifier><identifier><parameter - clause><compound - statement>
	std::optional<CompilationError> Analyser::analyseFunctionDefinition(){
		while (true) {
			
			auto next = nextToken();
			if (!next.has_value())
				return {};
			//std::string s = next.value().GetValueString();
			//std::cout << "read::"+s << std::endl;
			//如果无法处理
			if (next.value().GetType() != TokenType::INT &&
				next.value().GetType() != TokenType::VOID) {
				unreadToken();
				return {};
			}
			SymbolType retype;
			if (next.value().GetType() == TokenType::INT) {
				retype = SymbolType::INT_TYPE;
				
			}else{
				retype = SymbolType::VOID_TYPE;
			}

			next = nextToken();
			if (next.value().GetType() != TokenType::IDENTIFIER) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
			}

			_index_function index_function;
			index_function.ecount = 0;
			index_function.outern = _current_level;
			index_function.pointer = _current_level+1;
			_index_functions.emplace_back(index_function);
			
			
			_current_function_name= next.value().GetValueString();
			


			//_functions_table[]
			int32_t params_size=0;
			int32_t params_num=0;
			std::vector<SymbolType> type_table;
			_current_level++;
			//std::cout << "++++++++++++++++++++++++++++" << std::endl;
			//std::cout << _current_level << std::endl;
			//std::cout << _nextTokenIndex.size() << std::endl;
			//std::cout << _symbol_vector_map.size() << std::endl;
			if (_symbol_vector_map.size() > _current_level) {
				std::map<std::string, _one_symbol> initmap;
				_symbol_vector_map[_current_level] = initmap;
			}
			else {
				std::map<std::string, _one_symbol> initmap;
				_symbol_vector_map.emplace_back(initmap);
			}
			if (_nextTokenIndex.size() > _current_level) {
				_nextTokenIndex[_current_level] = 0;
				
			}
			else {
				_nextTokenIndex.emplace_back(0);
				
			}
			//std::cout << "mygod1" << std::endl;
			auto err = analyseParameterClause(params_size,params_num,type_table);
			if (err.has_value()) {
				return err;
			}
			_current_level--;
			OneFunction onefunction;
			onefunction.level = 1;
			onefunction.index = _nextFunction;
			onefunction.name = next.value().GetValueString();
			onefunction.params_size = params_size;
			onefunction.params_num = params_num;
			onefunction.type = type_table;
			onefunction.retype = retype;
			addFunction(onefunction);
			oneConst constant;
			constant.name= next.value().GetValueString();
			constant.value = next.value().GetValueString();
			constant.type = ConstType::STRING_Const_Type;
			addConsttable(constant);

			//std::cout << "mygod" << std::endl;
			err = analyseCompoundStatement();
			if (err.has_value()) {
				return err;
			}

			
			
		}
		return {};
	}

	//<parameter - clause> :: ='('[<parameter - declaration - list>] ')'
	std::optional<CompilationError> Analyser::analyseParameterClause(int32_t& params_size, int32_t& params_num, std::vector<SymbolType>& type) {
		auto next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedRightBracket);
		if (next.value().GetType() != TokenType::LEFT_BRACKET) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedRightBracket);
		}

		//std::cout << "mygod2" << std::endl;
		auto err = analyseParameterDeclarationList(params_size,params_num,type);
		if (err.has_value()) {
			return err;
		}
		//std::cout << "mygod2" << std::endl;
		next = nextToken();
		std::string s = next.value().GetValueString();
		//std::cout << "read::" + s << std::endl;
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedRightBracket);
		if (next.value().GetType() != TokenType::RIGHT_BRACKET) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedRightBracket);
		}

		return {};
	}
	//<parameter-declaration-list> ::= <parameter - declaration>{',' < parameter - declaration > }
	std::optional<CompilationError> Analyser::analyseParameterDeclarationList(int32_t& params_size, int32_t& params_num, std::vector<SymbolType>& type) {
		auto err = analyseParameterDeclaration(params_size,params_num,type);
		if (err.has_value()) {
			return err;
		}
		while (true) {
			auto next = nextToken();
			if (!next.has_value()) {
				return {};
			}
			auto s = next.value().GetValueString();
			//std::cout << "read::" + s << std::endl;
			if (next.value().GetType() != TokenType::COMMA_STATE_SIGN) {
				unreadToken();
				return {};
			}

			auto err = analyseParameterDeclaration(params_size,params_num,type);
			if (err.has_value()) {
				return err;
			}
		}

		return {};
	}
	//<parameter-declaration> ::= [<const - qualifier>]<type - specifier><identifier>
	std::optional<CompilationError> Analyser::analyseParameterDeclaration(int32_t& params_size, int32_t& params_num, std::vector<SymbolType>& type) {
		auto next = nextToken();
		if (!next.has_value())
			return {};

		//如果无法处理
		if (next.value().GetType() != TokenType::CONST
			&& next.value().GetType() != TokenType::INT
			&& next.value().GetType() != TokenType::VOID) {
			unreadToken();
			return {};
		}
		std::string s = next.value().GetValueString();
		//std::cout << "read::" + s << std::endl;
		SymbolKind thekind = SymbolKind::VAR_KIND;
		SymbolType thetype = SymbolType::INT_TYPE;
		//如果有const修饰符
		if (next.value().GetType() == TokenType::CONST) {
			thekind = SymbolKind::CONST_KIND;
			next = nextToken();

			if (!next.has_value()) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedType);
			}
			if (next.value().GetType() != TokenType::INT &&
				next.value().GetType() != TokenType::VOID) {

				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedType);
			}
			if (next.value().GetType() == TokenType::VOID) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrVoidType);
			}
			else {
				thetype = SymbolType::INT_TYPE;
				type.emplace_back(SymbolType::INT_TYPE);
				params_size = params_size + 1;
				params_num = params_num + 1;
			}

		}
		else if (next.value().GetType() == TokenType::INT) {

			thetype = SymbolType::INT_TYPE;
			type.emplace_back(SymbolType::INT_TYPE);
			params_size = params_size + 1;
			params_num = params_num + 1;
		}
		else if (next.value().GetType() == TokenType::VOID) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrVoidType);
		}
		//符号表？
		//初始化为未定义的变量或常量
		next = nextToken();
		s = next.value().GetValueString();
		//std::cout << "read::" + s << std::endl;
		if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
		}
		//在这个位置只生成符号表，不需要对栈进行操作（不需要生成指令）因为跳转的时候会把参数压栈
		//是按已初始化的变量来声明的，因为调用时一定要传参
		addInit(next.value(),thekind,thetype);
		s = next.value().GetValueString();
		//std::cout << "read::" + s << std::endl;

		return {};
	}

	//<function-call> ::= <identifier> '('[<expression - list>] ')'
	std::optional<CompilationError> Analyser::analyseFunctionCall() {
		auto next = nextToken();
		if (!next.has_value())
			return {};

		//如果无法处理
		if (next.value().GetType() != TokenType::IDENTIFIER) {
			unreadToken();
			return {};
		}
		auto s = next.value().GetValueString();
		//std::cout << s << std::endl;
		if (!isFunctionInit(next.value().GetValueString())) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
		}

		auto func = next;

		next = nextToken();
		if (!next.has_value())
			return {};
		if (next.value().GetType() != TokenType::LEFT_BRACKET) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedLeftBracket);
		}

		//int32_t function_symbol_index = getFunctionSymbolTableIndex(func.value().GetValueString());
		if (_functions_table[getFunctionIndex(func.value().GetValueString())].params_num > 0) {
			auto err = analyseExpressionList(func.value().GetValueString());
			if (err.has_value()) {
				return err;
			}
		}
		
		next = nextToken();
		if (!next.has_value())
			return {};
		if (next.value().GetType() != TokenType::RIGHT_BRACKET) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedRightBracket);
		}

		_instructions[_current_function_name].emplace_back(Operation::CALL,getFunctionIndex(func.value().GetValueString())-1);
		return {};
	}

	//<expression-list> ::= <expression>{',' < expression > }
	std::optional<CompilationError> Analyser::analyseExpressionList(const std::string name) {
		int index = getFunctionIndex(name);
		int32_t params_num = _functions_table[index].params_num;
		
		int i = 0;
		SymbolType type = _functions_table[index].type[i];

		auto err = analyseExpression(SymbolKind::VAR_KIND,type);
		if (err.has_value()) {
			return err;
		}
		i++;
		while (i<params_num) {
			SymbolType type = _functions_table[index].type[i];
			auto next = nextToken();
			if (!next.has_value()) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedExpression);
			}
			if (next.value().GetType() != TokenType::COMMA_STATE_SIGN) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedExpression);
			}

			auto err = analyseExpression(SymbolKind::VAR_KIND, type);
			if (err.has_value()) {
				return err;
			}
			i++;

		}

		return {};
	}

	//<compound-statement> ::= '{' {<variable - declaration>} < statement - seq> '}'
	std::optional<CompilationError> Analyser::analyseCompoundStatement() {
		auto next = nextToken();
		if (!next.has_value())
			return {};
		if (next.value().GetType() != TokenType::LEFT_BRACES_SIGN) {
			unreadToken();
			return {};
		}
		_current_level++;

		while (true) {
			bool isConstant = false;
			SymbolKind kind = SymbolKind::VAR_KIND;
			SymbolType type = SymbolType::VOID_TYPE;
			auto next = nextToken();
			//std::cout << next.value().GetValueString() << std::endl;
			//std::cout << "run here10" << std::endl;
			if (!next.has_value())
				break;

			//如果无法处理
			if (next.value().GetType() != TokenType::CONST
				&& next.value().GetType() != TokenType::INT
				&& next.value().GetType() != TokenType::VOID) {
				unreadToken();
				break;
			}
			//std::cout << "why" << std::endl;
			//如果有const修饰符
			if (next.value().GetType() == TokenType::CONST) {
				isConstant = true;
				next = nextToken();
				kind = SymbolKind::CONST_KIND;
				if (!next.has_value()) {
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedType);
				}
				if (next.value().GetType() != TokenType::INT &&
					next.value().GetType() != TokenType::VOID) {

					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedType);
				}
				if (next.value().GetType() == TokenType::VOID) {
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrVoidType);
				}
				type = SymbolType::INT_TYPE;

			}
			else if (next.value().GetType() == TokenType::INT) {
				kind = SymbolKind::VAR_KIND;
				type = SymbolType::INT_TYPE;
			}
			else if (next.value().GetType() == TokenType::VOID) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrVoidType);
			}
			//这个地方我怎么生成符号表？？？
			//这里不生成符号表，通过参数传递继承属性，在之后的函数里生成符号表

			auto err = analyseInitList(kind, type);
			if (err.has_value()) {
				return err;
			}
			//std::cout << "hahhaha" << std::endl;
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
			}
		}
		//std::cout << "hahhaha" << std::endl;
		auto err = analyseStatementSeq();
		if (err.has_value()) {
			return err;
		}
		//std::cout << "hahhaha" << std::endl;
		next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedRightBraces);
		if (next.value().GetType() != TokenType::RIGHT_BRACES_SIGN) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedRightBraces);
		}
		_current_level--;
		return {};
	}

	//<statement-seq> ::= {<statement>}


	std::optional<CompilationError> Analyser::analyseStatementSeq() {
		while (true) {
			// 预读
			auto next = nextToken();
			if (!next.has_value())
				return {};
			unreadToken();//所有的东西都会放到下一个递归里处理
			if (next.value().GetType() != TokenType::LEFT_BRACES_SIGN &&
				next.value().GetType() != TokenType::IF &&
				next.value().GetType() != TokenType::WHILE &&
				next.value().GetType() != TokenType::RETURN &&
				next.value().GetType() != TokenType::PRINT &&
				next.value().GetType() != TokenType::SCAN &&
				//这里无法用identifier区分assignmentexpression和functioncall
				//还要再预读一下
				next.value().GetType() != TokenType::IDENTIFIER &&
				next.value().GetType() != TokenType::SEMICOLON) {
				return {};
			}
			//std::cout << "what" << std::endl;
			//std::cout << next.value().GetValueString() << std::endl;
			//std::cout << "run here11" << std::endl;
			std::optional<CompilationError> err;
			switch (next.value().GetType()) {
			case LEFT_BRACES_SIGN: {

				auto next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACES_SIGN) {
					return {};
				}

				auto err = analyseStatementSeq();
				if (err.has_value()) {
					return err;
				}
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACES_SIGN) {
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedRightBraces);
				}
				break;
			}
			case IF: {
				auto err = analyseConditionStatement();
				if (err.has_value()) {
					return err;
				}
				break;
			}
			case WHILE: {
				auto err = analyseLoopStatement();
				if (err.has_value()) {
					return err;
				}
				break;
			}
			case RETURN: {
				auto err = analyseJumpStatement();
				if (err.has_value()) {
					return err;
				}
				break;
			}
			case PRINT: {
				auto err = analysePrintStatement();
				if (err.has_value()) {
					return err;
				}
				break;
			}
			case SCAN: {
				auto err = analyseScanStatement();
				if (err.has_value()) {
					return err;
				}
				break;
			}
					 //再预读一次
			case IDENTIFIER: {
				auto next = nextToken();
				next = nextToken();
				unreadToken();
				unreadToken();
				if (!next.has_value()) {
					return {};
				}
				if (next.value().GetType() == TokenType::LEFT_BRACKET) {

					auto err = analyseFunctionCall();
					if (err.has_value()) {
						return err;
					}
				}
				else if (next.value().GetType() == TokenType::EQUAL_SIGN) {
					auto err = analyseAssignmentExpression();
					if (err.has_value()) {
						return err;
					}
				}
				else {
					return {};
				}

				break;
			}
			case SEMICOLON: {
				auto next = nextToken();
				break;
			}
						  // 这里需要你针对不同的预读结果来调用不同的子程序
						  // 注意我们没有针对空语句单独声明一个函数，因此可以直接在这里返回
			default:
				break;
			}
		}
		return {};
	}

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
	std::optional<CompilationError> Analyser::analyseStatement() {
		// 预读
		auto next = nextToken();
		if (!next.has_value())
			return {};
		unreadToken();//所有的东西都会放到下一个递归里处理
		if (next.value().GetType() != TokenType::LEFT_BRACES_SIGN &&
			next.value().GetType() != TokenType::IF &&
			next.value().GetType() != TokenType::WHILE &&
			next.value().GetType() != TokenType::RETURN &&
			next.value().GetType() != TokenType::PRINT &&
			next.value().GetType() != TokenType::SCAN &&
			//这里无法用identifier区分assignmentexpression和functioncall
			//还要再预读一下
			next.value().GetType() != TokenType::IDENTIFIER &&
			next.value().GetType() != TokenType::SEMICOLON) {
			return {};
		}
		//std::cout << next.value().GetValueString() << std::endl;
		//std::cout << "run here11" << std::endl;
		std::optional<CompilationError> err;
		switch (next.value().GetType()) {
		case LEFT_BRACES_SIGN: {

			auto next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACES_SIGN) {
				return {};
			}

			auto err = analyseStatementSeq();
			if (err.has_value()) {
				return err;
			}
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACES_SIGN) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedRightBraces);
			}
			break;
		}
		case IF: {
			auto err = analyseConditionStatement();
			if (err.has_value()) {
				return err;
			}
			break;
		}
		case WHILE: {
			auto err = analyseLoopStatement();
			if (err.has_value()) {
				return err;
			}
			break;
		}
		case RETURN: {
			auto err = analyseJumpStatement();
			if (err.has_value()) {
				return err;
			}
			break;
		}
		case PRINT: {
			auto err = analysePrintStatement();
			if (err.has_value()) {
				return err;
			}
			break;
		}
		case SCAN: {
			auto err = analyseScanStatement();
			if (err.has_value()) {
				return err;
			}
			break;
		}
				 //再预读一次
		case IDENTIFIER: {
			auto next = nextToken();
			next = nextToken();
			unreadToken();
			unreadToken();
			if (!next.has_value()) {
				return {};
			}
			if (next.value().GetType() == TokenType::LEFT_BRACKET) {

				auto err = analyseFunctionCall();
				if (err.has_value()) {
					return err;
				}
			}
			else if (next.value().GetType() == TokenType::EQUAL_SIGN) {
				auto err = analyseAssignmentExpression();
				if (err.has_value()) {
					return err;
				}
			}
			else {
				return {};
			}

			break;
		}
		case SEMICOLON: {
			auto next = nextToken();
			break;
		}
					  // 这里需要你针对不同的预读结果来调用不同的子程序
					  // 注意我们没有针对空语句单独声明一个函数，因此可以直接在这里返回
		default:
			break;
		}

		return {};
	}

	//<jump-statement> ::= <return-statement>
	std::optional<CompilationError> Analyser::analyseJumpStatement() {
		auto err = analyseReturnStatement();
		if (err.has_value()) {
			return err;
		}
		return {};
	}
	//<return-statement> ::= 'return' [<expression>] ';'
	std::optional<CompilationError> Analyser::analyseReturnStatement() {
		//std::cout << "runhere1" << std::endl;
		auto nowfunc = _nextFunction - 1;
		SymbolType type = _functions_table[nowfunc].retype;
		auto next = nextToken();
		if (!next.has_value())
			return {};
		if (next.value().GetType() != TokenType::RETURN ){
			unreadToken();
			return {};
		}
		auto s = next.value().GetValueString();
		//std::cout << s << std::endl;
		if (type == SymbolType::INT_TYPE) {
			auto err = analyseExpression(SymbolKind::VAR_KIND, type);
			if (err.has_value()) {
				return err;
			}
		}
		else if(type == SymbolType::VOID_TYPE){
			next = nextToken();
			if (next.value().GetType() != TokenType::SEMICOLON) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
			}
			else {
				unreadToken();
			}
		}

		next = nextToken();
		if (!next.has_value())
			return {};
		if (next.value().GetType() != TokenType::SEMICOLON) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
		}

		if (type == SymbolType::INT_TYPE) {
			_instructions[_current_function_name].emplace_back(Operation::IRET, 0);
		}
		else if (type == SymbolType::VOID_TYPE) {
			_instructions[_current_function_name].emplace_back(Operation::RET, 0);
		}


		return {};
	}

	//<condition-statement> ::= 'if' '(' < condition > ')' < statement > ['else' < statement > ]
	std::optional<CompilationError> Analyser::analyseConditionStatement() {
		auto next = nextToken();
		if (!next.has_value())
			return {};
		if (next.value().GetType() != TokenType::IF) {
			return {};
		}
		next = nextToken();
		if (!next.has_value()||next.value().GetType() != TokenType::LEFT_BRACKET) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedLeftBracket);
		}

		std::string op;

		auto err=analyseCondition(op);
		if (err.has_value()) {
			return err;
		}

		int ifjumpindex = _instructions[_current_function_name].size();

		if (op == "<") {
			_instructions[_current_function_name].emplace_back(Operation::JGE, 0);
		}
		else if (op == "<=") {
			_instructions[_current_function_name].emplace_back(Operation::JG, 0);
		}
		else if (op == ">") {
			_instructions[_current_function_name].emplace_back(Operation::JLE, 0);
		}
		else if (op == ">=") {
			_instructions[_current_function_name].emplace_back(Operation::JL, 0);
		}
		else if (op == "==") {
			_instructions[_current_function_name].emplace_back(Operation::JNE, 0);
		}
		else if (op == "!=") {
			_instructions[_current_function_name].emplace_back(Operation::JE, 0);
		}
		else {
			_instructions[_current_function_name].emplace_back(Operation::JE, 0);
		}
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedRightBracket);
		}
		err = analyseStatement();
		if (err.has_value()) {
			return err;
		}

		next = nextToken();
		if (!next.has_value()) {
			return {};
		}

		if (next.value().GetType() == TokenType::ELSE) {

			//执行完if内容，需要跳过else的内容
			int elsejumpindex = _instructions[_current_function_name].size();
			_instructions[_current_function_name].emplace_back(Operation::JMP, 0);
			int jumptoelseindex = _instructions[_current_function_name].size();
			_instructions[_current_function_name][ifjumpindex].SetX(jumptoelseindex);
			auto err = analyseStatement();
			if (err.has_value()) {
				return err;
			}
			int jumpfinalindex = _instructions[_current_function_name].size();
			_instructions[_current_function_name][elsejumpindex].SetX(jumpfinalindex);
			


		}


		else {
			int jumpfinalsindex = _instructions[_current_function_name].size();
			_instructions[_current_function_name][ifjumpindex].SetX(jumpfinalsindex);
			unreadToken();

		}
		return {};
	}
	//<loop-statement> ::= 'while' '(' < condition > ')' < statement >
	std::optional<CompilationError> Analyser::analyseLoopStatement() {
		auto next = nextToken();
		if (!next.has_value())
			return {};
		if (next.value().GetType() != TokenType::WHILE) {
			return {};
		}
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedLeftBracket);
		}

		std::string op;

		int32_t beforecompareindex = _instructions[_current_function_name].size();
		auto err = analyseCondition(op);
		if (err.has_value()) {
			return err;
		}
		int32_t compareindex = _instructions[_current_function_name].size();
		if (op == "<") {
			_instructions[_current_function_name].emplace_back(Operation::JGE, 0);
		}
		else if (op == "<=") {
			_instructions[_current_function_name].emplace_back(Operation::JG, 0);
		}
		else if (op == ">") {
			_instructions[_current_function_name].emplace_back(Operation::JLE, 0);
		}
		else if (op == ">=") {
			_instructions[_current_function_name].emplace_back(Operation::JL, 0);
		}
		else if (op == "==") {
			_instructions[_current_function_name].emplace_back(Operation::JNE, 0);
		}
		else if (op == "!=") {
			_instructions[_current_function_name].emplace_back(Operation::JE, 0);
		}
		else {
			_instructions[_current_function_name].emplace_back(Operation::JE, 0);
		}

		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedRightBracket);
		}


		err = analyseStatement();
		if (err.has_value()) {
			return err;
		}

		int32_t whilefinalindex= _instructions[_current_function_name].size();
		_instructions[_current_function_name].emplace_back(Operation::JMP, beforecompareindex);
		int32_t comparejumptoindex = _instructions[_current_function_name].size();
		_instructions[_current_function_name][compareindex].SetX(comparejumptoindex);

		return {};
	}

	//<scan - statement>  :: = 'scan' '(' < identifier > ')' ';'
	std::optional<CompilationError> Analyser::analyseScanStatement() {
		auto next = nextToken();
		if (!next.has_value())
			return {};
		if (next.value().GetType() != TokenType::SCAN) {
			return {};
		}
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedLeftBracket);
		}
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
		}

		//判断是否被声明，且不是常量
		if (!isDeclared(next.value().GetValueString())) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
		}
		if (isDeclaredAndConstant(next.value().GetValueString())) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);
		}

		_instructions[_current_function_name].emplace_back(Operation::LOADA, getLevelDiff(next.value().GetValueString()),getIndex(next.value().GetValueString()));
		_instructions[_current_function_name].emplace_back(Operation::ISCAN, 0);
		_instructions[_current_function_name].emplace_back(Operation::ISTORE, getIndex(next.value().GetValueString()));
		_symbol_vector_map[getIndexLevel(next.value().GetValueString())][next.value().GetValueString()].isInitial = true;
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedRightBracket);
		}
		
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
		}
		
		

		return {};
	}

	//< print - statement > :: = 'print' '('[<printable - list>] ')' ';'
	std::optional<CompilationError> Analyser::analysePrintStatement() {
		auto next = nextToken();
		if (!next.has_value())
			return {};
		if (next.value().GetType() != TokenType::PRINT) {
			return {};
		}
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedLeftBracket);
		}

		int32_t printable_num=0;
		//预读判断
		next = nextToken();
		if (next.value().GetType()!= TokenType::RIGHT_BRACKET) {
			unreadToken();
			auto err = analysePrintableList(printable_num);
			if (err.has_value()) {
				return err;
			}
		} else{
			unreadToken();
		}

		

		_instructions[_current_function_name].emplace_back(Operation::PRINTL, 0);

		
		
		next = nextToken();
		//std::cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << std::endl;
		//std::cout << next.value().GetValueString() << std::endl;
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedRightBracket);
		}

		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
		}

		return {};
	}

	//< printable - list > :: = <printable>{ ',' < printable > }
	std::optional<CompilationError> Analyser::analysePrintableList(int32_t& printable_num) {
		auto err = analysePrintable(SymbolKind::VAR_KIND,SymbolType::INT_TYPE);
		if (err.has_value()) {
			
			
			return err;
		}
		_instructions[_current_function_name].emplace_back(Operation::IPRINT, 0);
		printable_num++;
		while (true) {
			auto next = nextToken();
			if (!next.has_value())
				return {};
			if (next.value().GetType() != TokenType::COMMA_STATE_SIGN) {
				unreadToken();
				return {};
			}
			_instructions[_current_function_name].emplace_back(Operation::BIPUSH, 32);
			_instructions[_current_function_name].emplace_back(Operation::CPRINT, 0);

			//std::cout << "+_+_+_+_+_+_++_+_+_+_+_+_+_+" << std::endl;
			//std::cout << next.value().GetValueString() << std::endl;
			auto err = analysePrintable(SymbolKind::VAR_KIND, SymbolType::INT_TYPE);
			if (err.has_value()) {
				return err;
			}
			_instructions[_current_function_name].emplace_back(Operation::IPRINT, 0);
			
			printable_num++;
		}


		return {};
	}
	//<printable> :: = <expression>
	std::optional<CompilationError> Analyser::analysePrintable(SymbolKind kind, SymbolType type) {
		auto err = analyseExpression(kind, type);
		if (err.has_value()) {
			return err;
		}
		return {};
	}

	//<expression> :: =<additive - expression>
	std::optional<CompilationError> Analyser::analyseExpression(SymbolKind kind, SymbolType type) {
		auto err = analyseAdditiveExpression(kind, type);
		if (err.has_value()) {
			return err;
		}
		return {};
	}
	//	<additive - expression> :: =<multiplicative - expression>{ <additive - operator><multiplicative - expression> }
	std::optional<CompilationError> Analyser::analyseAdditiveExpression(SymbolKind kind, SymbolType type) {
		auto err = analyseMultiplicativeExpression(kind,type);
		if (err.has_value()) {
			return err;
		}
		
		while (true) {
			auto next = nextToken();
			if (!next.has_value()) {
				return {};
			}
			if (next.value().GetType() == TokenType::MINUS_SIGN || next.value().GetType() == TokenType::PLUS_SIGN) {

				auto err = analyseMultiplicativeExpression(kind, type);
				if (err.has_value()) {
					return err;
				}

				if (next.value().GetType() == TokenType::MINUS_SIGN) {
					if(type==SymbolType::INT_TYPE)
						_instructions[_current_function_name].emplace_back(Operation::ISUB, 0);
				}
				else {
					if (type == SymbolType::INT_TYPE)
						_instructions[_current_function_name].emplace_back(Operation::IADD, 0);
				}
			}
			else {
				unreadToken();
				return {};
			}
		}
		
		return {};
	}

	//	<multiplicative - expression> :: =<cast - expression>{ <multiplicative - operator><cast - expression> }
	//由于是基础c0，所以这里以下面的为准
	//<multiplicative-expression> ::= <unary - expression>{<multiplicative - operator><unary - expression>}
	std::optional<CompilationError> Analyser::analyseMultiplicativeExpression(SymbolKind kind, SymbolType type) {
		auto err = analyseUnaryExpression(kind,type);
		if (err.has_value()) {
			return err;
		}

		while (true) {
			auto next = nextToken();
			//std::cout << "*******************&&&&&&" << std::endl;
			//std::cout << next.value().GetValueString() << std::endl;
			if (!next.has_value()) {
				return {};
			}
			if (next.value().GetType() == TokenType::MULTIPLICATION_SIGN || next.value().GetType() == TokenType::DIVISION_SIGN) {
				auto err = analyseUnaryExpression(kind,type);
				if (err.has_value()) {
					return err;
				}

				if (next.value().GetType() == TokenType::MULTIPLICATION_SIGN) {
					if (type == SymbolType::INT_TYPE) {
						_instructions[_current_function_name].emplace_back(Operation::IMUL,0);
					}
				}
				else {
					if (type == SymbolType::INT_TYPE) {
						_instructions[_current_function_name].emplace_back(Operation::IDIV, 0);
					}
				}

			}
			else {
				unreadToken();
				return {};
			}
		}

		return {};
	}
	//	<cast - expression> :: ={ '(' < type - specifier > ')' }<unary - expression>
	//std::optional<CompilationError> Analyser::analyseCastExpression() {
	//	while (true) {
	//	}
	//	auto err = analyseUnaryExpression();
	//	if (err.has_value()) {
	//		return err;
	//	}
	//	return {};
	//}

	//	<unary - expression> :: =[<unary - operator>]<primary - expression>
		//	<primary - expression> :: =
	//	'(' < expression > ')'
	//	| <identifier>
	//	| <integer - literal>
	//	| <function - call>
	std::optional<CompilationError> Analyser::analyseUnaryExpression(SymbolKind kind, SymbolType type) {
		// [<符号>]
		auto next = nextToken();
		auto prefix = 1;
		//std::cout << "run here4" << std::endl;
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		if (next.value().GetType() == TokenType::PLUS_SIGN)
			prefix = 1;
		else if (next.value().GetType() == TokenType::MINUS_SIGN) {
			prefix = -1;
			if(type==SymbolType::INT_TYPE)
				_instructions[_current_function_name].emplace_back(Operation::IPUSH, 0);
		}
		else
			unreadToken();

		// 预读
		next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		switch (next.value().GetType()) {
		case IDENTIFIER:{
			auto nexttmp = nextToken();
			//std::cout << "46565656565656565656565656565"<<std::endl;
			//std::cout << nexttmp.value().GetValueString() << std::endl;
			unreadToken();
			if (!nexttmp.has_value()) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
			}
			//分析为表示符
			if (nexttmp.value().GetType() != TokenType::LEFT_BRACKET) {
				if (!isDeclared(next.value().GetValueString()))
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
				if (!isDeclaredAndInit(next.value().GetValueString())) {
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);
				}

				//std::cout << "now,levelsadasdsada is:" << std::endl;
				//std::cout << _current_level << std::endl;
				//本系统范围为-(2^31-1)~2^31-1
				_instructions[_current_function_name].emplace_back(Operation::LOADA, getLevelDiff(next.value().GetValueString()),getIndex(next.value().GetValueString()));
				_instructions[_current_function_name].emplace_back(Operation::ILOAD, 0);
			}
			//分析为函数
			else {
				unreadToken();
				auto err = analyseFunctionCall();
				if (err.has_value()) {
					return err;
				}
			}
			
			break;
		}
		case UNSIGNED_INTEGER:
		case HEX_INTEGER: {
			std::string temp = next.value().GetValueString();
			int32_t out;
			if (!numInRange(temp, out,next.value().GetType())) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIntegerOverflow);
			}
			//std::cout << prefix << std::endl;
			_instructions[_current_function_name].emplace_back(Operation::IPUSH, out);
			break;
		}
		case TokenType::LEFT_BRACKET: {
			auto err = analyseExpression(kind,type);
			if (err.has_value()) {
				return err;
			}
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
			break;
		}

									// 这里和 <语句序列> 类似，需要根据预读结果调用不同的子程序
									// 但是要注意 default 返回的是一个编译错误，
		default:
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		}

		// 取负
		if (prefix == -1 && type == SymbolType::INT_TYPE)
			_instructions[_current_function_name].emplace_back(Operation::ISUB, 0);
		return {};
	}



	//<assignment - expression> :: = <identifier><assignment - operator><expression>
	std::optional<CompilationError> Analyser::analyseAssignmentExpression() {
		auto next = nextToken();
		auto s = next.value().GetValueString();
		//std::cout << s << std::endl;
		if (!next.has_value())
			return {};
		if (next.value().GetType() != TokenType::IDENTIFIER) {
			unreadToken();
			return {};
		}
		 s = next.value().GetValueString();
		//std::cout << s << std::endl;

		if (!isDeclared(next.value().GetValueString())) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
		}
		//isDeclaredAndConstant
		if (isDeclaredAndConstant(next.value().GetValueString())) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);
		}
		_one_symbol symbol = getOneSymbol(next.value().GetValueString());
		auto temp = next;
		//存疑，有个bug,好了
		//std::cout << "now,level is:" << std::endl;
		//std::cout << _current_level << std::endl;
		_instructions[_current_function_name].emplace_back(Operation::LOADA, getLevelDiff(temp.value().GetValueString()), getIndex(temp.value().GetValueString()));
		//_instructions[_current_function_name].emplace_back(Operation::ILOAD,0);
		next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedAssignmentOperator);
		if (next.value().GetType() != TokenType::EQUAL_SIGN) {
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedAssignmentOperator);
		}

		auto err = analyseExpression(symbol.kind,symbol.type);
		if (err.has_value()) {
			return err;
		}

		_instructions[_current_function_name].emplace_back(Operation::ISTORE, getIndex(temp.value().GetValueString()));
		_symbol_vector_map[getIndexLevel(temp.value().GetValueString())][temp.value().GetValueString()].isInitial = true;

		return {};
	}
	//	<condition> :: = <expression>[<relational - operator><expression>]
	std::optional<CompilationError> Analyser::analyseCondition(std::string& op) {
		auto err = analyseExpression(SymbolKind::VAR_KIND,SymbolType::INT_TYPE); 
		if (err.has_value()) {
			return err;
		}
		auto next = nextToken();

		switch (next.value().GetType())
		{
		case TokenType::LESS_SIGN:
			op = "<";
			break;
		case TokenType::LESS_EQUAL_SIGN:
			op = "<=";
			break;
		case TokenType::GREATER_SIGN:
			op = ">";
			break;
		case TokenType::GREATER_EQUAL_SIGN:
			op = ">=";
			break;
		case TokenType::EQUAL_EQUAL_SIGN:
			op = "==";
			break;
		case TokenType::NOT_EQUAL_SIGN:
			op = "!=";
			break;
		default: {
			unreadToken();
			return {};
		}
		}

		err = analyseExpression(SymbolKind::VAR_KIND, SymbolType::INT_TYPE);
		if (err.has_value()) {
			return err;
		}

		_instructions[_current_function_name].emplace_back(Operation::ICMP,0);
		return {};
	}



	std::optional<Token> Analyser::nextToken() {
		if (_offset == _tokens.size())
			return {};
		// 考虑到 _tokens[0..._offset-1] 已经被分析过了
		// 所以我们选择 _tokens[0..._offset-1] 的 EndPos 作为当前位置
		_current_pos = _tokens[_offset].GetEndPos();
		return _tokens[_offset++];
	}

	void Analyser::unreadToken() {
		if (_offset == 0)
			DieAndPrint("analyser unreads token from the begining.");
		_current_pos = _tokens[_offset - 1].GetEndPos();
		_offset--;
	}

	void Analyser::addConsttable(oneConst constant) {
		_const_table.emplace_back(constant);
	}

	void Analyser::_add(const Token& tk,const _one_symbol& tk_symbol) {
		if (tk.GetType() != TokenType::IDENTIFIER)
			DieAndPrint("only identifier can be added to the table.");

		_symbol_vector_map[_current_level][tk.GetValueString()] = tk_symbol;
		//std::cout << "iamok" << std::endl;
		//_symbol_table[_nextTokenIndex[_current_level]] = tk_symbol;
		_nextTokenIndex[_current_level]++;
	}

	void Analyser::addInit(const Token& tk, SymbolKind kind, SymbolType type) {
		_one_symbol tk_symbol;
		tk_symbol.name = tk.GetValueString();
		tk_symbol.level = _current_level;
		tk_symbol.kind = kind;
		tk_symbol.type = type;
		tk_symbol.isInitial = true;
		tk_symbol.index = _nextTokenIndex[_current_level];
		_add(tk, tk_symbol);
	}


	void Analyser::addUninitialized(const Token& tk, SymbolKind kind, SymbolType type) {
		//std::cout << "current_level:" << std::endl;
		//std::cout << _current_level << std::endl;
		
		_one_symbol tk_symbol;
		tk_symbol.name = tk.GetValueString();
		tk_symbol.level = _current_level;
		tk_symbol.kind = kind;
		tk_symbol.type = type;
		tk_symbol.isInitial = false;
		//std::cout << "ready" << std::endl;
		tk_symbol.index = _nextTokenIndex[_current_level];
		//std::cout << "go" << std::endl;
		_add(tk,tk_symbol);
	}

	void Analyser::addFunction(OneFunction onefuntion) {
		_functions_table.emplace_back(onefuntion);
		_nextFunction++;

	}

	int32_t Analyser::getIndex(const std::string& s) {
		//for (int i = _current_ss; i < _current_sp;i++) {
		//	if (_symbol_table[i].name == s) {
		//		return i;
		//	}
		//}
		int32_t level = _current_level;
		while (true) {
			if (_symbol_vector_map[level].find(s) != _symbol_vector_map[level].end()) {
				//std::cout << "=========================" << std::endl;
				//std::cout << _symbol_vector_map[level][s].index << std::endl;
				//std::cout << "----------------------" << std::endl;
				//std::cout << level << std::endl;
				return _symbol_vector_map[level][s].index;
				
			}
			if (level == 0) {
				break;
			}
			level = _index_functions[level].outern;
		}
		return _symbol_vector_map[level][s].index;
		
	}

	int32_t Analyser::getIndexLevel(const std::string& s) {
		int32_t level = _current_level;
		while (true) {
			if (_symbol_vector_map[level].find(s) != _symbol_vector_map[level].end()) {
				return level;
			}
			if (level == 0) {
				break;
			}
			level = _index_functions[level].outern;
		}
		return level;
	}

	int32_t Analyser::getLevelDiff(const std::string& s) {
		return _current_level - getIndexLevel(s);
	}

	//bool Analyser::isDeclared(const std::string& s) {
	//	return isConstant(s) || isUninitializedVariable(s) || isInitializedVariable(s);
	//}
	bool Analyser::isDeclaredCurrentLevel(int32_t level, const std::string& s) {
		return _symbol_vector_map[level].find(s) != _symbol_vector_map[level].end();
	}

	bool Analyser::isDeclared(const std::string& s) {
		int32_t level=_current_level;
		//std::cout << level << std::endl;
		while (true) {
			if (_symbol_vector_map[level].find(s) != _symbol_vector_map[level].end()) {
				//std::cout << level << std::endl;
				return _symbol_vector_map[level].find(s) != _symbol_vector_map[level].end();
			}
			//std::cout << level << std::endl;
			if (level == 0) {
				return false;
			}
			//std::cout << level << std::endl;
			level = _index_functions[level].outern;
			//std::cout << level << std::endl;


		}
		return false;
	}

	bool Analyser::isDeclaredAndInit(const std::string& s) {
		int32_t level = _current_level;
		while (true) {
			if (_symbol_vector_map[level].find(s) != _symbol_vector_map[level].end()) {
				return _symbol_vector_map[level][s].isInitial;
				
			}
			if (level == 0) {
				return false;
			}
			level = _index_functions[level].outern;


		}
		return false;
	}
	bool Analyser::isDeclaredAndConstant(const std::string& s) {
		int32_t level = _current_level;
		//std::cout << level << std::endl;
		while (true) {
			if (_symbol_vector_map[level].find(s) != _symbol_vector_map[level].end()) {
				return _symbol_vector_map[level][s].kind==SymbolKind::CONST_KIND;

			}
			if (level == 0) {
				return false;
			}
			level = _index_functions[level].outern;


		}
		return false;
	}


	bool Analyser::isUninitializedVariable(int32_t level,const std::string& s) {
		if (_symbol_vector_map[level].find(s) != _symbol_vector_map[level].end()) {
			return  !_symbol_vector_map[level][s].isInitial &&
				_symbol_vector_map[level][s].kind == SymbolKind::VAR_KIND;
		}
		return false;
	}
	bool Analyser::isInitializedVariable(int32_t level,const std::string& s) {
		if (_symbol_vector_map[level].find(s) != _symbol_vector_map[level].end()) {
			return  _symbol_vector_map[level][s].isInitial &&
				_symbol_vector_map[level][s].kind == SymbolKind::VAR_KIND;
		}
		return false;
	}
	// 是否是常量
	bool Analyser::isConstant(int32_t level, const std::string& s) {
		if (_symbol_vector_map[level].find(s) != _symbol_vector_map[level].end()) {
			return  _symbol_vector_map[level][s].kind == SymbolKind::CONST_KIND;
		}
		return false;
	}

	_one_symbol Analyser::getOneSymbol(std::string s) {
		int32_t level = _current_level;
		while (true) {
			if (_symbol_vector_map[level].find(s) != _symbol_vector_map[level].end()) {
				return _symbol_vector_map[level][s];

			}
			if (level == 0) {
				break;
			}
			level = _index_functions[level].outern;


		}
		return {};
	}
	
	bool Analyser::isFunctionInit(const std::string& s) {
		for (auto f : _functions_table) {
			if (f.name == s) {
				return true;
			}
		}
		return false;
	}

	int32_t Analyser::getFunctionIndex(const std::string s) {
		int i = 0;
		for (auto f : _functions_table) {
			if (f.name == s) {
				return i;
			}
			i++;
		}
		return -1;
	}



	/*int32_t Analyser::getFunctionSymbolTableIndex(const std::string s) {

	}*/

	void Analyser::backLevel(int32_t level) {
		_current_level = _index_functions[level].outern;
	}

	//bool Analyser::isConstant(const std::string& s) {
	//	return _consts.find(s) != _consts.end();
	//}
	bool Analyser::numInRange(std::string s,int32_t& out,TokenType tokentype) {
		if (tokentype == TokenType::UNSIGNED_INTEGER) {
			std::string temp = s;
			int32_t i = 0;
			for (auto c : s) {
				if (c == '0') {
					//temp = temp.substr(1);
					i++;
				}
				else if (c != '0') {
					break;
				}
			}
			temp = temp.substr(i);
			if (temp.length() == 0) {
				temp = '0';
			}
			//std::cout << out << std::endl;
			//std::cout << out.length() << std::endl;
			//std::cout << out.compare("2147483647") << std::endl;
			if ((temp.length() > 10) || (temp.length() == 10 && temp.compare("2147483647") > 0)) {
				return false;
			}
			std::stringstream ss;
			ss << temp;
			ss >> out;
			return true;
		}
		else if(tokentype == TokenType::HEX_INTEGER) {
			//7fffffff
			//std::cout << out << std::endl;
			//std::cout << out.length() << std::endl;
			//std::cout << out.compare("2147483647") << std::endl;
			/*int32_t i = 2;
			for (i = 2; i < s.length(); i++) {
				if (s[i] != '0') {
					break;
				}
			}
			i = i;*/

			/*if ((s.length() - i > 8) || ((s.length()-i) == 8 && s.compare("0xffffffff") > 0)) {
				return false;
			}*/
			//s=s.substr(i);
			//std::cout << s << std::endl;
			std::stringstream ss;
			//ss >> std::hex;
			
			ss << std::hex;
			ss << s;
			uint32_t temp;
			ss >> temp;
			out = (int)temp;
			//ss >> out;
			//out= std::stoi(s, nullptr, 16);
			//fuct it i can't
			return true;

		}

		return false;
	}









}