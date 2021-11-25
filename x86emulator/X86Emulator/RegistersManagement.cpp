#include "pch.h"
#include "RegistersManagement.h"
/*
• AX Accumulator eXtended
• AL Accumulator Low
• AH Accumulator High
• BX Base eXtended
• BL Base Low
• BH Base High
• CX Count eXtended
• CL Count Low
• CH Count High
• DX Data eXtended
• DL Data Low
• DH Data High
• SI Source Index
• DI Destination Index
• BP Base Pointer
• SP Stack Pointer

• CS Code Segment
• DS Data Segment
• SS Stack Segment
• ES Extra Segment

*/

RegistersManagement::RegistersManagement()
{
	registers = new unsigned char[8 * 2];
	segmentRegisters = new unsigned short[6];

	memset(registers, 0x00, 8 * 2);
	memset(segmentRegisters, 0x00, 6*sizeof(unsigned short));

	flags = 0;

	set(SegmentRegisters::CS, 0);
	set(SegmentRegisters::DS, 0);
	set(SegmentRegisters::ES, 0);
	set(SegmentRegisters::SS, 0);


}

void RegistersManagement::setFlag(int flag) {
	flags |= (flag);
}
void RegistersManagement::clearFlag(int flag) {
	flags &= ~(flag);
}
int RegistersManagement::getFlag(int flag) {
	return flags & flag;
}

void RegistersManagement::set_i16(int regIndex, unsigned short v)
{
	memcpy(registers + (regIndex * 2), &v, 2);
	//registers[regIndex] = v;
}

unsigned short RegistersManagement::get_i16(int regIndex) {
	return  *(unsigned short*)(registers + regIndex * 2);
}

unsigned short RegistersManagement::get_i16(SegmentRegisters name) {
	return  segmentRegisters[(int)name];
}

unsigned short RegistersManagement::get_i16(Registers16Bit name) {
	return  *(unsigned short*)(registers + (int)name * 2);
}

unsigned short RegistersManagement::get_i8(Registers8Bit name) {
	return *(byte*)(registers+(int)name);
}


void RegistersManagement::set(SegmentRegisters name, unsigned short v) {
	segmentRegisters[(int)name] = v;
}


void RegistersManagement::set(Registers8Bit name, unsigned char v) {
	registers[(int)name] = v;
}


void RegistersManagement::set_i8L(int regIndex, unsigned char v) {

	registers[(regIndex * 2)+1] = v;

	//memcpy(registers + regIndex+1, &v, 1);
}
void RegistersManagement::set_i8H(int regIndex, unsigned char v) {
	registers[regIndex * 2] = v;

	//memcpy(registers + regIndex, &v, 1);
}


RegistersManagement::~RegistersManagement()
{
}
