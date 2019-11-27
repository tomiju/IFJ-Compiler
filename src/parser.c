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
#include "expression.h"
#include "symtable.h"
#include "generator.h"

int currentLine = 1;
int inFunDef = 0;
int inFunDefHead = 0;
int paramCount = 0;
tList list;



void unreviewVariables(htab_t* table){
    htab_item_t* item;
    for(int i = 0; i < SIZE; i++){
        item =  table->ptr[i];
        while(item != NULL){
            if(item->type != FUNC){
                item->reviewed = 0;
            }
            
            item = item->next;
        }
    }
}

int checkCompleteDefinition(htab_item_t* func){
    if(func->defined == 0){
        fprintf(stderr,"Function %s not defined\n",func->key);
        return 0;
    }
    if(func->reviewed == 1)return 1;
    func->reviewed = 1;

    htab_t *funcCalls = func->local_vars;
    if(funcCalls == NULL)return 1;
    htab_item_t* call;
    htab_item_t* def;
    //fprintf(stderr,"here %s\n", func->key);

    for(int i = 0; i < SIZE; i++){
        call =  funcCalls->ptr[i];
        while(call != NULL){
            if(call->type == FUNC){
                def = htab_find(globalSymtable,call->key);
                if(def == NULL){
                    fprintf(stderr,"function %s not defind\n",def->key);
                    return 0;
                }
                if(def->type != FUNC){
                    fprintf(stderr,"%s not function\n",def->key);
                }
                if(checkCompleteDefinition(def) == 0)return 0;
            }
            
            call = call->next;
        }
    }

    return 1;
}

int checkAllDefinitions(htab_t* table){
    
    htab_item_t* func;
    //fprintf(stderr,"here %s\n", func->key);

    for(int i = 0; i < SIZE; i++){
        func =  table->ptr[i];
        while(func != NULL){
            if(func->type == FUNC){
                if(func == NULL){
                    fprintf(stderr,"function %s not defind\n",func->key);
                    return 0;
                }
                if(func->type != FUNC){
                    fprintf(stderr,"%s not function\n",func->key);
                }
                if(checkCompleteDefinition(func) == 0)return 0;
            }
            
            func = func->next;
        }
    }
    return 1;
}

int param(){
    fprintf(stderr,"param\n");
    htab_item_t* identifier;
    htab_item_t* constValue;
    int result;
    paramCount +=1;
    if(inFunDefHead){
        if(token_ptr->type != TOKEN_IDENTIFIER)return SYNTAX_ERROR;
        if(htab_insert(localSymtable,token_ptr->dynamic_value,UNKNOWN,LF,0,0,1) == INTERNAL_ERROR)return INTERNAL_ERROR;

        generate_instr(&list, DEFVAR,1,localSymtable);
        generate_instr(&list,MOVE,2,localSymtable,get_param(paramCount-1));

        result = getToken(&token_ptr, &indent_stack );
        return result;
    }else{
        switch(token_ptr->type)
        {
        //pravidlo param -> term
        case TOKEN_IDENTIFIER:
            
            if(inFunDef){
                identifier = htab_find(localSymtable,token_ptr->dynamic_value);
                if(identifier != NULL){
                    if(identifier->type == FUNC){
                        fprintf(stderr,"%s already used as function call\n",token_ptr->dynamic_value);
                        return SEMANTIC_UNDEF_VALUE_ERROR;
                    }
                }else{
                    identifier = htab_find(globalSymtable,token_ptr->dynamic_value);
                    if(identifier == NULL){
                        fprintf(stderr,"%s not defined\n",token_ptr->dynamic_value);
                        return SEMANTIC_UNDEF_VALUE_ERROR;
                    }
                     if(identifier->type == FUNC){
                        fprintf(stderr,"%s is function\n",token_ptr->dynamic_value);
                        return SEMANTIC_UNDEF_VALUE_ERROR;
                    }

                }
            }else{
                identifier = htab_find(globalSymtable,token_ptr->dynamic_value);          
                if(identifier == NULL){
                    fprintf(stderr,"Undefined %s\n",token_ptr->dynamic_value);
                    return SEMANTIC_UNDEF_VALUE_ERROR;
                }
                if(identifier->type == FUNC){
                    fprintf(stderr,"Cant use function %s in param\n",token_ptr->dynamic_value);
                    return SEMANTIC_UNDEF_VALUE_ERROR;
                }
                
            }
            send_param(identifier);
            result = getToken(&token_ptr, &indent_stack );
            return result;
        break;
        case TOKEN_INT:
            constValue = make_const(token_ptr->dynamic_value,INT);
            constValue->ival = token_ptr->number_value;
            send_param(constValue);
            result = getToken(&token_ptr, &indent_stack );
            return result;

        case TOKEN_DOUBLE:
            constValue = make_const(token_ptr->dynamic_value,FLOAT);
            constValue->dval = token_ptr->number_value;
            send_param(constValue);
            result = getToken(&token_ptr, &indent_stack );
            return result;
        case TOKEN_STRING:
            constValue = make_const(token_ptr->dynamic_value,STRING);
            constValue->sval = token_ptr->dynamic_value;
            send_param(constValue);
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
    htab_item_t* funcInLocalTable;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    if(token_ptr->type != TOKEN_LEFT_BRACKET)return SYNTAX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    result = paramList();
    if(result != TOKEN_OK)return result;

    if(inFunDef == 0){
        if(funcInTable == NULL){
            fprintf(stderr,"Not defined %s\n",funcName->dynamic_value);
            return SEMANTIC_UNDEF_VALUE_ERROR;
        }
        if(funcInTable->type != FUNC){
            fprintf(stderr,"Not function %s\n",funcName->dynamic_value);
            return SEMANTIC_UNDEF_VALUE_ERROR;
        }
        if(checkCompleteDefinition(funcInTable) == 0){
            fprintf(stderr,"Not completely defined %s\n",funcName->dynamic_value);
            return SEMANTIC_UNDEF_VALUE_ERROR;
        }
    }else{
        funcInLocalTable = htab_find(localSymtable,funcName->dynamic_value);
        if(funcInLocalTable != NULL){
            if(funcInLocalTable->type != FUNC){
                fprintf(stderr,"%s is used as local variable\n",funcName->dynamic_value);
                return SEMANTIC_UNDEF_VALUE_ERROR;
            }
        }
        if(funcInTable == NULL){        
            htab_insert(globalSymtable,funcName->dynamic_value,FUNC,GF,0,1,0);

            funcInTable = htab_find(globalSymtable,funcName->dynamic_value);
            if(funcInTable == NULL)return INTERNAL_ERROR;
            funcInTable->ival = paramCount;
            
            }
            if(htab_insert(localSymtable,funcName->dynamic_value,FUNC,LF,0,1,0)==INTERNAL_ERROR){
                
                return INTERNAL_ERROR;
            }
         if(funcInTable->type != FUNC){
            fprintf(stderr,"Not function %s\n",funcName->dynamic_value);
            return SEMANTIC_UNDEF_VALUE_ERROR;
        }
    }
    if(paramCount != funcInTable->ival && funcInTable->ival != -2){
        fprintf(stderr,"Wrong number of params in %s: %d\n",funcInTable->key,paramCount);
        return SEMANTIC_WRONG_PARAMETER_NUMBER_ERROR;
    }

    paramCount = 0;

    if(token_ptr->type != TOKEN_RIGHT_BRACKET)return SYNTAX_ERROR;

    func_call(&list,funcInTable);
    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    return TOKEN_OK;
}

int assignment(){
    fprintf(stderr,"assignment ");
    TokenPTR next_token;
    int result;
    TokenTYPE expressionType = UNKNOWN;
    int created = 0;
    //pravidlo id = neco

    if(token_ptr->type != TOKEN_IDENTIFIER)return SYNTAX_ERROR;
    
    htab_item_t *varInGlobalTable;
    htab_item_t *varInLocalTable;
    varInGlobalTable = htab_find(globalSymtable,token_ptr->dynamic_value);;
    
    if(inFunDef){
        varInLocalTable = htab_find(localSymtable,token_ptr->dynamic_value);
        if(varInGlobalTable == NULL && varInLocalTable == NULL){
            htab_insert(localSymtable, token_ptr->dynamic_value,expressionType,LF,0,0,1);
            created = 1;
            varInLocalTable = htab_find(localSymtable,token_ptr->dynamic_value);
            if(varInLocalTable == NULL)return INTERNAL_ERROR;
        }else if(varInGlobalTable != NULL && varInLocalTable == NULL){
            if(varInGlobalTable->type == FUNC){
                fprintf(stderr,"Already defined as function\n");
                return SEMANTIC_UNDEF_VALUE_ERROR;
            }
            if(varInGlobalTable->reviewed > 1){
                fprintf(stderr,"Already used as global variable in this function\n");
                return SEMANTIC_UNDEF_VALUE_ERROR;
            }
            htab_insert(localSymtable, token_ptr->dynamic_value, expressionType,LF,0,0,1);
            created = 1;
            varInLocalTable = htab_find(localSymtable,token_ptr->dynamic_value);
            if(varInLocalTable == NULL)return INTERNAL_ERROR;      
        }else if(varInGlobalTable == NULL && varInLocalTable != NULL){
            if(varInLocalTable->type == FUNC){
                fprintf(stderr,"Function call with same identifier\n");
                return SEMANTIC_UNDEF_VALUE_ERROR;
            }
        }
    }else{
        if(varInGlobalTable != NULL){
            if(varInGlobalTable->type == FUNC){
                fprintf(stderr,"Already defined as function\n");
                return SEMANTIC_UNDEF_VALUE_ERROR;
            }
        }else{
            htab_insert(globalSymtable, token_ptr->dynamic_value, expressionType,GF,0,0,1);
            created = 1;
            varInGlobalTable = htab_find(globalSymtable,token_ptr->dynamic_value);
            if(varInGlobalTable == NULL)return INTERNAL_ERROR;
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
        result = expression(&expressionType);
        fprintf(stderr,"Type: %d\n",expressionType);
        if(expressionType == BOOL){
            fprintf(stderr,"Cant assign bool to variable\n");
            return SEMANTIC_TYPE_COMPATIBILITY_ERROR;
        }
        if(inFunDef){
            varInLocalTable->type = expressionType;
            if(created)generate_instr(&list, DEFVAR,1,varInLocalTable);
        }else{
            varInGlobalTable->type = expressionType;
             if(created)generate_instr(&list, DEFVAR,1,varInGlobalTable);
        }
        return result;
    }else{
        result = preloadToken(&next_token, &indent_stack );
        if(result != TOKEN_OK)return result;
        //prirazujeme vysledek volani fce
        if(next_token->type == TOKEN_LEFT_BRACKET){
            result = funcCall();
            if(inFunDef){
                varInLocalTable->type = UNKNOWN;
                if(created)generate_instr(&list, DEFVAR,1,varInLocalTable);
                generate_save_return_value(&list, varInLocalTable);
            }else{
                varInGlobalTable->type = UNKNOWN;
                if(created)generate_instr(&list, DEFVAR,1,varInGlobalTable);
                generate_save_return_value(&list, varInGlobalTable);
            }
            return result;
        }else{
            //prirazujeme vyraz
            result = expression(&expressionType);
            
            if(expressionType == BOOL){
                fprintf(stderr,"Cant assign bool to variable\n");
                return SEMANTIC_TYPE_COMPATIBILITY_ERROR;
            }
            if(inFunDef){
                varInLocalTable->type = expressionType;
                 if(created)generate_instr(&list, DEFVAR,1,varInLocalTable);
            }else{
                varInGlobalTable->type = expressionType;
                 if(created)generate_instr(&list, DEFVAR,1,varInGlobalTable);
            }
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
    TokenTYPE expressionType;

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
            result = assignment();
            return result;
        break;
        
        default:
            result = expression(&expressionType);

            return result;
        break;
    }
    //pravidlo nenalezeno
    return SYNTAX_ERROR;
}

int stat(){
    fprintf(stderr,"stat\n");
    int result;
    TokenTYPE expressionType;
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
            result = expression(&expressionType);
            fprintf(stderr,"result: %d\n",result);
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

            generate_while_start(&list);
            

            result = expression(&expressionType);
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

            generate_while_end(&list);
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

            start_if_else(&list);
            //TODO
            //generate_condition(&list);

            result = expression(&expressionType);
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

            generate_if(&list);

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

            generate_else(&list);

            result = stat();
            if(result != TOKEN_OK)return result;
            
            result = statList();
            if(result != TOKEN_OK)return result;

            if(token_ptr->type != TOKEN_DEDENT)return SYNTAX_ERROR;

            end_if_else(&list);

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
            fprintf(stderr,"return");
            if(inFunDef){
                result = getToken(&token_ptr, &indent_stack );
                if(result != TOKEN_OK)return result;
                //return
                if(token_ptr->type == TOKEN_DEDENT)return TOKEN_OK;
                if(token_ptr->type == TOKEN_EOF)return TOKEN_OK;

                if(token_ptr->type == TOKEN_EOL){
                    result = getToken(&token_ptr, &indent_stack );
                    if(result != TOKEN_OK)return result;
                    return TOKEN_OK;
                }
                
                //return expression
                
                result = expression(&expressionType);
                
                if(result != TOKEN_OK)return result;
                
                if(token_ptr->type == TOKEN_DEDENT)return TOKEN_OK;
                if(token_ptr->type == TOKEN_EOF)return TOKEN_OK;
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
    htab_item_t* func = htab_find(globalSymtable,funcName->dynamic_value);
    int alreadyCalled = 0;
    
    if(func!= NULL){
        if(func->type != FUNC){
            fprintf(stderr,"%s Already defined as variable\n",funcName->dynamic_value);
            return SEMANTIC_UNDEF_VALUE_ERROR;
        }
        if(func->defined == 1){
            fprintf(stderr,"Redefinition %s\n",funcName->dynamic_value);
            return SEMANTIC_UNDEF_VALUE_ERROR;
        }
        alreadyCalled = 1;
        
    }else{

        htab_insert(globalSymtable,funcName->dynamic_value,FUNC,GF,0,1,1);
        
        func = htab_find(globalSymtable,funcName->dynamic_value);
        if(func == NULL)return INTERNAL_ERROR;
        func->ival = paramCount;
        
    }
    htab_init(&(func->local_vars));
        if(func->local_vars == NULL)return INTERNAL_ERROR;
        localSymtable = func->local_vars;


    generate_func_start(&list, func);

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    if(token_ptr->type != TOKEN_LEFT_BRACKET)return SYNTAX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    result = paramList();
    if(result != TOKEN_OK)return result;

    fprintf(stderr,"Param count: %d\n",paramCount);
    
        
    if(alreadyCalled){
        if(func->ival != paramCount){
            fprintf(stderr,"Function %s was called with different number of params\n",func->key);
            return SEMANTIC_WRONG_PARAMETER_NUMBER_ERROR;
        }
        func->defined = 1;
    }else{
        func->ival = paramCount;
    }
    unreviewVariables(globalSymtable);
    
    

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

    generate_func_end(&list);

    inFunDef = 0;
    localSymtable = NULL;
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
            if(checkAllDefinitions(globalSymtable)){
                return TOKEN_OK;
            }
            return SEMANTIC_UNDEF_VALUE_ERROR;
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

    if(htab_init(&globalSymtable) == INTERNAL_ERROR){
        destroyStack(&indent_stack);
        return INTERNAL_ERROR;
    }
    localSymtable = NULL;
    
    
    
    InitList(&list);
    generator_start(&list);
    //get first token
    if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
        return LEX_ERROR;
    }
    //TokenPTR firstToken = token_ptr;

    int result = program();

    //printing instruction is very dangerous prints many instructions
    //printInstructions(&list);
    //cleaning
    destroyStack(&indent_stack);
    htab_free(globalSymtable);
    DisposeList(&list);

    return result;
}