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

      iStack indent_stack = initStack();
      
      if (indent_stack == NULL)
      {
         return -1;
      }
      
      TokenPTR testPole[1000]; // jak určíme velikost pole tokenů????
      int x = -1;

      do
      {
        
        x++;

        if (getToken(&testPole[x], &indent_stack) == 1)
         {
            printf("lex error %d\n", x);
            return 1;
         }

         printf("\n");
         printf("debug token (number %d) values:\n", x); 
         printf("value: %s\n", testPole[x]->dynamic_value );
         printf("number with exponent value: %lf\n",testPole[x]->number_value );
         printf("length: %d\n",testPole[x]->size );
         printf("allocated_size:%d\n",testPole[x]->allocated_size );
         printf("type: %d\n",testPole[x]->type );
         printf("keyword: %d\n",testPole[x]->keyword );  

         printf("\nStack test: \n");
         printf("init stack top: %d\n", indent_stack->value);
         pushStack(&indent_stack, 5);
         printf("stack top after push: %d\n", indent_stack->value);
         popStack(&indent_stack);
         printf("stack top after pop: %d\n", indent_stack->value);

      } while (testPole[x]->type != 1);
      
      for (int i = 0; i < x; ++i)
      {
         freeMemory(testPole[i]);
      }
   }
   /**************************************************************************/

    return 0;
}