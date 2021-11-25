#pragma once

enum Registers16Bit {
	AX, CX, DX, BX, SP, BP, SI,DI
};

enum Registers8Bit {
	AH, AL, CH, CL, DH, DL, BH, BL, SPL, BPL, SiL, DiL
};

enum SegmentRegisters {
	DS,ES,SS,CS
};


class RegistersManagement
{
private:
	unsigned char *registers;
	unsigned short *segmentRegisters;
	unsigned int flags;
public:
	RegistersManagement();
	void setFlag(int flag);
	void clearFlag(int flag);
	int getFlag(int flag);
	void set_i16(int regIndex, unsigned short v);
	unsigned short get_i16(int regIndex);
	unsigned short get_i16(SegmentRegisters name);
	unsigned short get_i16(Registers16Bit name);
	unsigned short get_i8(Registers8Bit name);
	void set(SegmentRegisters name, unsigned short v);
	void set(Registers8Bit name, unsigned char v);
	void set_i8L(int regIndex, unsigned char v);
	void set_i8H(int regIndex, unsigned char v);
	void set_i16(Registers16Bit name);
	~RegistersManagement();
};

