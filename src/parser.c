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
#include "semantic.h"
#include "symtable.h"


int currentLine = 1;
int inFunDef = 0;
int inFunDefHead = 0;
int paramCount = 0;




int param(){
    fprintf(stderr,"param\n");

    int result;
    paramCount +=1;
    if(inFunDefHead){
        if(token_ptr->type != TOKEN_IDENTIFIER)return SYNTAX_ERROR;

        result = getToken(&token_ptr, &indent_stack );
        return result;
    }else{
        switch(token_ptr->type)
        {
        //pravidlo param -> term
        case TOKEN_IDENTIFIER: 
        case TOKEN_LEFT_BRACKET:
        case TOKEN_INT: 
        case TOKEN_DOUBLE: 
        case TOKEN_STRING:
            result = getToken(&token_ptr, &indent_stack );
            return result;
        break;

        default:
        break;
        }
    }
    
    return SYNTAX_ERROR;
}

int paramList2(){
    fprintf(stderr,"paramList2\n");

    int result;
    
    switch(token_ptr->type)
    {
    //pravidlo paramList2 -> epsilon
    case TOKEN_RIGHT_BRACKET:
        return TOKEN_OK;
    break;
    //pravidlo paramList2 -> , param paramList2
    case TOKEN_COMMA:
        if(token_ptr->type != TOKEN_COMMA)return SYNTAX_ERROR;

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

    return SYNTAX_ERROR;
}

int paramList(){
    fprintf(stderr,"paramList\n");

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
       
    return SYNTAX_ERROR;
}

int funcCall(){
    fprintf(stderr,"funcCall\n");

    int result;
    paramCount = 0;
    //pravidlo funcCall -> id ( paramList )
    if(token_ptr->type != TOKEN_IDENTIFIER)return SYNTAX_ERROR;
    TokenPTR funcName = token_ptr;
    htab_item_t *funcInTable = htab_find(globalSymtable,funcName->dynamic_value);

    if(funcInTable == NULL){
        fprintf(stderr,"Not defined %s\n",funcName->dynamic_value);
        return SEMANTIC_UNDEF_VALUE_ERROR;
    }

    if(funcInTable->isFunc != 1){
        fprintf(stderr,"Not function %s\n",funcName->dynamic_value);
        return SEMANTIC_UNDEF_VALUE_ERROR;
    }

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    if(token_ptr->type != TOKEN_LEFT_BRACKET)return SYNTAX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    result = paramList();
    if(result != TOKEN_OK)return result;
    
    if(paramCount != funcInTable->value){
        fprintf(stderr,"Wrong number of params: %d\n",paramCount);
        return SEMANTIC_WRONG_PARAMETER_NUMBER_ERROR;
    }
    paramCount = 0;

    if(token_ptr->type != TOKEN_RIGHT_BRACKET)return SYNTAX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    return TOKEN_OK;
}

int assigment(){
    fprintf(stderr,"assigment ");
    TokenPTR next_token;
    int result;
    //pravidlo id = neco

    if(token_ptr->type != TOKEN_IDENTIFIER)return SYNTAX_ERROR;
    
    htab_item_t *varInTable = htab_find(globalSymtable,token_ptr->dynamic_value);
    if(varInTable == NULL){
        htab_insert(globalSymtable, token_ptr->dynamic_value, TOKEN_INT,0);
    }else{
        if(varInTable->isFunc == 1){
            fprintf(stderr,"Already defined as function");
            return SEMANTIC_UNDEF_VALUE_ERROR;
        }
    }

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;
    
    if(token_ptr->type != TOKEN_ASSIGN)return SYNTAX_ERROR;
    
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
    
    return SYNTAX_ERROR;
}

int statWithId(){
    fprintf(stderr,"statWithId\n");
    if(token_ptr->type != TOKEN_IDENTIFIER)return SYNTAX_ERROR;

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
    return SYNTAX_ERROR;
}

int stat(){
    fprintf(stderr,"stat\n");
    int result;
    switch(token_ptr->type){
        //pravidlo: Stat -> StatWithId eol
        case TOKEN_IDENTIFIER: 
            result = statWithId();
            if(result != TOKEN_OK)return result;
           
           if(token_ptr->type == TOKEN_DEDENT)return result;
           if(token_ptr->type == TOKEN_EOF)return result;
            if(token_ptr->type != TOKEN_EOL)return SYNTAX_ERROR;
            
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

            if(token_ptr->type == TOKEN_DEDENT)return result;
            if(token_ptr->type == TOKEN_EOF)return result;
            if(token_ptr->type != TOKEN_EOL)return SYNTAX_ERROR;
       
            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            return result;
        break;
        //pravidlo: Stat -> while (Expression) : eol indent Stat StatList dedent
        case KEYWORD_WHILE:
            fprintf(stderr,"while\n");
            if(token_ptr->type != KEYWORD_WHILE)return SYNTAX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            result = expression();
            if(result != TOKEN_OK)return result;
                     
            if(token_ptr->type != TOKEN_COLON)return SYNTAX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            if(token_ptr->type != TOKEN_EOL)return SYNTAX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            if(token_ptr->type != TOKEN_INDENT)return SYNTAX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            result = stat();
            if(result != TOKEN_OK)return result;

            result = statList();
            if(result != TOKEN_OK)return result;

            if(token_ptr->type != TOKEN_DEDENT)return SYNTAX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;
        
            return result;
        break;
        //pravidlo stat -> if expression : eol dedent stat statList indent else : dedent stat statList indent
        case KEYWORD_IF:
            fprintf(stderr,"if\n");
            if(token_ptr->type != KEYWORD_IF)return SYNTAX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            result = expression();
            if(result != TOKEN_OK)return result;
            
            if(token_ptr->type != TOKEN_COLON)return SYNTAX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            if(token_ptr->type != TOKEN_EOL)return SYNTAX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            if(token_ptr->type != TOKEN_INDENT)return SYNTAX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            result = stat();
            if(result != TOKEN_OK)return result;

            result = statList();
            if(result != TOKEN_OK)return result;
            
            if(token_ptr->type != TOKEN_DEDENT)return SYNTAX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;
            
            if(token_ptr->type != KEYWORD_ELSE)return SYNTAX_ERROR;
            fprintf(stderr,"else\n");

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            if(token_ptr->type != TOKEN_COLON)return SYNTAX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            if(token_ptr->type != TOKEN_EOL)return SYNTAX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;
           
            if(token_ptr->type != TOKEN_INDENT)return SYNTAX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;
            
            result = stat();
            if(result != TOKEN_OK)return result;
            
            result = statList();
            if(result != TOKEN_OK)return result;

            if(token_ptr->type != TOKEN_DEDENT)return SYNTAX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            return result;
        break;
        //pravidlo stat -> pass eol
        case KEYWORD_PASS:
            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;
            
            if(token_ptr->type == TOKEN_DEDENT)return TOKEN_OK;
            if(token_ptr->type != TOKEN_EOL)return SYNTAX_ERROR;

            result = getToken(&token_ptr, &indent_stack );          
            return result;
        break;

        case KEYWORD_RETURN:
            if(inFunDef){
                result = getToken(&token_ptr, &indent_stack );
                if(result != TOKEN_OK)return result;
            
                result = expression();
                if(result != TOKEN_OK)return result;

                if(token_ptr->type == TOKEN_DEDENT)return TOKEN_OK;
                if(token_ptr->type != TOKEN_EOL)return SYNTAX_ERROR;

                result = getToken(&token_ptr, &indent_stack );
                if(result != TOKEN_OK)return result;
                
                return result;
            }
        break;

        default:
        break;
    }

    //pravidlo nenalezeno
    return SYNTAX_ERROR;
}

int funcDef(){
    fprintf(stderr,"funcDef\n");
    inFunDef = 1;
    inFunDefHead = 1;
    paramCount = 0;
    int result;

    //pravidlo funcDef -> def id ( paramList ) : eol dedent stat statList indent
    if(token_ptr->type != KEYWORD_DEF)return SYNTAX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    if(token_ptr->type != TOKEN_IDENTIFIER)return SYNTAX_ERROR;

    TokenPTR funcName = token_ptr;
    
    if(htab_find(globalSymtable,funcName->dynamic_value)!= NULL){
        fprintf(stderr,"Redefinition %s\n",funcName->dynamic_value);
        return SEMANTIC_UNDEF_VALUE_ERROR;
    }

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    if(token_ptr->type != TOKEN_LEFT_BRACKET)return SYNTAX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    result = paramList();
    if(result != TOKEN_OK)return result;

    fprintf(stderr,"Param count: %d\n",paramCount);
    htab_insert(globalSymtable,funcName->dynamic_value,paramCount,1);
    paramCount = 0;

    if(token_ptr->type != TOKEN_RIGHT_BRACKET)return SYNTAX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    if(token_ptr->type != TOKEN_COLON)return SYNTAX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    if(token_ptr->type != TOKEN_EOL)return SYNTAX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    inFunDefHead = 0;

    if(token_ptr->type != TOKEN_INDENT)return SYNTAX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    result = stat();
    if(result != TOKEN_OK)return result;

    result = statList();
    if(result != TOKEN_OK)return result;

    if(token_ptr->type != TOKEN_DEDENT)return SYNTAX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;   

    inFunDef = 0;
    return TOKEN_OK;
}


int statList(){
    fprintf(stderr,"statList\n");

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
            return SYNTAX_ERROR;
        break;
    }
    

    //pravidlo nenalezeno   
    return SYNTAX_ERROR;
}

int program(){
    fprintf(stderr,"program\n");
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
            return SYNTAX_ERROR;
        break;
    }
    //pravidlo nenalezeno   
    return SYNTAX_ERROR;
}

int parse(){
    
    //inicilazation
    indent_stack = initStack();
      
    if (indent_stack == NULL)
    {
        fprintf(stderr,"Allocation error.\n");
        return -1;
    }

    if(htab_init(&globalSymtable) == INTERNAL_ERROR)return INTERNAL_ERROR;


    //get first token
    if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
        fprintf(stderr,"line: %d\n",currentLine);
        return LEX_ERROR;
    }
    
    int result = program();


    //cleaning
    destroyStack(&indent_stack);
    htab_free(globalSymtable);


    return result;
}