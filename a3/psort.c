#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "helper.h"

void print_structs(struct rec* rec_array, int n){
	
	for(int i = 0; i<n;i++){
		printf("word : %s freq : %i\n", rec_array[i].word, rec_array[i].freq);
	}
}

void process_sort(int N, int *child_sizes, char *infile, FILE *outfp){

	int pipe_fd[N][2];
	//Create n child processes and set each child to where it should read 
	for (int i = 0; i < N; i++){
	
		//Pipe before fork
		pipe(pipe_fd[i]);
	
		int ret = fork();
	
		if (ret < 0){
			perror("fork");
			exit(1);
		}else if (ret == 0){
			//Should close previous pipe reading ends
			for (int j = 0; j < i; j++){
				close(pipe_fd[j][0]);
			}
			//Should close current pipe reading end
			close(pipe_fd[i][0]); 
			
			//Allocate memory to temporarily store structs for sorting
			struct rec *temp_recs = malloc(sizeof(struct rec) * child_sizes[i]);
			
			//Open file pointer in child
			FILE* infp;
			if ((infp = fopen(infile, "rb")) == NULL) {
        		fprintf(stderr, "Could not open %s\n", infile);
        	exit(1);
    		}
    		
			//Each child will read in 1/N of the file and use qsort to sort it	
			fread(temp_recs, sizeof(struct rec), child_sizes[i], infp); 
			qsort(temp_recs, child_sizes[i], sizeof(struct rec), compare_freq); 
			
			//print_structs(temp_recs, child_sizes[i]);
			
			//After sorting each child will write each record to the pipe
			write(pipe_fd[i][1], temp_recs, sizeof(struct rec) * child_sizes[i]); 
			close(pipe_fd[i][1]);
			
			//Free Memory
			free(temp_recs); 
			fclose(infp);
			exit(0); 
		
		}else{
			//Close writing end in parent
			close(pipe_fd[i][1]); 
		}
	}
	//Parent will merge the data from each of the children and write to output file
	merge(pipe_fd, N, outfp);
	fclose(outfp); 
}


int main(int argc, char** argv){
	
    extern char *optarg;
    int N;   
    int ch;
    int file_size; 
    char *infile = NULL;
    char *outfile = NULL; 
    FILE *outfp; 

    if (argc != 7) {
        fprintf(stderr, "Usage: psort -n <number of processes> -f <input file name> -o <output file name>\n");
        exit(1);
    }

    //Use getopt to read in command-line arguments
    while ((ch = getopt(argc, argv, "n:f:o:")) != -1) {
        switch(ch) {
        case 'n':
            N = strtol(optarg, NULL ,10);
            break;
        case 'f':
            infile = optarg;
            break;
		case 'o':
            outfile = optarg;
            break;
        default:
            fprintf(stderr, "Usage: psort -n <number of processes> -f <input file name> -o <output file name>\n");
            exit(1);
        }
    }
       
    if ((outfp = fopen(outfile, "wb")) == NULL) {
        fprintf(stderr, "Could not open %s\n", outfile);
        exit(1);
    }

    //Get the outfile size 
    file_size = get_file_size(infile)/sizeof(struct rec);
    if(file_size == 0){
    	exit(0);
	}
	//Check if number of recs exceeds N
	if(N > file_size){
		N = file_size;
	}
	 
    //Create an array of child sizes 
    int child_sizes[N]; 
    if (divide_even(child_sizes, N, file_size) == 1){
    	exit(0); 
	}
	printf("%i\n",child_sizes[0]);
	printf("%i\n",child_sizes[N-1]);
	
	process_sort(N, child_sizes, infile, outfp);
	fclose(outfp);
	return 0;
}

