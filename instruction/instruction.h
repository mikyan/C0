#pragma once

#include <cstdint>
#include <utility>

namespace miniplc0 {

	enum Operation {
		NOP=0x00,
		BIPUSH=0x01,
		IPUSH=0x02,
		POP = 0x04,
		POP2 = 0x05,
		POPN = 0x06,
		DUP =0x07,
		DUP2 = 0x08,
		LOADC =0x09 ,
		LOADA =0x0a,
		NEW = 0x0b,
		SNEW = 0x0c,
		ILOAD =0x10,
		DLOAD =0x11,
		ALOAD=0x12,
		IALOAD=0x18,
		DALOAD=0x19,
		AALOAD=0x1a,
		ISTORE=0x20,
		DSTORE=0x21,
		ASTORE=0x22,
		IASTORE=0x28,
		DASTORE=0x29,
		AASTORE=0x2a,
		IADD=0x30,
		DADD=0x31,
		ISUB=0x34,
		DSUB=0x35,
		IMUL=0x38,
		DMUL=0x39,
		IDIV=0x3c,
		DDIV=0x3d,
		INEG=0x40,
		DNEG=0x41,
		ICMP=0x44,
		DCMP=0x45,
		I2D=0x60,
		D2I=0x61,
		I2C=0x62,
		JMP=0x70,
		JE=0x71,
		JNE=0x72,
		JL=0x73,
		JGE=0x74,
		JG=0x75,
		JLE=0x76,
		CALL=0x80,
		RET=0x88,
		IRET=0x89,
		DRET=0x8a,
		ARET=0x8b,
		IPRINT=0xa0,
		DPRINT=0xa1,
		CPRINT=0xa2,
		SPRINT=0xa3,
		PRINTL=0xaf,
		ISCAN=0xb0,
		DSCAN=0xb1,
		CSCAN=0xb2,
	};
	
	class Instruction final {
	private:
		using int32_t = std::int32_t;
	public:
		friend void swap(Instruction& lhs, Instruction& rhs);
	public:
		Instruction(Operation opr, int32_t x) : _opr(opr), _x(x), _y(0){}
		Instruction(Operation opr, int32_t x,int32_t y) : _opr(opr), _x(x), _y(y) {}
		Instruction() : Instruction(Operation::NOP, 0,0){}
		Instruction(const Instruction& i) { _opr = i._opr; _x = i._x; _y = i._y; }
		Instruction(Instruction&& i) :Instruction() { swap(*this, i); }
		Instruction& operator=(Instruction i) { swap(*this, i); return *this; }
		bool operator==(const Instruction& i) const { return _opr == i._opr && _x == i._x &&_y ==i._y; }

		Operation GetOperation() const { return _opr; }
		int32_t GetX() const { return _x; }
		void SetX(int32_t x) { _x = x; }
		int32_t GetY() const { return _y; }
		void SetY(int32_t y) { _y = y; }
	private:
		Operation _opr;
		int32_t _x;
		int32_t _y;
	};

	inline void swap(Instruction& lhs, Instruction& rhs) {
		using std::swap;
		swap(lhs._opr, rhs._opr);
		swap(lhs._x, rhs._x);
	}
}