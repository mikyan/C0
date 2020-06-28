#include "tokenizer/tokenizer.h"

#include <cctype>
#include <sstream>

namespace miniplc0 {

	std::pair<std::optional<Token>, std::optional<CompilationError>> Tokenizer::NextToken() {
		if (!_initialized)
			readAll();
		if (_rdr.bad())
			return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrStreamError));
		if (isEOF())
			return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrEOF));
		auto p = nextToken();
		if (p.second.has_value())
			return std::make_pair(p.first, p.second);
		auto err = checkToken(p.first.value());
		if (err.has_value())
			return std::make_pair(p.first, err.value());
		return std::make_pair(p.first, std::optional<CompilationError>());
	}

	std::pair<std::vector<Token>, std::optional<CompilationError>> Tokenizer::AllTokens() {
		std::vector<Token> result;
		while (true) {
			auto p = NextToken();
			if (p.second.has_value()) {
				if (p.second.value().GetCode() == ErrorCode::ErrEOF)
					return std::make_pair(result, std::optional<CompilationError>());
				else
					return std::make_pair(std::vector<Token>(), p.second);
			}
			result.emplace_back(p.first.value());
		}
	}

	// 注意：这里的返回值中 Token 和 CompilationError 只能返回一个，不能同时返回。
	std::pair<std::optional<Token>, std::optional<CompilationError>> Tokenizer::nextToken() {
		// 用于存储已经读到的组成当前token字符
		std::stringstream ss;
		// 分析token的结果，作为此函数的返回值
		std::pair<std::optional<Token>, std::optional<CompilationError>> result;
		// <行号，列号>，表示当前token的第一个字符在源代码中的位置
		std::pair<int64_t, int64_t> pos;
		// 记录当前自动机的状态，进入此函数时是初始状态
		DFAState current_state = DFAState::INITIAL_STATE;
		// 这是一个死循环，除非主动跳出
		// 每一次执行while内的代码，都可能导致状态的变更
		while (true) {
			// 读一个字符，请注意auto推导得出的类型是std::optional<char>
			// 这里其实有两种写法
			// 1. 每次循环前立即读入一个 char
			// 2. 只有在可能会转移的状态读入一个 char
			// 因为我们实现了 unread，为了省事我们选择第一种
			auto current_char = nextChar();
			// 针对当前的状态进行不同的操作
			switch (current_state) {

				// 初始状态
				// 这个 case 我们给出了核心逻辑，但是后面的 case 不用照搬。
			case INITIAL_STATE: {
				// 已经读到了文件尾
				if (!current_char.has_value())
					// 返回一个空的token，和编译错误ErrEOF：遇到了文件尾
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrEOF));

				// 获取读到的字符的值，注意auto推导出的类型是char
				auto ch = current_char.value();
				// 标记是否读到了不合法的字符，初始化为否
				auto invalid = false;

				// 使用了自己封装的判断字符类型的函数，定义于 tokenizer/utils.hpp
				// see https://en.cppreference.com/w/cpp/string/byte/isblank
				if (miniplc0::isspace(ch)) // 读到的字符是空白字符（空格、换行、制表符等）
					current_state = DFAState::INITIAL_STATE; // 保留当前状态为初始状态，此处直接break也是可以的
				else if (!miniplc0::isprint(ch)) { // control codes and backspace
					invalid = true;
					//std::cout << "get isprint，sad" << std::endl;
				}
				else if (ch == '0')
					current_state = DFAState::ZERO_STATE;
				else if (miniplc0::isdigit(ch)&& ch != '0') // 读到的字符是数字且不是0
					current_state = DFAState::UNSIGNED_INTEGER_STATE; // 切换到无符号整数的状态
				else if (miniplc0::isalpha(ch)) { // 读到的字符是英文字母
					current_state = DFAState::IDENTIFIER_STATE;
					//std::cout << ch << std::endl;
					//std::cout << "change stata read" + ch << std::endl;
				}// 切换到标识符的状态
				else {
					switch (ch) {
					case '=': // 如果读到的字符是`=`，则切换到等于号的状态
						current_state = DFAState::EQUAL_SIGN_STATE;
						break;
					case '-':
						current_state = DFAState::MINUS_SIGN_STATE;
						break;
						// 请填空：切换到减号的状态
					case '+':
						current_state = DFAState::PLUS_SIGN_STATE;
						break;
						// 请填空：切换到加号的状态
					case '*':
						current_state = DFAState::MULTIPLICATION_SIGN_STATE;
						break;
						// 请填空：切换状态
					case '/':
						current_state = DFAState::DIVISION_SIGN_STATE;
						break;
						// 请填空：切换状态

					///// 请填空：
					///// 对于其他的可接受字符
					///// 切换到对应的状态
					case '(':
						current_state = DFAState::LEFTBRACKET_STATE;
						break;
					case ')':
						current_state = DFAState::RIGHTBRACKET_STATE;
						break;
					case ';':
						current_state = DFAState::SEMICOLON_STATE;
						break;
					
					case ',':
						current_state = DFAState::COMMA_STATE;
						break;
					case '{':
						current_state = DFAState::LEFT_BRACES_STATE;
						break;
					case '}':
						current_state = DFAState::RIGHT_BRACES_STATE;
						break;
					case '<':
						current_state = DFAState::LESS_SIGN_STATE;
						break;
					case '>':
						current_state = DFAState::GREATER_SIGN_STATE;
						break;
					case '!':
						current_state = DFAState::NOT_EQUAL_STATE;
						break;

					// 不接受的字符导致的不合法的状态
					default:
						invalid = true;
						break;
					}
				}
				// 如果读到的字符导致了状态的转移，说明它是一个token的第一个字符
				if (current_state != DFAState::INITIAL_STATE)
					pos = previousPos(); // 记录该字符的的位置为token的开始位置
				// 读到了不合法的字符
				if (invalid) {
					// 回退这个字符
					unreadLast();
					// 返回编译错误：非法的输入
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(previousPos(), ErrorCode::ErrInvalidInput));
				}
				// 如果读到的字符导致了状态的转移，说明它是一个token的第一个字符
				if (current_state != DFAState::INITIAL_STATE) // ignore white spaces
					ss << ch; // 存储读到的字符
				break;
			 }

			case ZERO_STATE: {
				if (!current_char.has_value()) {
					std::string s = ss.str();
					if (isNum(s)) {
						return std::make_pair(std::make_optional<Token>(TokenType::UNSIGNED_INTEGER, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else {
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidInput));
					}
				}

				auto ch = current_char.value();

				if (ch == 'x' || ch == 'X') {
					ss << ch;
					current_state = HEX_INTEGER_STATE;
				}
				else if (miniplc0::isalpha(ch) || miniplc0::isdigit(ch)) {
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidInput));
				}
				else{
					unreadLast();
					std::string s = ss.str();
					if (isNum(s)) {
						return std::make_pair(std::make_optional<Token>(TokenType::UNSIGNED_INTEGER, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else {
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidInput));
					}
				}

				break;
			}

							  // 当前状态是无符号整数
			case UNSIGNED_INTEGER_STATE: {
				if (!current_char.has_value()){
					std::string s = ss.str();
					if (isNum(s)) {
						return std::make_pair(std::make_optional<Token>(TokenType::UNSIGNED_INTEGER, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else {
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidInput));
					}
				}

				auto ch = current_char.value();

				if (miniplc0::isdigit(ch)) {
					ss << ch;
				}
				else if (miniplc0::isalpha(ch)) {
					ss << ch;
					current_state = DFAState::IDENTIFIER_STATE;
				}
				else {
					unreadLast();
					std::string s = ss.str();
					if (isNum(s)) {
						return std::make_pair(std::make_optional<Token>(TokenType::UNSIGNED_INTEGER, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else {
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidInput));
					}
				}
					
				// 请填空：
				// 如果当前已经读到了文件尾，则解析已经读到的字符串为整数
				//     解析成功则返回无符号整数类型的token，否则返回编译错误
				// 如果读到的字符是数字，则存储读到的字符
				// 如果读到的是字母，则存储读到的字符，并切换状态到标识符
				// 如果读到的字符不是上述情况之一，则回退读到的字符，并解析已经读到的字符串为整数
				//     解析成功则返回无符号整数类型的token，否则返回编译错误
				break;
			}

			case HEX_INTEGER_STATE: {
				if (!current_char.has_value()) {
					std::string s = ss.str();
					if (isHexNum(s)) {
						return std::make_pair(std::make_optional<Token>(TokenType::HEX_INTEGER, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else {
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidInput));
					}
				}

				auto ch = current_char.value();

				if (isHexdigit(ch)) {
					ss << ch;
				}
				else if (miniplc0::isalpha(ch) && !isHexdigit(ch)) {
					ss << ch;
					current_state = DFAState::IDENTIFIER_STATE;
				}
				else {
					unreadLast();
					std::string s = ss.str();
					if (isHexNum(s)) {
						return std::make_pair(std::make_optional<Token>(TokenType::HEX_INTEGER, ss.str(), pos, currentPos()), std::optional<CompilationError>());
					}
					else {
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidInput));
					}
				}

				break;
			}
			case IDENTIFIER_STATE: {
			
				if (!current_char.has_value()) {
					std::string s = ss.str();
					if (isIdentifier(s)) {
						TokenType type;
						if(isReservedWord(s,type)){
							return std::make_pair(std::make_optional<Token>(type, ss.str(), pos, currentPos()), std::optional<CompilationError>());
						}else {
							return std::make_pair(std::make_optional<Token>(TokenType::IDENTIFIER, ss.str(), pos, currentPos()), std::optional<CompilationError>());
						}
					}
					else {
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidIdentifier));
					}
				}

				auto ch = current_char.value();

				if (miniplc0::isdigit(ch)|| miniplc0::isalpha(ch)) {

					ss << ch;
				}
				else {
					unreadLast();
					std::string s = ss.str();
					if (isIdentifier(s)) {
						TokenType type;
						if (isReservedWord(s, type)) {
							return std::make_pair(std::make_optional<Token>(type, ss.str(), pos, currentPos()), std::optional<CompilationError>());
						}
						else {
							return std::make_pair(std::make_optional<Token>(TokenType::IDENTIFIER, ss.str(), pos, currentPos()), std::optional<CompilationError>());
						}
					}
					else {
						return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidIdentifier));
					}
				}

				// 请填空：
				// 如果当前已经读到了文件尾，则解析已经读到的字符串
				//     如果解析结果是关键字，那么返回对应关键字的token，否则返回标识符的token
				// 如果读到的是字符或字母，则存储读到的字符
				// 如果读到的字符不是上述情况之一，则回退读到的字符，并解析已经读到的字符串
				//     如果解析结果是关键字，那么返回对应关键字的token，否则返回标识符的token
				break;
			}

			

								 // 如果当前状态是加号
			case PLUS_SIGN_STATE: {
				// 请思考这里为什么要回退，在其他地方会不会需要
				std::string sign = "+";
				unreadLast(); // Yes, we unread last char even if it's an EOF. 都放在INITIAL_STATE里处理
				return std::make_pair(std::make_optional<Token>(TokenType::PLUS_SIGN,sign, pos, currentPos()), std::optional<CompilationError>());
				
			}
								// 当前状态为减号的状态
			case MINUS_SIGN_STATE: {
				std::string sign = "-";
				unreadLast(); // Yes, we unread last char even if it's an EOF.都放在INITIAL_STATE里处理
				return std::make_pair(std::make_optional<Token>(TokenType::MINUS_SIGN, sign, pos, currentPos()), std::optional<CompilationError>());
				
			}
			case MULTIPLICATION_SIGN_STATE: {
				std::string sign = "*";
				unreadLast(); // Yes, we unread last char even if it's an EOF.都放在INITIAL_STATE里处理
				return std::make_pair(std::make_optional<Token>(TokenType::MULTIPLICATION_SIGN, sign, pos, currentPos()), std::optional<CompilationError>());
				
			}
			case DIVISION_SIGN_STATE: {
				std::string sign = "/";
				unreadLast(); // Yes, we unread last char even if it's an EOF.都放在INITIAL_STATE里处理
				return std::make_pair(std::make_optional<Token>(TokenType::DIVISION_SIGN, sign, pos, currentPos()), std::optional<CompilationError>());
				
			}
			case COMMA_STATE: {
				std::string sign = ",";
				unreadLast(); // Yes, we unread last char even if it's an EOF.都放在INITIAL_STATE里处理
				return std::make_pair(std::make_optional<Token>(TokenType::COMMA_STATE_SIGN, sign, pos, currentPos()), std::optional<CompilationError>());

			}
			case EQUAL_SIGN_STATE: {
				if (!current_char.has_value()) {
					return std::make_pair(std::make_optional<Token>(TokenType::EQUAL_SIGN, ss.str(), pos, currentPos()), std::optional<CompilationError>());
				}
				auto ch = current_char.value();
				if (ch == '=') {
					ss << ch;
					current_state = DFAState::EQUAL_EQUAL_STATE;
				}
				else {
					std::string sign = "=";
					unreadLast(); // Yes, we unread last char even if it's an EOF.都放在INITIAL_STATE里处理
					return std::make_pair(std::make_optional<Token>(TokenType::EQUAL_SIGN, sign, pos, currentPos()), std::optional<CompilationError>());
				}
				
				break;
			}
			case EQUAL_EQUAL_STATE: {
				std::string sign = "==";
				unreadLast(); // Yes, we unread last char even if it's an EOF.都放在INITIAL_STATE里处理
				return std::make_pair(std::make_optional<Token>(TokenType::EQUAL_EQUAL_SIGN, sign, pos, currentPos()), std::optional<CompilationError>());
			}
			case SEMICOLON_STATE: {
				std::string sign = ";";
				unreadLast(); // Yes, we unread last char even if it's an EOF.都放在INITIAL_STATE里处理
				return std::make_pair(std::make_optional<Token>(TokenType::SEMICOLON, sign, pos, currentPos()), std::optional<CompilationError>());
				
			}
			case LEFTBRACKET_STATE: {
				std::string sign = "(";
				unreadLast(); // Yes, we unread last char even if it's an EOF.都放在INITIAL_STATE里处理
				return std::make_pair(std::make_optional<Token>(TokenType::LEFT_BRACKET, sign, pos, currentPos()), std::optional<CompilationError>());
				
			}
			
			case RIGHTBRACKET_STATE: {
				std::string sign = ")";
				unreadLast(); // Yes, we unread last char even if it's an EOF.都放在INITIAL_STATE里处理
				return std::make_pair(std::make_optional<Token>(TokenType::RIGHT_BRACKET, sign, pos, currentPos()), std::optional<CompilationError>());
				 
			}
			case RIGHT_BRACES_STATE: {
				std::string sign = "}";
				unreadLast(); // Yes, we unread last char even if it's an EOF.都放在INITIAL_STATE里处理
				return std::make_pair(std::make_optional<Token>(TokenType::RIGHT_BRACES_SIGN, sign, pos, currentPos()), std::optional<CompilationError>());
			}
			case LEFT_BRACES_STATE:{
				std::string sign = "{";
				unreadLast(); // Yes, we unread last char even if it's an EOF.都放在INITIAL_STATE里处理
				return std::make_pair(std::make_optional<Token>(TokenType::LEFT_BRACES_SIGN, sign, pos, currentPos()), std::optional<CompilationError>());
			}
			case NOT_EQUAL_STATE: {
				if (!current_char.has_value()) {
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidIdentifier));
				}
				auto ch = current_char.value();
				if (ch == '=') {
					ss << ch;
					return std::make_pair(std::make_optional<Token>(TokenType::NOT_EQUAL_SIGN, ss.str(), pos, currentPos()), std::optional<CompilationError>());
				}
				else {
					unreadLast(); // Yes, we unread last char even if it's an EOF.都放在INITIAL_STATE里处理
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidIdentifier));
				}
				break;
			}
			case LESS_SIGN_STATE: {
				if (!current_char.has_value()) {
					return std::make_pair(std::make_optional<Token>(TokenType::LESS_SIGN, ss.str(), pos, currentPos()), std::optional<CompilationError>());
				}

				auto ch = current_char.value();
				if (ch == '=') {
					ss << ch;
					current_state = LESS_EQUAL_STATE;
				}
				else {
					std::string sign = "<";
					unreadLast(); // Yes, we unread last char even if it's an EOF.都放在INITIAL_STATE里处理
					return std::make_pair(std::make_optional<Token>(TokenType::LESS_SIGN, sign, pos, currentPos()), std::optional<CompilationError>());
				}
				break;
			}
			case LESS_EQUAL_STATE:{
				std::string sign = "<=";
				unreadLast(); // Yes, we unread last char even if it's an EOF.都放在INITIAL_STATE里处理
				return std::make_pair(std::make_optional<Token>(TokenType::LESS_EQUAL_SIGN, sign, pos, currentPos()), std::optional<CompilationError>());
			}

			case GREATER_SIGN_STATE: {
				if (!current_char.has_value()) {
					return std::make_pair(std::make_optional<Token>(TokenType::GREATER_SIGN, ss.str(), pos, currentPos()), std::optional<CompilationError>());
				}

				auto ch = current_char.value();
				if (ch == '=') {
					ss << ch;
					current_state = GREATER_EQUAL_STATE;
				}
				else {
					std::string sign = ">";
					unreadLast(); // Yes, we unread last char even if it's an EOF.都放在INITIAL_STATE里处理
					return std::make_pair(std::make_optional<Token>(TokenType::GREATER_SIGN, sign, pos, currentPos()), std::optional<CompilationError>());
				}
				break;
			}
			case GREATER_EQUAL_STATE: {
				std::string sign = ">=";
				unreadLast(); // Yes, we unread last char even if it's an EOF.都放在INITIAL_STATE里处理
				return std::make_pair(std::make_optional<Token>(TokenType::GREATER_EQUAL_SIGN, sign, pos, currentPos()), std::optional<CompilationError>());
			}

			
								 // 请填空：
								 // 对于其他的合法状态，进行合适的操作
								 // 比如进行解析、返回token、返回编译错误

								 // 预料之外的状态，如果执行到了这里，说明程序异常
			default:
				DieAndPrint("unhandled state.");
				break;
			}
		}
		// 预料之外的状态，如果执行到了这里，说明程序异常
		return std::make_pair(std::optional<Token>(), std::optional<CompilationError>());
	}

	std::optional<CompilationError> Tokenizer::checkToken(const Token& t) {
		switch (t.GetType()) {
		case IDENTIFIER: {
			auto val = t.GetValueString();
			if (miniplc0::isdigit(val[0]))
				return std::make_optional<CompilationError>(t.GetStartPos().first, t.GetStartPos().second, ErrorCode::ErrInvalidIdentifier);
			break;
		}
		default:
			break;
		}
		return {};
	}

	void Tokenizer::readAll() {
		if (_initialized)
			return;
		for (std::string tp; std::getline(_rdr, tp);)
			_lines_buffer.emplace_back(std::move(tp + "\n"));
		//std::cout << _lines_buffer[0];
		//std::cout << _lines_buffer[1];
		//std::cout << _lines_buffer[2];
		_initialized = true;
		_ptr = std::make_pair<int64_t, int64_t>(0, 0);
		return;
	}

	// Note: We allow this function to return a postion which is out of bound according to the design like std::vector::end().
	std::pair<uint64_t, uint64_t> Tokenizer::nextPos() {
		if (_ptr.first >= _lines_buffer.size())
			DieAndPrint("advance after EOF");
		if (_ptr.second == _lines_buffer[_ptr.first].size() - 1)
			return std::make_pair(_ptr.first + 1, 0);
		else
			return std::make_pair(_ptr.first, _ptr.second + 1);
	}

	std::pair<uint64_t, uint64_t> Tokenizer::currentPos() {
		return _ptr;
	}

	std::pair<uint64_t, uint64_t> Tokenizer::previousPos() {
		if (_ptr.first == 0 && _ptr.second == 0)
			DieAndPrint("previous position from beginning");
		if (_ptr.second == 0)
			return std::make_pair(_ptr.first - 1, _lines_buffer[_ptr.first - 1].size() - 1);
		else
			return std::make_pair(_ptr.first, _ptr.second - 1);
	}

	std::optional<char> Tokenizer::nextChar() {
		//std::cout << "usenextchar" << std::endl;
		if (isEOF()) {
			//std::cout << "getROF" << std::endl;
			return {};
		}// EOF
		auto result = _lines_buffer[_ptr.first][_ptr.second];
		//std::cout << result << std::endl ;
		_ptr = nextPos();
		return result;
	}

	bool Tokenizer::isEOF() {
		return _ptr.first >= _lines_buffer.size();
	}

	// Note: Is it evil to unread a buffer?
	void Tokenizer::unreadLast() {
		_ptr = previousPos();
	}

	bool Tokenizer::isNum(std::string s) {
		for (int i = 0; i < s.length(); i++) {
			if (!miniplc0::isdigit(s[i])) {
				return false;
			}
		}
		return true;
	}

	bool Tokenizer::isIdentifier(std::string s) {

		
		if (std::isdigit(s[0]))
			return false;
		
		return true;


	}

	bool Tokenizer::isHexdigit(char c) {
		
		if (c >= '0' && c <= '9')
			return true;
		if (c >= 'a' && c <= 'f')
			return true;
		if (c >= 'A' && c <= 'F') {
			return true;
		}

		return false;


	}

	bool Tokenizer::isHexNum(std::string s) {
		//std::cout << s << std::endl;
		//std::cout << s[0] << std::endl;
		if (s[0] != '0') {
			return false;
		}
		//std::cout << s[1] << std::endl;
		if (s[1] != 'x' && s[1] != 'X') {
			return false;
		}
		//std::cout << s << std::endl;
		for (int i = 2; i < s.length(); i++) {
			//std::cout << s[i] << std::endl;
			if (!isHexdigit(s[i])) {
				return false;
			}
		}
		

		return true;
	}

	bool Tokenizer::isReservedWord(std::string s,TokenType& type) {
		if (s == "const") {
			type = TokenType::CONST;
			return true;
		}

		if (s == "void") {
			type = TokenType::VOID;
			return true;
		}
		if (s == "int") {
			type = TokenType::INT;
			return true;
		}
		if (s == "char") {
			type = TokenType::CHAR;
			return true;
		}
		if (s == "double") {
			type = TokenType::DOUBLE;
			return true;
		}
		if (s == "struct") {
			type = TokenType::STRUCT;
			return true;
		}
		if(s=="if"){
			type = TokenType::IF;
			return true;
		}
		if (s == "else") {
			type = TokenType::ELSE;
			return true;
		}
		if (s == "switch") {
			type = TokenType::SWITCH;
			return true;
		}
		if (s == "case") {
			type = TokenType::CASE;
			return true;
		}
		if (s == "default") {
			type = TokenType::DEFAULT;
			return true;
		}
		if (s == "while") {
			type = TokenType::WHILE;
			return true;
		}
		if (s == "for") {
			type = TokenType::FOR;
			return true;
		}
		if (s == "do") {
			type = TokenType::DO;
			return true;
		}
		if (s == "return") {
			type = TokenType::RETURN;
			return true;
		}
		if (s == "break") {
			type = TokenType::BREAK;
			return true;
		}
		if (s == "continue") {
			type = TokenType::CONTINUE;
			return true;
		}
		if (s == "print") {
			type = TokenType::PRINT;
			return true;
		}
		if (s == "scan") {
			type = TokenType::SCAN;
			return true;
		}
		return false;
	}
	/*'const'
		| 'void' | 'int' | 'char' | 'double'
		| 'struct'
		| 'if' | 'else'
		| 'switch' | 'case' | 'default'
		| 'while' | 'for' | 'do'
		| 'return' | 'break' | 'continue'
		| 'print' | 'scan'*/

}