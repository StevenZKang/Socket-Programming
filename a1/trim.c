

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Reads a trace file produced by valgrind and an address marker file produced
 * by the program being traced. Outputs only the memory reference lines in
 * between the two markers
 */

int main(int argc, char **argv) {
    
    if(argc != 3) {
         fprintf(stderr, "Usage: %s tracefile markerfile\n", argv[0]);
         exit(1);
    }

    // Addresses should be stored in unsigned long variables
    // unsigned long start_marker, end_marker;
	FILE *fp = fopen(argv[2], "r");
	unsigned long start_marker;
	unsigned long end_marker; 
	unsigned long address; 
	char access; 
	
	fscanf(fp, "%lx %lx", &start_marker, &end_marker);
	fclose(fp);
	
	FILE *fp2 = fopen(argv[1], "r");
	
	
	_Bool keep = 0; 
	while(fscanf(fp2, "%c %lx,%*i\n", &access , &address)!= EOF){
		if (keep == 0){
			if (address == start_marker){
			keep = 1;
			}
		}
		else{	
			if (address == end_marker){
				keep = 0;
				break; 
			}
			printf("%c,%#lx\n", access, address);
		}
	}
	
	fclose(fp2);
		
    /* For printing output, use this exact formatting string where the
     * first conversion is for the type of memory reference, and the second
     * is the address
     */
    // printf("%c,%#lx\n", VARIABLES TO PRINT GO HERE);
    return 0;
}

