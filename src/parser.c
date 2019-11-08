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
 *           Lukáš Kuchta   <xkucht09@stud.fit.vutbr.cz>
 */

#include "scanner.h"
#include "parser.h"


iStack indent_stack; //required by scanner
TokenPTR token_ptr; //pointer to the token

int paramList2(){
    int result = TOKEN_OK;

    if(token_ptr->type != TOKEN_COMMA){
        return SYNATX_ERROR;
    }

    if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
        return LEX_ERROR;
    }
    //---------------------------------------------------

    if(token_ptr->type != TOKEN_IDENTIFIER){
        return SYNATX_ERROR;
    }

    if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
        return LEX_ERROR;
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

    return SYNATX_ERROR;
}

int paramList(){
    int result = TOKEN_OK;

    if(token_ptr->type == TOKEN_RIGHT_BRACKET){
        return TOKEN_OK; 
    }

    if(token_ptr->type != TOKEN_IDENTIFIER){
        return SYNATX_ERROR; 
    }

    if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
        return LEX_ERROR;
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
    int result = TOKEN_OK;

    if(token_ptr->type != KEYWORD_DEF){
        return SYNATX_ERROR; 
    }
    
    if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
        return LEX_ERROR;
    }
    //---------------------------------------------------
    if(token_ptr->type != TOKEN_IDENTIFIER){
        return SYNATX_ERROR; 
    }
    
    if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
        return LEX_ERROR;
    }
    //---------------------------------------------------

    if(token_ptr->type != TOKEN_LEFT_BRACKET){
        return SYNATX_ERROR; 
    }

    if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
        return LEX_ERROR;
    }

    //---------------------------------------------------
    if(token_ptr->type == TOKEN_IDENTIFIER){
        result = paramList();
        if(result != TOKEN_OK)return result;
    }
    //---------------------------------------------------
    

    if(token_ptr->type != TOKEN_RIGHT_BRACKET){
        return SYNATX_ERROR; 
    }

    if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
        return LEX_ERROR;
    }
    //---------------------------------------------------

    if(token_ptr->type != TOKEN_COLON){
        return SYNATX_ERROR;
    }

    
    //---------------------------------------------------
    //TODO body
    //---------------------------------------------------

    return result;
}

int program(){
    int result;
    if(token_ptr->type == KEYWORD_DEF){
        result = funcDef();
    }else{
        return SYNATX_ERROR;
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
        return LEX_ERROR;
    }
    int result = program();


    //cleaning
    destroyStack(&indent_stack);
    
    return result;
}