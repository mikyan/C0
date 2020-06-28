#include "fmt/core.h"
#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"

namespace fmt {
	template<>
	struct formatter<miniplc0::ErrorCode> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::ErrorCode &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
			case miniplc0::ErrNoError:
				name = "No error.";
				break;
			case miniplc0::ErrStreamError:
				name = "Stream error.";
				break;
			case miniplc0::ErrEOF:
				name = "EOF";
				break;
			case miniplc0::ErrInvalidInput:
				name = "The input is invalid.";
				break;
			case miniplc0::ErrInvalidIdentifier:
				name = "Identifier is invalid";
				break;
			case miniplc0::ErrIntegerOverflow:
				name = "The integer is too big(int64_t).";
				break;
			case miniplc0::ErrNoBegin:
				name = "The program should start with 'begin'.";
				break;
			case miniplc0::ErrNoEnd:
				name = "The program should end with 'end'.";
				break;
			case miniplc0::ErrNeedIdentifier:
				name = "Need an identifier here.";
				break;
			case miniplc0::ErrConstantNeedValue:
				name = "The constant need a value to initialize.";
				break;
			case miniplc0::ErrNoSemicolon:
				name = "Zai? Wei shen me bu xie fen hao.";
				break;
			case miniplc0::ErrInvalidVariableDeclaration:
				name = "The declaration is invalid.";
				break;
			case miniplc0::ErrIncompleteExpression:
				name = "The expression is incomplete.";
				break;
			case miniplc0::ErrNotDeclared:
				name = "The variable or constant must be declared before being used.";
				break;
			case miniplc0::ErrAssignToConstant:
				name = "Trying to assign value to a constant.";
				break;
			case miniplc0::ErrDuplicateDeclaration:
				name = "The variable or constant has been declared.";
				break;
			case miniplc0::ErrNotInitialized:
				name = "The variable has not been initialized.";
				break;
			case miniplc0::ErrInvalidAssignment:
				name = "The assignment statement is invalid.";
				break;
			case miniplc0::ErrInvalidPrint:
				name = "The output statement is invalid.";
				break;

				//ErrNeedType,
				//	ErrVoidType,
				//	ErrNeedEqualSign,
				//	ErrNeedLeftBracket,
				//	ErrNeedRightBracket,
				//	ErrNeedRightBraces,
				//	ErrNeedAssignmentOperator,
				//	ErrNeedExpression,
			case miniplc0::ErrNeedType:
				name = "The const need type";
				break;
			case miniplc0::ErrVoidType:
				name = "can not be void";
				break;
			case miniplc0::ErrNeedEqualSign:
				name = "The assignmentstate need equalsign";
				break;
			case miniplc0::ErrNeedLeftBracket:
				name = "Need left bracket";
				break;
			case miniplc0::ErrNeedRightBracket:
				name = "ErrNeedRightBracket";
				break;
			case miniplc0::ErrNeedRightBraces:
				name = "ErrNeedRightBraces";
				break;
			case miniplc0::ErrNeedAssignmentOperator:
				name = "ErrNeedAssignmentOperator";
				break;
			case miniplc0::ErrNeedExpression:
				name = "ErrNeedExpression";
				break;
			}
			return format_to(ctx.out(), name);
		}
	};

	template<>
	struct formatter<miniplc0::CompilationError> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::CompilationError &p, FormatContext &ctx) {
			return format_to(ctx.out(), "Line: {} Column: {} Error: {}", p.GetPos().first, p.GetPos().second, p.GetCode());
		}
	};
}

namespace fmt {
	template<>
	struct formatter<miniplc0::Token> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::Token &p, FormatContext &ctx) {
			return format_to(ctx.out(),
				"Line: {} Column: {} Type: {} Value: {}",
				p.GetStartPos().first, p.GetStartPos().second, p.GetType(), p.GetValueString());
		}
	};

	template<>
	struct formatter<miniplc0::TokenType> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const miniplc0::TokenType &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
			case miniplc0::NULL_TOKEN:
				name = "NullToken";
				break;
			case miniplc0::IDENTIFIER:
				name = "Identifier";
				break;
			case miniplc0::HEX_INTEGER:
				name = "hexadeciaml";
				break;
			case miniplc0::VOID:
				name = "VOID";
				break;
			case miniplc0::INT:
				name = "int";
				break;
			case miniplc0::CHAR:
				name = "char";
				break;
			case miniplc0::DOUBLE:
				name = "double";
				break;
			case miniplc0::CONST:
				name = "Const";
				break;
			case miniplc0::SCAN:
				name = "scan";
				break;
			case miniplc0::PRINT:
				name = "Print";
				break;
			case miniplc0::STRUCT:
				name = "struct";
				break;
			case miniplc0::IF:
				name = "if";
				break;
			case miniplc0::ELSE:
				name = "else";
				break;
			case miniplc0::SWITCH:
				name = "switch";
				break;
			case miniplc0::CASE:
				name = "case";
				break;
			case miniplc0::DEFAULT:
				name = "default";
				break;
			case miniplc0::WHILE:
				name = "while";
				break;
			case miniplc0::FOR:
				name = "for";
				break;
			case miniplc0::DO:
				name = "do";
				break;
			case miniplc0::RETURN:
				name = "return";
				break;
			case miniplc0::BREAK:
				name = "break";
				break;
			case miniplc0::CONTINUE:
				name = "continue";
				break;
			case miniplc0::PLUS_SIGN:
				name = "PlusSign";
				break;
			case miniplc0::MINUS_SIGN:
				name = "MinusSign";
				break;
			case miniplc0::MULTIPLICATION_SIGN:
				name = "MultiplicationSign";
				break;
			case miniplc0::DIVISION_SIGN:
				name = "DivisionSign";
				break;
			case miniplc0::EQUAL_SIGN:
				name = "EqualSign";
				break;
			case miniplc0::LESS_SIGN:
				name = "Less";
				break;
			case miniplc0::LESS_EQUAL_SIGN:
				name = "Lessequal";
				break;
			case miniplc0::GREATER_SIGN:
				name = "Greater";
				break;
			case miniplc0::GREATER_EQUAL_SIGN:
				name = "Greaterequal";
				break;
			case miniplc0::EQUAL_EQUAL_SIGN:
				name = "Equal_Equal";
				break;
			case miniplc0::NOT_EQUAL_SIGN:
				name = "Notequal";
				break;
			case miniplc0::SEMICOLON:
				name = "Semicolon";
				break;
			case miniplc0::LEFT_BRACKET:
				name = "LeftBracket";
				break;
			case miniplc0::RIGHT_BRACKET:
				name = "RightBracket";
				break;
			case miniplc0::LEFT_BRACES_SIGN:
				name = "LeftBracesSign";
				break;
			case miniplc0::RIGHT_BRACES_SIGN:
				name = "RightBracesSign";
				break;
			case miniplc0::COMMA_STATE_SIGN:
				name = "Comma";
				break;
			}
			return format_to(ctx.out(), name);
		}
	};
}

