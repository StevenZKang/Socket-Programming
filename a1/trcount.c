#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Constants that determine that address ranges of different memory regions

#define GLOBALS_START 0x400000
#define GLOBALS_END   0x700000
#define HEAP_START   0x4000000
#define HEAP_END     0x8000000
#define STACK_START 0xfff000000

int main(int argc, char **argv) {
    
    FILE *fp = NULL;

    if(argc == 1) {
        fp = stdin;

    } else if(argc == 2) {
        fp = fopen(argv[1], "r");
        if(fp == NULL){
            perror("fopen");
            exit(1);
        }
    } else {
        fprintf(stderr, "Usage: %s [tracefile]\n", argv[0]);
        exit(1);
    }

    unsigned long address; 
    char access; 
    int IMLS[5];
    int GHS[4]; 
    
    while(fscanf(fp, "%c %lx", &access, &address) != EOF){
    	if (access == 'I'){
	}
	else if (access = 'M'){
	}
	else if (access = 'L'){
	}
	else if (access = 'S';){
	}
	
	// Check for address location 
    } 
    
    /* Complete the implementation */


    /* Use these print statements to print the ouput. It is important that 
     * the output match precisely for testing purposes.
     * Fill in the relevant variables in each print statement.
     * The print statements are commented out so that the program compiles.  
     * Uncomment them as you get each piece working.
     */
    /*
    printf("Reference Counts by Type:\n");
    printf("    Instructions: %d\n", IMLS[0]);
    printf("    Modifications: %d\n", IMLS[1]);
    printf("    Loads: %d\n", IMLS[2]);
    printf("    Stores: %d\n", IMLS[3]);
    printf("Data Reference Counts by Location:\n");
    printf("    Globals: %d\n", GHS[0]);
    printf("    Heap: %d\n", GHS[1]);
    printf("    Stack: %d\n", GHS[2]);
    */

    return 0;
}
