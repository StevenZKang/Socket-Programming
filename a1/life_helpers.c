#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//print the characters of the given array following by a single newline character
void print_state(char *s, int size){
	
	for(int i = 0; i<size;i++){
		printf("%c",s[i]); 
	}
	printf("\n"); 
	
}

//update the state of the array according to the game of life rules
void update_state(char *s, int size){
	
	//let 0 rep . and 1 rep a X
	_Bool flip[size-1];
	flip[0] = 0; 
	for(int i = 1; i < size-1;i++){
		if(s[i-1] == s[i+1]){
			flip[i] = 0;
		}
		else{
			flip[i] = 1;
		}
	}
	
	for (int j = 1; j < size-1; j++){
		if(flip[j] == 0){
			s[j] = ".";
		}
		else{
			s[j] = "X"; 
		}
	}
	
	
}
