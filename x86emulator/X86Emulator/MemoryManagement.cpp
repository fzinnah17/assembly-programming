#include "pch.h"
#include "MemoryManagement.h"



MemoryManagement::MemoryManagement(int _size , RegistersManagement *_registers)
{
	registers = _registers;


	IP = 0x100;
	size = _size;
	memory = new unsigned char[size];
	memset(memory, 0x00, size);
}
unsigned char MemoryManagement::get(int offset)
{
	if (offset >= size)
		return (char)0;
	return memory[offset];
}

void MemoryManagement::LoadMemory(std::ifstream& in)
{
	in.seekg(0, std::ios::end);
	int fileSize = in.tellg();
	in.seekg(0, std::ios::beg);



	in.read((char*)(memory+ getSIP()), fileSize);


	return;

}



unsigned short MemoryManagement::getShort() {
	unsigned short r = *(unsigned short*)(memory + IP);
	IP += 2;
	return r;
}
unsigned short MemoryManagement::getShort(int offset) {
	return *(unsigned short*)(memory+offset);
}

unsigned char MemoryManagement::getByte(int offset) {
	return *(unsigned char*)(memory + (offset));
}
void MemoryManagement::setIP(int _IP) {
	IP = _IP;
}
int MemoryManagement::getIP() {
	return IP;
}


int MemoryManagement::getSIP() {
	return registers->get_i16(SegmentRegisters::CS) * 0x10 + IP;
}

unsigned char MemoryManagement::getByte() {
	unsigned char r= *(unsigned char*)(memory + (IP));
	IP++;
	return r;
}

void MemoryManagement::set(int offset, char* data, int len) {
	memcpy(memory + offset, (void*)data, len);
}
void MemoryManagement::set(int offset, int data, int len) {
	memcpy(memory + offset,(void*) &data, len);
}
std::string MemoryManagement::getString(int offset, int len) {
	std::string r = "";

	for (int i = 0; i < len;i++) {
		
		
		r = r + (char)memory[offset];
		
		offset++;
	}





	return r;
}
std::string MemoryManagement::getString(int offset) {
	std::string r = "";
	char current = '0';
	
	for (;;) {
		current = memory[offset];
		if (current == '$' || current == '\0') {
			break;
		}
		else {
			r = r + current;
		}
		offset++;
	}

		



	return r;
}

MemoryManagement::~MemoryManagement()
{
}
