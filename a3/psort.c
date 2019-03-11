#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "helper.h"

//Divide file_size into N evenly distributed groups
void divide_even(int * child_sizes, int N, int file_size){

  


}


void process_sort(int N, int *child_sizes, FILE *infp){

	struct rec pipe_fd[N][2];

	//Dynamically allocate a list of pointers of size n.
	struct rec **ptr_list = malloc(sizeof(struct *rec)* N); 
	for (int j = 0; j < N, i++){
		ptr_list[i] = malloc(sizeof(struct rec) * child_sizes[i]); 
	}
	
	//Create n child processes and set each child to where it should read 
	for (int i = 0; i < n; i++){
	
		//Pipe before fork
		pipe(pipe_fd[i]);
	
	
		int ret = fork();
	
		if (ret < 0){
			perror("fork");
			exit(1);
		}else if (ret == 0){
			//Should close previous pipe reads

			//Each child will read in 1/N of the file and use qsort to sort it	
			fread(ptr_list[i], sizeof(struct rec), child_sizes[i], infp); 
			qsort(ptr_list[i], child_sizes[i], sizeof(struct rec), compare_freq); 
		
			//After sorting each child will write each record to the pipe
			close(pipe_fd[i][0]); 
			write(pipe_fd[i][1], ptr_list[i]); 
		
		}else{
			close(pipe_fd[i][1]); 
		
		
		
		}
	}

	//Parent wait for children to finish
	//Parent will merge the data from each of the children 


	//Parent will write final list to output file

}


int main(int argc, char** argv){
	
    extern char *optarg;
    int N;   
    int file_size; 
    char *infile = NULL;
    char *outfile = NULL; 
    FILE *infp, *outfp; 

    if (argc != 7) {
        fprintf(stderr, "Usage: psort -n <number of processes> -f <input file name> -o <output file name>\n");
        exit(1);
    }

    //Use getopt to read in command-line arguments
    while ((ch = getopt(argc, argv, "n:f:o")) != -1) {
        switch(ch) {
        case 'n':
            N = optarg;
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
   
    if ((infp = fopen(infile, "rb")) == NULL) {
        fprintf(stderr, "Could not open %s\n", infile);
        exit(1);
    }
    if ((outfp = fopen(outfile, "wb")) == NULL) {
        fprintf(stderr, "Could not open %s\n", outfile);
        exit(1);
    }

    //Get the outfile size 
    file_size = get_file_size(infile);

    //Create an array of child sizes 
    int child_sizes[N]; 
    divide_even(child_sizes, N, file_size); 
}

