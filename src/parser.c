/**
 * Předmět:  IFJ
 * Projekt:  Implementace překladače imperativního jazyka IFJ19
 * Varianta: Tým 018, varianta II
 * Soubor:   parser.c
 * 
 *
 * Datum:    xx.xx.xxxx
 *
 * Autoři:   Matej Hockicko  <xhocki00@stud.fit.vutbr.cz>
 *           Tomáš Julina    <xjulin08@stud.fit.vutbr.cz>
 *           Tomáš Kantor    <xkanto14@stud.fit.vutbr.cz>
 *           Lukáš Kuchta    <xkucht09@stud.fit.vutbr.cz>
 */

#include "scanner.h"
#include "parser.h"


iStack indent_stack; //required by scanner
TokenPTR token_ptr; //pointer to the token

int currentLine = 1;
int inFunction = 0;


int expression(){
  
   if(token_ptr->type != TOKEN_INT  ){
       
        fprintf(stderr,"line: %d\n",currentLine);
        return SYNATX_ERROR;
    }

    if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
        fprintf(stderr,"line: %d\n",currentLine);
        return LEX_ERROR;
    }
    return TOKEN_OK;
}

int param(){
      
    if(inFunction){
        if(token_ptr->type != TOKEN_IDENTIFIER  ){
            fprintf(stderr,"line: %d\n",currentLine);
            return SYNATX_ERROR;
        }
        if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
            fprintf(stderr,"line: %d\n",currentLine);
            return LEX_ERROR;
        }
    }else{
        return expression();
    }

    return TOKEN_OK;
}

int paramList2(){
    int result = TOKEN_OK;

    if(token_ptr->type != TOKEN_COMMA){
        fprintf(stderr,"line: %d\n",currentLine);
        return SYNATX_ERROR;
    }

    if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
        fprintf(stderr,"line: %d\n",currentLine);
        return LEX_ERROR;
    }
    //---------------------------------------------------
    
    result = param();

    if(result != TOKEN_OK) {
        return result;
    }
    //---------------------------------------------------

    if(token_ptr->type == TOKEN_COMMA){
        result = paramList2();
        if(result != TOKEN_OK){
            return result;
        }
    }

    if(token_ptr->type == TOKEN_RIGHT_BRACKET){
        return TOKEN_OK;
    }

    fprintf(stderr,"line: %d\n",currentLine);
        return SYNATX_ERROR;
}

int paramList(){
    int result = TOKEN_OK;

    if(token_ptr->type == TOKEN_RIGHT_BRACKET){
        return TOKEN_OK; 
    }

    result = param();

    if(result != TOKEN_OK) {
        return result;
    }
    //---------------------------------------------------

    if(token_ptr->type == TOKEN_COMMA){
        result = paramList2();
        if(result != TOKEN_OK){
            return result;
        }
    }
    return result;
}

int funcDef(){
    inFunction = 1;

    int result = TOKEN_OK;

    if(token_ptr->type != KEYWORD_DEF){
        fprintf(stderr,"line: %d\n",currentLine);
        return SYNATX_ERROR; 
    }
    
    if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
        fprintf(stderr,"line: %d\n",currentLine);
        return LEX_ERROR;
    }
    //---------------------------------------------------
    if(token_ptr->type != TOKEN_IDENTIFIER){
        fprintf(stderr,"line: %d\n",currentLine);
        return SYNATX_ERROR; 
    }
    
    if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
        fprintf(stderr,"line: %d\n",currentLine);
        return LEX_ERROR;
    }
    //---------------------------------------------------

    if(token_ptr->type != TOKEN_LEFT_BRACKET){
        fprintf(stderr,"line: %d\n",currentLine);
        return SYNATX_ERROR; 
    }

    if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
        fprintf(stderr,"line: %d\n",currentLine);
        return LEX_ERROR;
    }

    //---------------------------------------------------
    
    result = paramList();
    if(result != TOKEN_OK)return result;
    
    //---------------------------------------------------
    

    if(token_ptr->type != TOKEN_RIGHT_BRACKET){
        fprintf(stderr,"line: %d\n",currentLine);
        return SYNATX_ERROR; 
    }

    if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
        fprintf(stderr,"line: %d\n",currentLine);
        return LEX_ERROR;
    }
    //---------------------------------------------------

    if(token_ptr->type != TOKEN_COLON){
        fprintf(stderr,"line: %d\n",currentLine);
        return SYNATX_ERROR;
    }

    if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
        fprintf(stderr,"line: %d\n",currentLine);
        return LEX_ERROR;
    }
    //---------------------------------------------------

    if(token_ptr->type != TOKEN_EOL){
        fprintf(stderr,"line: %d\n",currentLine);
        return SYNATX_ERROR;
    }

    currentLine += 1;    

    if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
        fprintf(stderr,"line: %d\n",currentLine);
        return LEX_ERROR;
    }

    //---------------------------------------------------
    //TODO body
    //---------------------------------------------------
    inFunction = 0;
    return result;
}


int program(){
    int result;

    if(token_ptr->type == KEYWORD_DEF){
        result = funcDef();

        if(result != TOKEN_OK){
            return result;
        }
        
        result = program();
    }
        
    return result;
}

int parse(){
    //inicilazation
    indent_stack = initStack();
      
    if (indent_stack == NULL)
    {
        fprintf(stderr,"Allocation error.\n");
        return -1;
    }

    //get first token
    if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
        fprintf(stderr,"line: %d\n",currentLine);
        return LEX_ERROR;
    }
    int result = program();


    //cleaning
    destroyStack(&indent_stack);
    
    return result;
}