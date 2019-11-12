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
int inFunDef = 0;

int statList();

int expression(){
    printf("expression\n");
    int result;
    //sezere tri tokeny bez syntakticke kontroly
    
    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    return TOKEN_OK;
}

int param(){
    printf("param\n");

    int result;

    if(token_ptr->type != TOKEN_IDENTIFIER)return SYNATX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    return result;
    /*
    switch(token_ptr->type)
    {
    //pravidlo param -> expression
    case TOKEN_IDENTIFIER: 
    case TOKEN_LEFT_BRACKET:
    case TOKEN_INT: 
    case TOKEN_DOUBLE: 
    case TOKEN_STRING:
        result = expression();

        return  result;
    break;

    default:
    break;
    }
*/
    return SYNATX_ERROR;
}

int paramList2(){
    printf("paramList2\n");

    int result;
    
    switch(token_ptr->type)
    {
    //pravidlo paramList2 -> epsilon
    case TOKEN_RIGHT_BRACKET:
        return TOKEN_OK;
    break;
    //pravidlo paramList2 -> , param paramList2
    case TOKEN_COMMA:
        if(token_ptr->type != TOKEN_COMMA)return SYNATX_ERROR;

        result = getToken(&token_ptr, &indent_stack );
        if(result != TOKEN_OK)return result;

        result = param();
        if(result != TOKEN_OK)return result;

        result = paramList2();
        if(result != TOKEN_OK)return result;

        return TOKEN_OK;
    break;

    default:
    break;
    }

    return SYNATX_ERROR;
}

int paramList(){
    printf("paramList\n");

    int result;
    
    switch(token_ptr->type)
    {
    //pravidlo paramList -> epsilon
    case TOKEN_RIGHT_BRACKET:
        return TOKEN_OK;
    break;
    //pravidlo paramList -> param paramList2
    case TOKEN_IDENTIFIER: 
    case TOKEN_LEFT_BRACKET:
    case TOKEN_INT: 
    case TOKEN_DOUBLE: 
    case TOKEN_STRING:
        result = param();
        if(result != TOKEN_OK)return result;

        result = paramList2();
        if(result != TOKEN_OK)return result;

        return TOKEN_OK;
    break;

    default:
    break;
    }
       
    return SYNATX_ERROR;
}

int funcCall(){
    printf("funcCall\n");

    int result;

    //pravidlo funcCall -> id ( paramList )
    if(token_ptr->type != TOKEN_IDENTIFIER)return SYNATX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    if(token_ptr->type != TOKEN_LEFT_BRACKET)return SYNATX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    result = paramList();
    if(result != TOKEN_OK)return result;
    
    
    if(token_ptr->type != TOKEN_RIGHT_BRACKET)return SYNATX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    return TOKEN_OK;
}

int assigment(){
    printf("assigment ");
    TokenPTR next_token;
    int result;
    //pravidlo id = neco

    if(token_ptr->type != TOKEN_IDENTIFIER)return SYNATX_ERROR;
    
    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;
    
    if(token_ptr->type != TOKEN_ASSIGN)return SYNATX_ERROR;
    
    //vime id =
    // zjistime, co prirazujeme
    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    if(token_ptr->type != TOKEN_IDENTIFIER){
        result = expression();
        return result;
    }else{
        result = preloadToken(&next_token, &indent_stack );
        if(result != TOKEN_OK)return result;
        //prirazujeme vysledek volani fce
        if(next_token->type == TOKEN_LEFT_BRACKET){
            result = funcCall();
            return result;
        }else{
            //prirazujeme vyraz
            result = expression();
            
            return result;
        }
    }
    
    return SYNATX_ERROR;
}

int statWithId(){
    printf("statWithId\n");
    if(token_ptr->type != TOKEN_IDENTIFIER)return SYNATX_ERROR;

    TokenPTR next_token;
    int result;

    result = preloadToken(&next_token, &indent_stack );
    if(result != TOKEN_OK)return result;

    switch (next_token->type)
    {
        //volani funkce
        case TOKEN_LEFT_BRACKET:
            result = funcCall();

            return result;
        break;

        case TOKEN_ASSIGN:
            result = assigment();
            return result;
        break;
        
        default:
            result = expression();

            return result;
        break;
    }
    //pravidlo nenalezeno
    return SYNATX_ERROR;
}

int stat(){
    printf("stat\n");
    int result;
    switch(token_ptr->type){
        //pravidlo: Stat -> StatWithId eol
        case TOKEN_IDENTIFIER: 
            result = statWithId();
            if(result != TOKEN_OK)return result;
           
            if(token_ptr->type != TOKEN_EOL)return SYNATX_ERROR;
            
            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            return result;
        break;

        //pravidlo: Stat -> Expression eol
        case TOKEN_LEFT_BRACKET:
        case TOKEN_INT: 
        case TOKEN_DOUBLE: 
        case TOKEN_STRING: 
            result = expression();
            if(result != TOKEN_OK)return result;

            if(token_ptr->type != TOKEN_EOL)return SYNATX_ERROR;
       
            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            return result;
        break;
        //pravidlo: Stat -> while (Expression) : eol indent Stat StatList dedent
        case KEYWORD_WHILE:
            printf("while\n");
            if(token_ptr->type != KEYWORD_WHILE)return SYNATX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            result = expression();
            if(result != TOKEN_OK)return result;
            
            
            if(token_ptr->type != TOKEN_COLON)return SYNATX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            if(token_ptr->type != TOKEN_EOL)return SYNATX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            if(token_ptr->type != TOKEN_INDENT)return SYNATX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            result = stat();
            if(result != TOKEN_OK)return result;

            result = statList();
            if(result != TOKEN_OK)return result;

            if(token_ptr->type != TOKEN_DEDENT)return SYNATX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;
        
            return result;
        break;
        //pravidlo stat -> if expression : eol dedent stat statList indent else : dedent stat statList indent
        case KEYWORD_IF:
            printf("if\n");
            if(token_ptr->type != KEYWORD_IF)return SYNATX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            result = expression();
            if(result != TOKEN_OK)return result;
            
            if(token_ptr->type != TOKEN_COLON)return SYNATX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            if(token_ptr->type != TOKEN_EOL)return SYNATX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            if(token_ptr->type != TOKEN_INDENT)return SYNATX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            result = stat();
            if(result != TOKEN_OK)return result;

            result = statList();
            if(result != TOKEN_OK)return result;
            
            if(token_ptr->type != TOKEN_DEDENT)return SYNATX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;
            
            if(token_ptr->type != KEYWORD_ELSE)return SYNATX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            if(token_ptr->type != TOKEN_COLON)return SYNATX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            if(token_ptr->type != TOKEN_EOL)return SYNATX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;
           
            if(token_ptr->type != TOKEN_INDENT)return SYNATX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;
            
            result = stat();
            if(result != TOKEN_OK)return result;
            
            result = statList();
            if(result != TOKEN_OK)return result;

            if(token_ptr->type != TOKEN_DEDENT)return SYNATX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            return result;
        break;
        //pravidlo stat -> pass eol
        case KEYWORD_PASS:
            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;
            
            if(token_ptr->type != TOKEN_EOL)return SYNATX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            return result;
        break;

        case KEYWORD_RETURN:
            if(inFunDef){
                result = getToken(&token_ptr, &indent_stack );
                if(result != TOKEN_OK)return result;
            
                result = expression();
                if(result != TOKEN_OK)return result;

                if(token_ptr->type != TOKEN_EOL)return SYNATX_ERROR;

                result = getToken(&token_ptr, &indent_stack );
                if(result != TOKEN_OK)return result;

                return result;
            }
        break;

        default:
        break;
    }

    //pravidlo nenalezeno
    return SYNATX_ERROR;
}

int funcDef(){
    printf("funcDef\n");
    inFunDef = 1;
    int result;

    //pravidlo funcDef -> def id ( paramList ) : eol dedent stat statList indent
    if(token_ptr->type != KEYWORD_DEF)return SYNATX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    if(token_ptr->type != TOKEN_IDENTIFIER)return SYNATX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    if(token_ptr->type != TOKEN_LEFT_BRACKET)return SYNATX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    result = paramList();
    if(result != TOKEN_OK)return result; 
    
    if(token_ptr->type != TOKEN_RIGHT_BRACKET)return SYNATX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    if(token_ptr->type != TOKEN_COLON)return SYNATX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    if(token_ptr->type != TOKEN_EOL)return SYNATX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    if(token_ptr->type != TOKEN_INDENT)return SYNATX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    result = stat();
    if(result != TOKEN_OK)return result;

    result = statList();
    if(result != TOKEN_OK)return result;

    if(token_ptr->type != TOKEN_DEDENT)return SYNATX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;   

    inFunDef = 0;
    return TOKEN_OK;
}


int statList(){
    printf("statList\n");

    int result;
    
    switch(token_ptr->type){
        //pravidlo:     StatList -> Stat StatList
        case TOKEN_IDENTIFIER: 
        case TOKEN_LEFT_BRACKET: 
        case KEYWORD_PASS: 
        case TOKEN_INT: 
        case TOKEN_DOUBLE: 
        case TOKEN_STRING: 
        case KEYWORD_WHILE: 
        case KEYWORD_IF:
        case KEYWORD_RETURN:
            result = stat();
            if(result != TOKEN_OK)return result;

            result = statList();

            return result;
        break;

        case TOKEN_DEDENT:
        case TOKEN_EOF:
        case KEYWORD_DEF:
            return TOKEN_OK;
        break;
        //pravidlo nenalezeno   
        default:
            return SYNATX_ERROR;
        break;
    }
    

    //pravidlo nenalezeno   
    return SYNATX_ERROR;
}

int program(){
    printf("program\n");
    int result;
    
    switch(token_ptr->type){
        //pravidlo:     Program -> eof
        case TOKEN_EOF:
            return TOKEN_OK;
        break;

        //pravidlo:     Program -> StatList Program
        case TOKEN_IDENTIFIER: 
        case TOKEN_LEFT_BRACKET: 
        case KEYWORD_PASS: 
        case TOKEN_INT: 
        case TOKEN_DOUBLE: 
        case TOKEN_STRING: 
        case KEYWORD_WHILE: 
        case KEYWORD_IF: 
            result = statList();
            if(result != TOKEN_OK)return result;

            result = program();
            return result;
        break;

        case KEYWORD_DEF: 
            result = funcDef();
            if(result != TOKEN_OK)return result;

            result = program();
            return result;
        break;

        default:
            return SYNATX_ERROR;
        break;
    }
    //pravidlo nenalezeno   
    return SYNATX_ERROR;
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