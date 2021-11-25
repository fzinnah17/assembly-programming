// X86Emulator.cpp : Bu dosya 'main' işlevi içeriyor. Program yürütme orada başlayıp biter.
//

#include "pch.h"
#include <iostream>
#include "Machine.h"
int main(int argc, char *argv[])
{
	if (argc <= 1) {
		printf("usage: x86.exe [comFile] \n");	
		return 0;
	}


	Machine *m = new Machine();
	if (m->LoadFile(argv[1]) == false) {
		printf("%s file not found!",argv[1]);
		return -1;
	}

	printf("Executable File Loaded: %s\n",argv[1]);
	printf("Welcome to X86 Emulator\n");
	printf("1.) Run\n");
	printf("2.) Debug\n");
	printf("Choice:");
	int choice = 0;
	scanf("%d", &choice);
	clearInputBuffer;
	if (choice == 1) {
		m->run();
	}
	else if(choice == 2) {
		printf("# Press T to run next instruction!\n");
		printf("# Press Q to terminate emulator!\n");
		printf("# Press H to show all registers!\n");

		printf("# Press R:[RegisterName]=[HEXValue] to change register value!\n");
		printf("-Example: R:AL=00\n");
		printf("-Example: R:AX=0101\n");


		m->run(true);
	}

	return 1;
}

