#include <stdio.h>
#include "scanner.h"


int main(int argc, char const *argv[]) {

	FILE *f;

	if (argc == 1)
   {
      printf("No input file.\n");
      return -1;
   }
   if ((f = fopen(argv[1], "r")) == NULL)
   {
      printf("File couldn't be opened.\n");
      return -1;
   }

   setSourceFile(f);



   TokenPTR test;
   if (getToken(&test) == 1)
   {
	   	printf("error\n");
	   	return 1;
   }
    
    return 0;
}