#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 256
#define MAX_PASSWORD 10

#define SUCCESS "Password verified\n"
#define INVALID "Invalid password\n"
#define NO_USER "No such user\n"

int main(void) {
  char user_id[MAXLINE];
  char password[MAXLINE];
  int pipe_fd[2];
  int status; 
  int result;
  
  //Must memeset to all nulls so its length of MAX_PASSWORD
  memset(user_id, '\0', MAX_PASSWORD);
  memset(password, '\0', MAX_PASSWORD);
  
  if(fgets(user_id, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }
  if(fgets(password, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }
  
  if ((pipe(pipe_fd)) == -1) {
	perror("pipe");
	exit(1);
  }

  close(pipe_fd[0]);
  write(pipe_fd[1], user_id, MAX_PASSWORD);
  write(pipe_fd[1], password, MAX_PASSWORD);
	
  int ret;
  char *argv[1];
  argv[0] = NULL;
  
  ret = fork(); 
  
  if (ret == -1){
	perror("fork error");
	exit(1);
  } 
  else if(ret == 0){
  	  close(pipe_fd[1]); 
  	  dup2(pipe_fd[0], 0);
  	  execvp("validate", argv);
  	  //Child process will exit in validate.c	
  }
  else {
  	wait(&status);
  	if (WIFEXITED(status)){
  		result = WEXITSTATUS(status); 
    }
    close(pipe_fd[1]); 
  }

  //Display result
  if(result == 0){
  	printf(SUCCESS);
  	
  }else if(result == 2){
  	printf(INVALID);
  	
  }else if(result == 3){
  	printf(NO_USER);
  	
  }

  return 0;
}






