#include "error_handle.h"
#include <stdio.h>

int get_value(int start,int stop,char*str){
	int choice;
	do{
		printf("Enter %s :",str);
		scanf("%d",&choice);
		if(choice<start || choice>stop)printf("\nINVALID\nRE-ENTER\n");
	}while(choice<start || choice>stop);
	return choice;
}
