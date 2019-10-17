#include <stdio.h>
#include "scanner.h"


int main(int argc, char const *argv[]) {

	FILE *f;

	if (argc == 1) // TODO: do jedné funkce
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



   TokenPTR test, test2, test3;
   if (getToken(&test) == 1)
   {
	   	printf("lex error\n");
	   	return 1;
   }

   if (getToken(&test2) == 1)
   {
         printf("lex error 2\n");
         return 1;
   }

   if (getToken(&test3) == 1)
   {
         printf("lex error 3\n");
         return 1;
   }

   /*int testik = 2e3;
   printf("test: %d\n", testik );*/

   printf("\n");
   printf("debug token values:\n");
   printf("\n");
   printf("value: %s\n", test->dynamic_value );
   printf("integer: %d\n",test->integer );
   printf("decimal (float):%f \n",test->decimal );
   printf("type: %d\n",test->type );
   printf("keyword: %d\n",test->keyword );

   printf("\n");
   printf("debug token2 values:\n");
   printf("\n");
   printf("value: %s\n", test2->dynamic_value );
   printf("integer: %d\n",test2->integer );
   printf("decimal (float):%f \n",test2->decimal );
   printf("type: %d\n",test2->type );
   printf("keyword: %d\n",test2->keyword );

   printf("\n");
   printf("debug token3 values:\n");
   printf("\n");
   printf("value: %s\n", test3->dynamic_value );
   printf("integer: %d\n",test3->integer );
   printf("decimal (float):%f \n",test3->decimal );
   printf("type: %d\n",test3->type );
   printf("keyword: %d\n",test3->keyword );

    return 0;
}