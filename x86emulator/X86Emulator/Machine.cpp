#include "pch.h"
#include "Machine.h"
#include <ctime>

Machine::Machine()
{

	registers = new RegistersManagement();
	std::srand(std::time(NULL));


	memory = new MemoryManagement(1000000,registers);
	IP = 0x100;
	
}


Machine::~Machine()
{
}

bool Machine::LoadFile(char *file) {
	std::ifstream input(file, std::ios::binary);
	if (input.is_open()) {
		memory->LoadMemory(input);
		input.close();
		return true;
	}
	return false;
}

void Machine::call_DOSFunctionDispatcher() {

	/*
	Call DOS functions by using AH register.
	Reference:
	https://stanislavs.org/helppc/int_21.html
	*/

	int AH = registers->get_i8(Registers8Bit::AH);
	if (AH == 0x4C) {
		/*
		Terminate process with return code(AL)
		*/
		int retCode = registers->get_i8(Registers8Bit::AL);
		printf("Process terminated with return code(%d)\n", retCode);
		exit(retCode);
	}else
		/*
		Read From File or Device Using Handle
		AH = 3F
		BX = file handle
		CX = number of bytes to read
		DS:DX = pointer to read buffer


		on return:
		AX = number of bytes read is CF not set
		   = error code if CF set  (see DOS ERROR CODES)

		*/
		if (AH == 0x3F) {
			int fileHandle = registers->get_i16(Registers16Bit::BX);
			auto itr = addresses.find(fileHandle);
			if (itr == addresses.end()) {
				registers->setFlag(F_CF);
				registers->set_i16(Registers16Bit::AX, 0x05);

			}
			else {
				registers->clearFlag(F_CF);
				char inp[128]; memset(inp, 0x00, 128);

				if (feof((FILE*)itr->second) == 0) {
					int numberOfBytesRead = registers->get_i16(Registers16Bit::CX);
					fread(inp, numberOfBytesRead, 1, (FILE*)itr->second);
					unsigned short bufferPointer = registers->get_i16(Registers16Bit::DX);
					memory->set(bufferPointer, inp, strlen(inp));

				}
				registers->set_i16(Registers16Bit::AX, strlen(inp));



			}
		}else
		/*
		Close File Using Handle
		AH = 3E
		BX = file handle to close


		on return:
		AX = error code if CF set  (see DOS ERROR CODES)

		
		*/
		if (AH == 0x3E) {
			int fileHandle = registers->get_i16(Registers16Bit::BX);

			auto itr = addresses.find(fileHandle);
			if (itr == addresses.end()) {
				registers->setFlag(F_CF);
				registers->set_i16(Registers16Bit::AX, 0x05);

			}
			else {
				fclose((FILE*)itr->second);
				addresses.erase(itr);
			}

		}else

		/*
		
		Write To File or Device Using Handle

		AH = 40h
		BX = file handle
		CX = number of bytes to write, a zero value truncates/extends
			 the file to the current file position
		DS:DX = pointer to write buffer


		on return:
		AX = number of bytes written if CF not set
		   = error code if CF set  (see DOS ERROR CODES)


		*/
		if (AH == 0x40) {
			int fileHandle = registers->get_i16(Registers16Bit::BX);
			int writeSize = registers->get_i16(Registers16Bit::CX);



			std::string content = memory->getString(registers->get_i16(Registers16Bit::DX));
			

			auto itr = addresses.find(fileHandle);
			if (itr == addresses.end()) {
				registers->setFlag(F_CF);
				registers->set_i16(Registers16Bit::AX, 0x05);

			}
			else {
				registers->clearFlag(F_CF);
				registers->set_i16(Registers16Bit::AX, content.length());
				fputs(content.c_str(),(FILE*)itr->second);


			}


		}
		else
		/*
		Buffered Keyboard Input

		AH = 0A
		DS:DX = pointer to input buffer of the format:
		*/
	if (AH == 0x0A) {
		char inp[32];
		printf("Keyboard Input:");
		scanf("%s", inp);
		clearInputBuffer;
		unsigned short bufferPointer = registers->get_i16(Registers16Bit::DX);
		memory->set(bufferPointer, inp, strlen(inp));

	}
	else if (AH == 0x3C) {
		// Create File
		// No need just return OK
		registers->clearFlag(F_CF);
		registers->set_i16(Registers16Bit::AX, std::rand() % 30);
	}else
	if (AH == 0x3D) {


		/*
		Open File Using Handle

			AH = 3C
			CX = file attribute
			DS:DX = pointer to ASCIIZ path name


			on return:
			CF = 0 if successful
			   = 1 if error
			AX = files handle if successful
			   = error code if failure  (see DOS ERROR CODES)


			- if file already exists, it is truncated to zero bytes on opening
		*/


		std::string fileName = memory->getString(registers->get_i16(Registers16Bit::DX));
		int fileAttribute = registers->get_i8(Registers8Bit::AL);

		FILE *f = nullptr; 
		

		if(fileAttribute == 0)
			f = fopen(fileName.c_str(), "r");
		else if(fileAttribute == 2)
			f = fopen(fileName.c_str(), "w");
		else
			f = fopen(fileName.c_str(), "w+");

		if (f == nullptr) {
			registers->setFlag(F_CF);
			// lets return 'Access denied'(0x05) error
			registers->set_i16(Registers16Bit::AX, 0x05);
		}
		else {
			registers->clearFlag(F_CF);

			unsigned short fileHandle2 = std::rand() % 30;
			addresses.insert( std::make_pair(fileHandle2, (unsigned int)f) );
			registers->set_i16(Registers16Bit::AX, fileHandle2);

			printf("File Handle Created %04X, Original:%08X\n", fileHandle2,f);


		}
		
	}
	else
	if (AH == 0x2C) {
		/*
		Get Time
		on return:
			CH = hour (0-23)
			CL = minutes (0-59)
			DH = seconds (0-59)
			DL = hundredths (0-99)

		*/
		

		std::time_t t = std::time(0);
		std::tm* now = std::localtime(&t);



		registers->set(Registers8Bit::CH, now->tm_hour);
		registers->set(Registers8Bit::CL, now->tm_min);


		int v = registers->get_i16(Registers16Bit::CX);


		registers->set(Registers8Bit::DH, now->tm_sec);
		registers->set(Registers8Bit::DL, std::rand() % 100);

	}
	else
	if (AH == 0x9) {
		/*
		Print String to STDOUT up to "$"
		*/
		std::string s = memory->getString(registers->get_i16(Registers16Bit::DX));
		printf("STDOUT: %s\n", s.c_str());
	}
	else
		printf("Unknown DOSFunction %X\n", AH);
}

void Machine::call_near(int address) {

	IP = memory->getIP();

	memory->setIP(IP+address);


}
void Machine::changeRegisterValue(char *regName, short regVal) {
	if (!strcmp(regName, "AX")) {
		registers->set_i16(Registers16Bit::AX, regVal);
	}else if (!strcmp(regName, "CX")) {
		registers->set_i16(Registers16Bit::CX, regVal);
	}else if (!strcmp(regName, "DX")) {
		registers->set_i16(Registers16Bit::DX, regVal);
	}
	else if (!strcmp(regName, "BX")) {
		registers->set_i16(Registers16Bit::BX, regVal);
	}
	else if (!strcmp(regName, "SP")) {
		registers->set_i16(Registers16Bit::SP, regVal);
	}
	else  if (!strcmp(regName, "BP")) {
		registers->set_i16(Registers16Bit::BP, regVal);
	}
	else  if (!strcmp(regName, "SI")) {
		registers->set_i16(Registers16Bit::SI, regVal);
	}
	else  if (!strcmp(regName, "DI")) {
		registers->set_i16(Registers16Bit::DI, regVal);
	}
	else  if (!strcmp(regName, "AH")) {
		registers->set(Registers8Bit::AH, regVal);
	}
	else  if (!strcmp(regName, "AL")) {
		registers->set(Registers8Bit::AL, regVal);
	}
	else  if (!strcmp(regName, "CH")) {
		registers->set(Registers8Bit::CH, regVal);
	}
	else  if (!strcmp(regName, "CL")) {
		registers->set(Registers8Bit::CL, regVal);
	}
	else  if (!strcmp(regName, "DH")) {
		registers->set(Registers8Bit::DH, regVal);
	}
	else  if (!strcmp(regName, "DL")) {
		registers->set(Registers8Bit::DL, regVal);
	}
	else  if (!strcmp(regName, "BH")) {
		registers->set(Registers8Bit::BH, regVal);
	}
	else  if (!strcmp(regName, "BL")) {
		registers->set(Registers8Bit::BL, regVal);
	}
	else  if (!strcmp(regName, "SPL")) {
		registers->set(Registers8Bit::SPL, regVal);
	}
	else  if (!strcmp(regName, "BPL")) {
		registers->set(Registers8Bit::BPL, regVal);
	}
	else  if (!strcmp(regName, "SiL")) {
		registers->set(Registers8Bit::SiL, regVal);
	}
	else  if (!strcmp(regName, "DiL")) {
		registers->set(Registers8Bit::DiL, regVal);
	}
	
	
}
void Machine::runInstruction(unsigned char instruction) {
	if (debug) {
		print_debug(instruction);
		char cmd[10]; memset(cmd, 0x00, 10);
		char regName[10]; memset(regName, 0x00, 10);
		int regVal = 0;

		do {
			
			printf("->");
			
			cmd[0] = getchar();
			

			if (cmd[0] == 'R' || cmd[0] == 'r') {
				char tmp = getchar();
				if (tmp == ':') {
					regName[0] = getchar();
					regName[1] = getchar();
					tmp = getchar();
					if (tmp == '=') {
						scanf("%x", &regVal);
						
						changeRegisterValue(regName, regVal);
						clearInputBuffer;
						
					}
				}
			}else if (cmd[0] == 'Q' || cmd[0] == 'q') {
				printf("Bye!\n");
				exit(0);
			}
			else if (cmd[0] == 'H' || cmd[0] == 'h') {
				clearInputBuffer;
				print_debug(instruction);
			}
			else {
				clearInputBuffer;
			}

		} while (cmd[0] != '\0' && cmd[0] != 'T' && cmd[0] != 't');
		
		
		

		/*
		int op = getchar();
		do {
			if (op == 'R') {

			}
		} while (op == '\n' || op == 'T');
		clearInputBuffer;*/
	}



	switch (instruction)
	{

	case 0xC3: {
		memory->setIP(IP);
		break;
	}
	// MOV	r/m8	r8
	case 0x88: {
		char reg0 = memory->getByte();
		char reg1 = memory->getByte(); // -1

		if (reg0 == 0x65) {
			//MOV DI,AH
			int v = registers->get_i16(Registers16Bit::DI);
			char t = memory->get( registers->get_i8(Registers8Bit::AH) );
			memory->set(registers->get_i16(Registers16Bit::DI) + reg1, t, 1);
		}


		break;
	}
	// MOV
	case 0x8B: {

		BYTE reg = memory->getByte();
		if (reg == 0xD8) {
			// mov %ax, %bx

			registers->set_i16(Registers16Bit::BX, registers->get_i16(Registers16Bit::AX));

		}else if (reg == 0xFA) {
			// mov DI, DX

			registers->set_i16(Registers16Bit::DI, registers->get_i16(Registers16Bit::DX));

		}
		else {
			printf("0x8B UNKNOWN MOV\n");
		}

		break;
	}
	case 0x75: {
		//JNE Jump short if not equal (ZF=0).
		char labelOff = memory->getByte();

		if (registers->getFlag(F_ZF) == 0)
			memory->setIP(memory->getIP() + labelOff);


		break;
	}
	case 0x38: {
		//CMP	r/m8	r8
		BYTE reg = memory->getByte();
		if (reg == 0x1D) {
			//cmp  	byte ptr[di],bl

			unsigned short dataAddress = registers->get_i16(Registers16Bit::DI);

			unsigned short imm16 = memory->get(dataAddress);
			unsigned short regData = registers->get_i8(Registers8Bit::BL);
			if (imm16 == regData) {
				registers->setFlag(F_ZF);
				registers->clearFlag(F_CF);

			}
			else if (regData > imm16) {
				registers->clearFlag(F_ZF);
				registers->setFlag(F_CF);

			}
			else {
				registers->clearFlag(F_ZF);
				registers->clearFlag(F_CF);

			}


		}


		



		break;
	}
	case 0x3D: {
		//CMP	AX,imm16
		// https://www.aldeid.com/wiki/X86-assembly/Instructions/cmp

		unsigned short imm16 = memory->getShort();
		unsigned short regData = registers->get_i16(Registers16Bit::AX);
		if (imm16 == regData) {
			registers->setFlag(F_ZF);
			registers->clearFlag(F_CF);

		}
		else if (regData > imm16) {
			registers->clearFlag(F_ZF);
			registers->setFlag(F_CF);

		}
		else {
			registers->clearFlag(F_ZF);
			registers->clearFlag(F_CF);

		}
		


		break;
	}
	//INC DX
	case 0x42: {
		registers->set_i16(Registers16Bit::DX, registers->get_i16(Registers16Bit::DX) + 1);
		break;
	}
	case 0x8A: {
		BYTE reg = memory->getByte();

		if (reg == 0x0E) {
			unsigned short dataAddress = memory->getShort();

			int v = registers->get_i8(Registers8Bit::CL);
			memory->set(dataAddress, v, 2);
		}else if (reg == 0x16) {
			unsigned short dataAddress = memory->getShort();

			int v = registers->get_i8(Registers8Bit::DL);
			memory->set(dataAddress, v, 2);
		}
		else if (reg == 0X1C) {
			//mov 	bl, byte ptr[si]
			unsigned short dataAddress = registers->get_i16(Registers16Bit::SI);
			
			registers->set(Registers8Bit::BL, memory->get(dataAddress));
		}
		else {
			printf("0x8A unknown reg\n");
		}



		break;
	}
	// ADD 
	case 0x83: {
		BYTE reg = memory->getByte();
		BYTE i = memory->getByte();
		if (reg == 0xC2) {
			registers->set_i16(Registers16Bit::DX, registers->get_i16(Registers16Bit::DX) + i);


		}
		else {
			printf("0x83 unknown reg\n");

		}


		break;
	}


	case 0x89: {
		BYTE reg = memory->getByte();
		unsigned short dataAddress = memory->getShort();

		if (reg == 0x0E) {
			int v = registers->get_i16(Registers16Bit::CX);
			memory->set(dataAddress, v, 2);
		}
		else {
			printf("0x89 unknown reg\n");
		}



		break;
	}
	/*
	
		NOP ; Do Nothing
	*/
	case 0x90:
		
		break;

	/*
		XOR R16, r/m16;		Logical Exclusive OR
	*/

	case 0x33: {

		BYTE param = memory->getByte();

		int reg0Index = (param & 0x38) >> 3;
		int reg1Index = param & 7;
		int result = registers->get_i16(reg0Index) ^ registers->get_i16(reg1Index);

		registers->set_i16(reg0Index, result);

		break;
	}
	/*
		CALL rel16	; Call near, relative, displacement relative to next instruction
	*/
	case 0xE8: {
		unsigned short address= memory->getShort();
		call_near(address);

		break;
	}
	/*
		MOV	r16	imm16
	*/
	case 0xB8:
	case 0xB9:
	case 0xBA:
	case 0xBB:
	case 0xBC:
	case 0xBD:
	case 0xBE:
	case 0xBF:
	{
		short imm16 = memory->getShort();
		registers->set_i16(instruction & 7, imm16);
		break;
	}

	/*
	MOV	r8	imm8	(L)
	*/
	case 0xB0:
	case 0xB1:
	case 0xB2:
	case 0xB3:
	
	{
		unsigned char imm8 = memory->getByte();
		registers->set_i8L(instruction & 3, imm8);
		break;
	}
	/*
	MOV	r8	imm8	(H)
	*/
	case 0xB4:
	case 0xB5: 
	case 0xB6: 
	case 0xB7: {
		
		unsigned char imm8 = memory->getByte();
		registers->set_i8H(instruction & 3, imm8);
		break;
	}
	/*
	 INT	imm8	;Call to Interrupt Procedure
	*/
	case 0xCD: {

		unsigned char intForm = memory->getByte();
		if (intForm == 0x21) {
			call_DOSFunctionDispatcher();
		}

		break;
	}



	default:
		
		printf("unknown inst %X\n",instruction);
		
		break;
	}

	
}
void Machine::print_debug(unsigned char instruction) {
	printf("CF=%04X\tPF=%04X\tAF=%04X\tZF=%04X\tSF=%04X\tTF=%04X\tIF=%04X\tDF=%04X\tOF=%04X\n",
		registers->getFlag(F_CF),
		registers->getFlag(F_PF),
		registers->getFlag(F_AF),
		registers->getFlag(F_ZF),
		registers->getFlag(F_SF),
		registers->getFlag(F_TF),
		registers->getFlag(F_IF),
		registers->getFlag(F_DF),
		registers->getFlag(F_OF));
	printf("AX=%04X\tCX=%04X\tDX=%04X\tBX=%04X\tSP=%04X\tBP=%04X\tSI=%04X\tDI=%04X\n", registers->get_i16(Registers16Bit::AX), registers->get_i16(Registers16Bit::CX), registers->get_i16(Registers16Bit::DX), registers->get_i16(Registers16Bit::BX), registers->get_i16(Registers16Bit::SP), registers->get_i16(Registers16Bit::BP), registers->get_i16(Registers16Bit::SI), registers->get_i16(Registers16Bit::DI));
	
	printf("DS=%04X\tES=%04X\tSS=%04X\tCS=%04X\tIP=%04X\n", registers->get_i16(SegmentRegisters::DS), registers->get_i16(SegmentRegisters::ES), registers->get_i16(SegmentRegisters::SS), registers->get_i16(SegmentRegisters::CS), memory->getIP());

	printf("%04X:%04X\tOpCode:%02X\n", registers->get_i16(SegmentRegisters::CS),memory->getIP(),instruction);
}
void Machine::run(bool _debug /*= false*/) {
	debug = _debug;

	memory->setIP(0x100);
	registers->set_i16(Registers16Bit::SP, 0xFFFE);


	while (1) {
		unsigned char instruction = memory->getByte();



		
		runInstruction(instruction);
		
	}
}
