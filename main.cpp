#include "argparse.hpp"
#include "fmt/core.h"
#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"
#include "fmts.hpp"

#include <iostream>
#include <fstream>

typedef int8_t  i1;
typedef int16_t i2;
typedef int32_t i4;
typedef int64_t i8;

// u2,u3,u4的内容，以大端序（big-endian）写入文件
typedef uint8_t  u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;

u4 swapInt32(u4 value)
{
	return ((value & 0x000000FF) << 24) |
		((value & 0x0000FF00) << 8) |
		((value & 0x00FF0000) >> 8) |
		((value & 0xFF000000) >> 24);
}

u2 swapInt16(u2 value) {
	return ((value & 0x00FF) << 8) |
		((value & 0xFF00) >> 8);
}


std::string typetostr(miniplc0::ConstType type) {
	std::string s;
	switch (type) {
	case miniplc0::ConstType::STRING_Const_Type: {
		s = "S";
		break;
	}
	case miniplc0::ConstType::INT_Const_Type: {
		s = "I";
		break;
	}
	case miniplc0::ConstType::DOUBLE_Const_Type: {
		s = "D";
		break;
	}

	}
	return s;

}

void printcode(std::ostream& output, miniplc0::Instruction code) {
	std::string name;
	int x1 = 0, x2 = 0;

	switch (code.GetOperation())
	{
	case miniplc0::Operation::NOP:
		name = "nop";
		break;
	case miniplc0::Operation::BIPUSH:
		x1 = 1; x2 = 0;
		name = "bipush";
		break;
	case miniplc0::Operation::IPUSH:
		x1 = 4;
		name = "ipush";
		break;
	case miniplc0::Operation::POP:
		name = "pop";
		break;
	case miniplc0::Operation::POP2:
		name = "pop2";
		break;
	case miniplc0::Operation::POPN:
		x1 = 4;
		name = "popn";
		break;
	case miniplc0::Operation::DUP:
		name = "dup";
		break;
	case miniplc0::Operation::DUP2:
		name = "dup2";
		break;
	case miniplc0::Operation::LOADC:
		x1 = 2;
		name = "loadc";
		break;
	case miniplc0::Operation::LOADA:
		x1 = 2; x2 = 4;
		
		name = "loada";
		break;
	case miniplc0::Operation::NEW:
		name = "new";
		break;
	case miniplc0::Operation::SNEW:
		x1 = 4;
		name = "snew";
		break;
	case miniplc0::Operation::ILOAD:
		name = "iload";
		break;
	case miniplc0::Operation::DLOAD:
		name = "dload";
		break;
	case miniplc0::Operation::ALOAD:
		name = "aload";
		break;
	case miniplc0::Operation::IALOAD:
		name = "iaload";
		break;
	case miniplc0::Operation::DALOAD:
		name = "daload";
		break;
	case miniplc0::Operation::AALOAD:
		name = "aaload";
		break;
	case miniplc0::Operation::ISTORE:
		name = "istore";
		break;
	case miniplc0::Operation::DSTORE:
		name = "dstore";
		break;
	case miniplc0::Operation::ASTORE:
		name = "astore";
		break;
	case miniplc0::Operation::IASTORE:
		name = "iastore";
		break;
	case miniplc0::Operation::DASTORE:
		name = "dastore";
		break;
	case miniplc0::Operation::AASTORE:
		name = "aastore";
		break;
	case miniplc0::Operation::IADD:
		name = "iadd";
		break;
	case miniplc0::Operation::DADD:
		name = "dadd";
		break;
	case miniplc0::Operation::ISUB:
		name = "isub";
		break;
	case miniplc0::Operation::DSUB:
		name = "dsub";
		break;
	case miniplc0::Operation::IMUL:
		name = "imul";
		break;
	case miniplc0::Operation::DMUL:
		name = "dmul";
		break;
	case miniplc0::Operation::IDIV:
		name = "idiv";
		break;
	case miniplc0::Operation::DDIV:
		name = "ddiv";
		break;
	case miniplc0::Operation::INEG:
		name = "ineg";
		break;
	case miniplc0::Operation::DNEG:
		name = "dneg";
		break;
	case miniplc0::Operation::ICMP:
		name = "icmp";
		break;
	case miniplc0::Operation::DCMP:
		name = "dcmp";
		break;
	case miniplc0::Operation::I2D:
		name = "i2d";
		break;
	case miniplc0::Operation::D2I:
		name = "d2i";
		break;
	case miniplc0::Operation::I2C:
		name = "i2c";
		break;
	case miniplc0::Operation::JMP:
		x1 = 2;
		name = "jmp";
		break;
	case miniplc0::Operation::JE:
		x1 = 2;
		name = "je";
		break;
	case miniplc0::Operation::JNE:
		x1 = 2;
		name = "jne";
		break;
	case miniplc0::Operation::JL:
		x1 = 2;
		name = "jl";
		break;
	case miniplc0::Operation::JGE:
		x1 = 2;
		name = "jge";
		break;
	case miniplc0::Operation::JG:
		x1 = 2;
		name = "jg";
		break;
	case miniplc0::Operation::JLE:
		x1 = 2;
		name = "jle";
		break;
	case miniplc0::Operation::CALL:
		x1 = 2;
		name = "call";
		break;
	case miniplc0::Operation::RET:
		name = "ret";
		break;
	case miniplc0::Operation::IRET:
		name = "iret";
		break;
	case miniplc0::Operation::DRET:
		name = "dret";
		break;
	case miniplc0::Operation::ARET:
		name = "aret";
		break;
	case miniplc0::Operation::IPRINT:
		name = "iprint";
		break;
	case miniplc0::Operation::DPRINT:
		name = "dprint";
		break;
	case miniplc0::Operation::CPRINT:
		name = "cprint";
		break;
	case miniplc0::Operation::SPRINT:
		name = "sprint";
		break;
	case miniplc0::Operation::PRINTL:
		name = "printl";
		break;
	case miniplc0::Operation::ISCAN:
		name = "iscan";
		break;
	case miniplc0::Operation::DSCAN:
		name = "dscan";
		break;
	case miniplc0::Operation::CSCAN:
		name = "csan";
		break;

	default:
		name = "nullerr";
	}
	if (x1 == 0 && x2 == 0) {
		output << name << std::endl;
	}
	else if (x1 != 0 && x2 == 0) {
		output << name << " " << code.GetX() << " " << std::endl;
	}
	else output << name << " " << code.GetX() << "," << code.GetY() << std::endl;
	return;
}



void printcodebinary(std::ostream& output, miniplc0::Instruction code) {
	std::string name;
	int x1 = 0, x2 = 0;
	switch (code.GetOperation())
	{
	case miniplc0::Operation::NOP:
		name = "nop";
		break;
	case miniplc0::Operation::BIPUSH:
		x1 = 1; x2 = 0;
		name = "bipush";
		break;
	case miniplc0::Operation::IPUSH:
		x1 = 4;
		name = "ipush";
		break;
	case miniplc0::Operation::POP:
		name = "pop";
		break;
	case miniplc0::Operation::POP2:
		name = "pop2";
		break;
	case miniplc0::Operation::POPN:
		x1 = 4;
		name = "popn";
		break;
	case miniplc0::Operation::DUP:
		name = "dup";
		break;
	case miniplc0::Operation::DUP2:
		name = "dup2";
		break;
	case miniplc0::Operation::LOADC:
		x1 = 2;
		name = "loadc";
		break;
	case miniplc0::Operation::LOADA:
		x1 = 2; x2 = 4;
		name = "loada";
		break;
	case miniplc0::Operation::NEW:
		name = "new";
		break;
	case miniplc0::Operation::SNEW:
		x1 = 4;
		name = "snew";
		break;
	case miniplc0::Operation::ILOAD:
		name = "iload";
		break;
	case miniplc0::Operation::DLOAD:
		name = "dload";
		break;
	case miniplc0::Operation::ALOAD:
		name = "aload";
		break;
	case miniplc0::Operation::IALOAD:
		name = "iaload";
		break;
	case miniplc0::Operation::DALOAD:
		name = "daload";
		break;
	case miniplc0::Operation::AALOAD:
		name = "aaload";
		break;
	case miniplc0::Operation::ISTORE:
		name = "istore";
		break;
	case miniplc0::Operation::DSTORE:
		name = "dstore";
		break;
	case miniplc0::Operation::ASTORE:
		name = "astore";
		break;
	case miniplc0::Operation::IASTORE:
		name = "iastore";
		break;
	case miniplc0::Operation::DASTORE:
		name = "dastore";
		break;
	case miniplc0::Operation::AASTORE:
		name = "aastore";
		break;
	case miniplc0::Operation::IADD:
		name = "iadd";
		break;
	case miniplc0::Operation::DADD:
		name = "dadd";
		break;
	case miniplc0::Operation::ISUB:
		name = "isub";
		break;
	case miniplc0::Operation::DSUB:
		name = "dsub";
		break;
	case miniplc0::Operation::IMUL:
		name = "imul";
		break;
	case miniplc0::Operation::DMUL:
		name = "dmul";
		break;
	case miniplc0::Operation::IDIV:
		name = "idiv";
		break;
	case miniplc0::Operation::DDIV:
		name = "ddiv";
		break;
	case miniplc0::Operation::INEG:
		name = "ineg";
		break;
	case miniplc0::Operation::DNEG:
		name = "dneg";
		break;
	case miniplc0::Operation::ICMP:
		name = "icmp";
		break;
	case miniplc0::Operation::DCMP:
		name = "dcmp";
		break;
	case miniplc0::Operation::I2D:
		name = "i2d";
		break;
	case miniplc0::Operation::D2I:
		name = "d2i";
		break;
	case miniplc0::Operation::I2C:
		name = "i2c";
		break;
	case miniplc0::Operation::JMP:
		x1 = 2;
		name = "jmp";
		break;
	case miniplc0::Operation::JE:
		x1 = 2;
		name = "je";
		break;
	case miniplc0::Operation::JNE:
		x1 = 2;
		name = "jne";
		break;
	case miniplc0::Operation::JL:
		x1 = 2;
		name = "jl";
		break;
	case miniplc0::Operation::JGE:
		x1 = 2;
		name = "jge";
		break;
	case miniplc0::Operation::JG:
		x1 = 2;
		name = "jg";
		break;
	case miniplc0::Operation::JLE:
		x1 = 2;
		name = "jle";
		break;
	case miniplc0::Operation::CALL:
		x1 = 2;
		name = "call";
		break;
	case miniplc0::Operation::RET:
		name = "ret";
		break;
	case miniplc0::Operation::IRET:
		name = "iret";
		break;
	case miniplc0::Operation::DRET:
		name = "dret";
		break;
	case miniplc0::Operation::ARET:
		name = "aret";
		break;
	case miniplc0::Operation::IPRINT:
		name = "iprint";
		break;
	case miniplc0::Operation::DPRINT:
		name = "dprint";
		break;
	case miniplc0::Operation::CPRINT:
		name = "cprint";
		break;
	case miniplc0::Operation::SPRINT:
		name = "sprint";
		break;
	case miniplc0::Operation::PRINTL:
		name = "printl";
		break;
	case miniplc0::Operation::ISCAN:
		name = "iscan";
		break;
	case miniplc0::Operation::DSCAN:
		name = "dscan";
		break;
	case miniplc0::Operation::CSCAN:
		name = "csan";
		break;

	default:
		name = "nullerr";

	}
	auto x = code.GetOperation();
	output.write((char*)&x, sizeof(u1));

	int op1, op2;
	op1 = code.GetX();
	op2 = code.GetY();
	if (x1 == 1 && x2 == 0) {
		u1 mid;
		mid = (u1)op1;
		output.write((char*)&mid, sizeof(u1));

	}
	if (x1 == 2 && x2 == 0) {
		u2 mid;
		mid = swapInt16((u2)op1);
		output.write((char*)&mid, sizeof(u2));

	}
	if (x1 == 4 && x2 == 0) {
		u4 mid;
		mid = swapInt32((u4)op1);
		output.write((char*)&mid, sizeof(u4));

	}
	if (x1 == 0) {
		return;
	}
	if (x1 == 2 && x2 == 4) {
		u2 mid1 = swapInt16((u2)op1);
		u4 mid2 = swapInt32((u4)op2);
		output.write((char*)&mid1, sizeof(u2));
		output.write((char*)&mid2, sizeof(u4));

	}


	return;
}



std::vector<miniplc0::Token> _tokenize(std::istream& input) {
	miniplc0::Tokenizer tkz(input);
	auto p = tkz.AllTokens();
	if (p.second.has_value()) {
		fmt::print(stderr, "Tokenization error: {}\n", p.second.value());
		exit(2);
	}
	return p.first;
}

void Tokenize(std::istream& input, std::ostream& output) {
	auto v = _tokenize(input);
	for (auto& it : v)
		output << fmt::format("{}\n", it);
	return;
}

void Analyse(std::istream& input, std::ostream& output){
	auto tks = _tokenize(input);
	//std::cout << "yaoqule" << std::endl;
	miniplc0::Analyser analyser(tks);
	
	auto p = analyser.Analyse();
	//std::cout << "wanshile" << std::endl;
	if (p.second.has_value()) {
		fmt::print(stderr, "Syntactic analysis error: {}\n", p.second.value());
		exit(2);
	}
	//std::cout << "shuchuconstantsla" << std::endl;
	output << ".constants:" << std::endl;
	unsigned long long int i;
	auto consttable = analyser.getConsttable();
	for (i = 0; i < consttable.size(); i++) {
		output << i << "\t" << typetostr(consttable[i].type) << "\t" << "\"" << consttable[i].value << "\"" << std::endl;
	}
	//std::cout << "shuchustartcodela" << std::endl;
	auto startcode = analyser.getInstructions()["0_begin"];
	output << ".start:" << std::endl;
	//std::cout << startcode.size() << std::endl;
	for (i = 0; i < startcode.size(); i++) {
		output << i << "\t";
		printcode(output, startcode[i]);
	}
	//std::cout << "shuchufunctionstablela" << std::endl;
	auto functiontable = analyser.getFunctionsTable();
	output << ".functions:" << std::endl;
	for (i = 1; i < functiontable.size(); i++) {
		output << i-1 << "\t" << i-1 << "\t" << functiontable[i].params_size << "\t" << "1" << std::endl;
		//std::cout << i-1 << "\t" << i-1 << "\t" << functiontable[i].params_size << "\t" << "1" << std::endl;
	}
	//std::cout << "shuchufunctionsla" << std::endl;
	auto functions = analyser.getInstructions();
	for (i = 1; i < functiontable.size(); i++) {
		output << ".F" << i-1 << ":" << std::endl;
		long long unsigned int j;
		//std::cout << functiontable[i].name << std::endl;
		for (j = 0; j < functions[functiontable[i].name].size(); j++) {
			output << j << "\t";
			printcode(output, functions[functiontable[i].name][j]);
		}
	}
	//std::cout << "shuchuwanla" << std::endl;
	//auto v = p.first;
	//for (auto& it : v)
	//	output << fmt::format("{}\n", it);
	return;
}


void Analyse2(std::istream& input, std::ostream& output) {
	auto tks = _tokenize(input);
	miniplc0::Analyser analyser(tks);
	auto p = analyser.Analyse();
	if (p.second.has_value()) {
		fmt::print(stderr, "Syntactic analysis error: {}\n", p.second.value());
		exit(2);
	}

	auto constantable = analyser.getConsttable();
	
	int magic = 0x43303a29;
	magic = swapInt32(magic);
	output.write((char*)&magic, sizeof(magic));
	u4 version = swapInt32(1);
	output.write((char*)&version, sizeof(version));
	u2 constants_count = swapInt16((u2)(constantable.size()));
	
	output.write((char*)&constants_count, sizeof(u2));
	unsigned long long int i;
	for (i = 0; i < constantable.size(); i++) {
		u1 type = 0;
		output.write((char*)&type, sizeof(u1));
		u2 length = swapInt16((u2)constantable[i].value.length());
		output.write((char*)&length, sizeof(u2));
		output << constantable[i].value;
	}
	auto startcode = analyser.getInstructions()["0_begin"];
	u2 startcode_count = swapInt16((u2)startcode.size());
	
	output.write((char*)&startcode_count, sizeof(u2));
	for (i = 0; i < startcode.size(); i++) {
		printcodebinary(output, startcode[i]);
	}

	auto programcodes = analyser.getInstructions();
	auto functiontable = analyser.getFunctionsTable();

	u2 program_num = swapInt16((u2)(functiontable.size()-1));
	
	output.write((char*)&program_num, sizeof(u2));
	
	for (i = 0; i < functiontable.size() - 1; i++) {
		u2 index = (u2)i;
		index = swapInt16(index);
		output.write((char*)&index, sizeof(u2));
		u2 params_size = (u2)functiontable[i+1].params_size;
		
		params_size = swapInt16(params_size);
		output.write((char*)&params_size, sizeof(u2));
		u2 level = 1;
		level = swapInt16(level);
		output.write((char*)&level, sizeof(u2));
		u2 instructcount = (u2)programcodes[functiontable[i+1].name].size();
		
		instructcount = swapInt16(instructcount);
		output.write((char*)&instructcount, sizeof(u2));
		unsigned long long int j;
		for (j = 0; j < programcodes[functiontable[i+1].name].size(); j++) {
			printcodebinary(output, programcodes[functiontable[i+1].name][j]);
		}

	}
}

int main(int argc, char** argv) {
	argparse::ArgumentParser program("cc0");
	program.add_argument("input")
		.help("speicify the file to be compiled.");
	program.add_argument("-c")
		.default_value(false)
		.implicit_value(true)
		.help("output binaryfile for the input file.");
	program.add_argument("-s")
		.default_value(false)
		.implicit_value(true)
		.help("output assemblefile for the input file.");
	program.add_argument("-o", "--output")
		.required()
		.default_value(std::string("-"))
		.help("specify the output file.");

	try {
		program.parse_args(argc, argv);
	}
	catch (const std::runtime_error& err) {
		fmt::print(stderr, "{}\n\n", err.what());
		program.print_help();
		exit(2);
	}

	auto input_file = program.get<std::string>("input");
	auto output_file = program.get<std::string>("--output");
	std::istream* input;
	std::ostream* output;
	std::ifstream inf;
	std::ofstream outf;
	
	if (program["-c"] == true && program["-s"] == true) {
		fmt::print(stderr, "You can only perform tokenization or syntactic analysis at one time.");
		exit(2);
	}

	if (program["-c"] == true) {
		if (input_file != "-") {
			inf.open(input_file, std::ios::in);
			if (!inf) {
				fmt::print(stderr, "Fail to open {} for reading.\n", input_file);
				exit(2);
			}
			input = &inf;
		}
		else
			input = &std::cin;

		if (output_file == "-") {
			output_file = "out";
		}
		if (input_file == output_file) {
			output_file = input_file + ".out";
		}
		outf.open(output_file, std::ios::out | std::ios::trunc| std::ios::binary);
		if (!outf) {
			fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
			exit(2);
		}
		output = &outf;

		Analyse2(*input, *output);
		



	}
	else if (program["-s"] == true) {
		if (input_file != "-") {
			inf.open(input_file, std::ios::in);
			if (!inf) {
				fmt::print(stderr, "Fail to open {} for reading.\n", input_file);
				exit(2);
			}
			input = &inf;
		}
		else
			input = &std::cin;
		if (output_file == "-") {
			output_file = "out";
		}
		if (input_file == output_file) {
			output_file = input_file + ".out";
		}
		outf.open(output_file, std::ios::out | std::ios::trunc);
		if (!outf) {
			fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
			exit(2);
		}
		output = &outf;

		Analyse(*input, *output);
	}
	else {
		fmt::print(stderr, "You must choose tokenization or syntactic analysis.");
		exit(2);
	}
	return 0;
}