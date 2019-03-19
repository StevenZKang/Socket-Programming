#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "helper.h"

#define EMPTY 40000

int get_file_size(char *filename) {
    struct stat sbuf;

    if ((stat(filename, &sbuf)) == -1) {
       perror("stat");
       exit(1);
    }

    return sbuf.st_size;
}

/* A comparison function to use for qsort */
int compare_freq(const void *rec1, const void *rec2) {

    struct rec *r1 = (struct rec *) rec1;
    struct rec *r2 = (struct rec *) rec2;

    if (r1->freq == r2->freq) {
        return 0;
    } else if (r1->freq > r2->freq) {
        return 1;
    } else {
        return -1;
    }
}

//Divide file_size into N evenly distributed groups
int divide_even(int * child_sizes, int N, int file_size){
	
	if (N <= 0){
		return 1;
	}
	int q = file_size/N;
	int r = file_size % N; 
	//Increment the first r elements by 1
	for (int i = 0; i < N; i++){
		child_sizes[i] = q;
		if(r > 0){
			child_sizes[i]++;
			r--;
		} 
	}
	return 0; 
}

//Merging array of pipes into file helper
void merge(int pipe_fd[][2],int N, FILE *outfp){
	
	struct rec first_layer[N];
	int smallest;
	
	//Fill first_layer from reading end of pipes
	for(int j = 0; j < N; j++){
		if(read(pipe_fd[j][0], &first_layer[j], sizeof(struct rec)) == -1){
			perror("parent pipe read error");
			exit(1);
		}
	}
	
    while(1){
		smallest = 0;
		//Loop through first_layer to find rec with smallest freq
		for(int i = 0; i < N; i++){
			if(compare_freq(&first_layer[smallest],&first_layer[i]) == 1){
				smallest = i;
			}
		}
		
		//Exit loop is first_layer is competely EMPTY
		if(first_layer[smallest].freq == EMPTY){
			break; 
		}
		
		if(fwrite(&first_layer[smallest], sizeof(struct rec), 1, outfp) != 1){
			fprintf(stderr, "fwrite error"); 
		}
		//printf("word : %s freq : %i\n", first_layer[smallest].word, first_layer[smallest].freq);
		//If there is no more recs in the pipe to read, set that index's rec to EMPTY
		if (read(pipe_fd[smallest][0], &first_layer[smallest], sizeof(struct rec)) == 0){
			//printf("Pipe Empty \n");
			first_layer[smallest].freq = EMPTY; 
		}	
	}
				
}

