#include <stdio.h>
#include <stdlib.h>

int phone_loop(){
   char s1[11];
   int num1 = 0;
   int error = 0;
   scanf("%s",s1);
   
   while(scanf("%d",&num1) != EOF){
	   if (num1 == -1){
		printf("%s\n",s1);
	   }
	   else if(num1 >= 0 && num1 <= 9){
		printf("%c\n",s1[num1]);
	   }
	   else{
		printf("ERROR\n");
		error = 1;
	   }
  } 
  return error; 
}

int main(){
	
  return phone_loop();
  	
}
