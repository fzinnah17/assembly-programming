#pragma once
class MemoryManagement
{
private:
	unsigned char *memory;
	int size;
	int IP;
	RegistersManagement *registers;
public:



	MemoryManagement(int size, RegistersManagement*);
	unsigned char get(int offset);
	void LoadMemory(std::ifstream &in);
	unsigned short getShort();
	unsigned short getShort(int offset);
	unsigned char getByte(int offset);
	void setIP(int _IP);
	int getIP();
	int getSIP();
	unsigned char getByte();
	void set(int offset, char * data, int len);
	void set(int offset, int  data, int len);
	std::string getString(int offset, int len);
	std::string getString(int offset);
	~MemoryManagement();
};

