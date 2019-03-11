#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "helper.h"


struct rec pipe_fd[n][2];

//Split the file by n even parts
//Create n child processes and set each child to where it should read 
for (int i = 0; i < n; i++){
	
	//Pipe before fork
	pipe(pipe_fd[i]);
	
	
	int ret = fork();
	
	if (ret < 0){
		perror("fork");
		exit(1);
	}else if (ret == 0){
	//Each child will read in 1/N of the file and use qsort to sort it	
		
	//Dynamically allocate a list of pointers of size n.
	//Each pointer will point to a list of rec. 	
	
	//After sorting each child will write each record to the pipe
		
	}else{
		
		
		
		
	}
}

//Parent wait for children to finish
//Parent will merge the data from each of the children 


//Parent will write final list to output file



int main(){
	//Use getopt to read in command-line arguments
	//Get the outfile size 
}

