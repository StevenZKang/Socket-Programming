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

  if(fgets(user_id, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }
  if(fgets(password, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }
  
  
  if ((pipe(pipe_fd[i-1])) == -1) {
	perror("pipe");
	exit(1);
  }
  //Must memeset to all nulls so its length of MAX_PASSWORD
  write(pipe_fd[1], user_id, MAXLINE);
  write(pipe_fd[1], password, MAXLINE);
	
  dup2(pipe_fd[0], 0);

  int ret;
  pipe(pipe_fd); 
  if (ret == -1){
	perror("fork error");
	exit(1);
  } 
  else if(ret == 0){
  	
  }


  return 0;
}
