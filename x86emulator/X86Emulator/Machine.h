#pragma once
#define F_CF 0x0001             /* CARRY flag  */
#define F_PF 0x0004             /* PARITY flag */
#define F_AF 0x0010             /* AUX  flag   */
#define F_ZF 0x0040             /* ZERO flag   */
#define F_SF 0x0080             /* SIGN flag   */
#define F_TF 0x0100             /* TRAP flag   */
#define F_IF 0x0200             /* INTERRUPT ENABLE flag */
#define F_DF 0x0400             /* DIR flag    */
#define F_OF 0x0800             /* OVERFLOW flag */

#define clearInputBuffer		while ((getchar()) != '\n')

class Machine
{
private:
	std::map<unsigned short, unsigned int>	addresses;


	unsigned short FLAGS;

	int IP;

	MemoryManagement *memory;
	RegistersManagement *registers;
	void runInstruction(unsigned char instruction);

	void print_debug(unsigned char instruction);

	bool debug;

public:
	Machine();
	~Machine();
	void call_DOSFunctionDispatcher();
	void call_near(int address);
	void changeRegisterValue(char * regName, short regVal);
	void run(bool _debug = false);
	bool LoadFile(char *file);
};

