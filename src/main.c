#include <stdio.h>
#include "scanner.h"


int main(int argc, char const *argv[]) {

	FILE *f;

   int debug = 1; // DEBUG

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


   /********** DEBUG SCANNER **********/
   if (debug)
   {
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
   

      // DEBUG:
      printf("\n");
      printf("debug token values:\n");
      printf("value: %s\n", test->dynamic_value );
      printf("number with exponent value: %lf\n",test->numberValueWithExponent );
      printf("exponent_value: %d\n", test->exponent_value );
      printf("length: %d\n",test->size );
      printf("allocated_size:%d\n",test->allocated_size );
      printf("type: %d\n",test->type );
      printf("keyword: %d\n",test->keyword );

      printf("\n");
      printf("debug token2 values:\n");
      printf("value: %s\n", test2->dynamic_value );
      printf("number with exponent value: %lf\n",test2->numberValueWithExponent );
      printf("exponent_value: %d\n", test2->exponent_value );
      printf("length: %d\n",test2->size );
      printf("allocated_size:%d\n",test2->allocated_size );
      printf("type: %d\n",test2->type );
      printf("keyword: %d\n",test2->keyword );

      printf("\n");
      printf("debug token3 values:\n");
      printf("value: %s\n", test3->dynamic_value );
      printf("number with exponent value: %lf\n",test3->numberValueWithExponent );
      printf("exponent_value: %d\n", test3->exponent_value );
      printf("length: %d\n",test3->size );
      printf("allocated_size:%d\n",test3->allocated_size );
      printf("type: %d\n",test3->type );
      printf("keyword: %d\n",test3->keyword );

      freeMemory(test);
      freeMemory(test2);
      freeMemory(test3);
   }
   /**************************************************************************/

    return 0;
}