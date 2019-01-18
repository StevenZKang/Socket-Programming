#include <stdio.h>
#include <stdlib.h>

int phone(){
   char s1[11];
   int num1;
   scanf("%s",s1);
   scanf("%i",&num1);
   
   if (num1 == -1){
	printf("%s",s1);
	return 0;
   }
   else if(num1 >= 1 && num1 <= 9){
	printf("%c",s1[num1]);
	return 0;
   }
   else{
	printf("ERROR");
	return 1;
   }
  
}

int main(){
	
  phone();
	
}
