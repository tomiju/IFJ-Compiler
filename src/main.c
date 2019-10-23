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

      TokenPTR testPole[1000]; // jak určíme velikost pole tokenů????
      int x = -1;

      do
      {
        
        x++;

        if (getToken(&testPole[x]) == 1)
         {
            printf("lex error %d\n", x);
            return 1;
         }

         printf("\n");
         printf("debug token values:\n");
         printf("value: %s\n", testPole[x]->dynamic_value );
         printf("number with exponent value: %lf\n",testPole[x]->numberValueWithExponent );
         printf("exponent_value: %d\n", testPole[x]->exponent_value );
         printf("length: %d\n",testPole[x]->size );
         printf("allocated_size:%d\n",testPole[x]->allocated_size );
         printf("type: %d\n",testPole[x]->type );
         printf("keyword: %d\n",testPole[x]->keyword );  

      } while (testPole[x]->type != 1);
      
      for (int i = 0; i < x; ++i)
      {
         freeMemory(testPole[i]);
      }
   }
   /**************************************************************************/

    return 0;
}