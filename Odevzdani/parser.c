/**
 * Předmět:  IFJ
 * Projekt:  Implementace překladače imperativního jazyka IFJ19
 * Varianta: Tým 018, varianta II
 * Soubor:   parser.c
 *
 *
 * Datum:    10.12.2019
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

int currentLine = 1; //pocitadlo radku
int inFunDef = 0; //zda se nachazime ve funkci
int inFunDefHead = 0; //zda se nachazime v hlavicce funkce
int paramCount = 0; //pocet nalezenyc parametru


//nastavi promenne v tabulce symbolu do stavu, kdy nebyly pouzity
void unreviewVariables(htab_t* table){
    htab_item_t* item;
    for(int i = 0; i < SIZE; i++){
        item =  table->ptr[i];
        while(item != NULL){
            if(item->type != FUNC){
                item->reviewed = 0;//nastaveni pouziti
            }
            item = item->next;
        }
    }
}

//zkontroluje zda je funkce definovana vcetne funkci, ktere vola
int checkCompleteDefinition(htab_item_t* func){
    if(func->defined == 0){
        //pokud nebyla definovana vraci 0
        fprintf(stderr,"Function %s not defined\n",func->key);
        return 0;
    }

    if(func->reviewed == 1)return 1;
    func->reviewed = 1;

    htab_t *funcCalls = func->local_vars;
    if(funcCalls == NULL)return 1;
    htab_item_t* call;
    htab_item_t* def;

    //kontrola funkci, ktere pouziva
    //prochazi lokalni tabulku symbolu
    for(int i = 0; i < SIZE; i++){
        call =  funcCalls->ptr[i];
        while(call != NULL){
            if(call->type == FUNC){

                def = htab_find(globalSymtable,call->key);//nalezeni pouzite funkce v globalni tabulce

                if(def == NULL){
                    fprintf(stderr,"function %s not defind\n",call->key);
                    return 0;
                }

                if(def->type != FUNC){
                    fprintf(stderr,"%s not function\n",def->key);
                }

                if(checkCompleteDefinition(def) == 0)return 0;//nalezenou funkci take zkontroluje
            }

            call = call->next;
        }
    }

    return 1;
}

//na konci programu zkontroluje zda jsou vsechny funkce definovane
int checkAllDefinitions(htab_t* table){

    htab_item_t* func;
    
    //prochazeni tabulky
    for(int i = 0; i < SIZE; i++){
        func =  table->ptr[i];
        while(func != NULL){
            if(func->type == FUNC){ //kontrolujeme pouze funkce
                if(func == NULL){
                    fprintf(stderr,"function %s not defind\n",func->key);
                    return 0;
                }
                if(func->type != FUNC){
                    fprintf(stderr,"%s not function\n",func->key);
                }
                if(checkCompleteDefinition(func) == 0)return 0;//funkce je zkontrolovana
            }

            func = func->next;
        }
    }
    return 1;
}

int param(){
    
    htab_item_t* identifier;
    htab_item_t* constValue;
    int result;
    paramCount +=1;
    if(inFunDefHead){
        //pravidlo <param> -> id

        //kontrola definice
        if(token_ptr->type != TOKEN_IDENTIFIER)return SYNTAX_ERROR;
        if(htab_insert(localSymtable,token_ptr->dynamic_value,UNKNOWN,LF,0,0,1) == INTERNAL_ERROR)return INTERNAL_ERROR;
        identifier = htab_find(localSymtable,token_ptr->dynamic_value);
        
        //definice lokalni promenne
        //if(generate_instr(&list, DEFVAR,1,identifier) == INTERNAL_ERROR)return INTERNAL_ERROR;
        if(generate_before_whiles(&list, identifier) == INTERNAL_ERROR)return INTERNAL_ERROR;
        
        //propojeni lokalni promenne s odpovidajicim parametrem
        htab_item_t* param = get_param(paramCount-1);
        if(param == NULL)return INTERNAL_ERROR;
        if(generate_instr(&list,MOVE,2,identifier,param)==INTERNAL_ERROR)return INTERNAL_ERROR;

        result = getToken(&token_ptr, &indent_stack );
        return result;
    }else{
        switch(token_ptr->type)
        {
        //pravidlo <param> -> <term>
        case TOKEN_IDENTIFIER:

            if(inFunDef){
                identifier = htab_find(localSymtable,token_ptr->dynamic_value);
                if(identifier != NULL){
                    //nalezena v lokalni tabulce
                    if(identifier->type == FUNC){
                        fprintf(stderr,"%s already used as function call\n",token_ptr->dynamic_value);
                        return SEMANTIC_UNDEF_VALUE_ERROR;
                    }
                }else{
                    //hleda se v globalni tabulce
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
                //hledame pouze v globalni tabulce
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
            //zaslani parametru generatoru
            send_param(identifier);

            result = getToken(&token_ptr, &indent_stack );
            return result;
        break;
        case TOKEN_INT:
            //<param> -> int
            constValue = make_const(token_ptr->dynamic_value,INT);
            if(constValue == NULL)return INTERNAL_ERROR;
            constValue->ival = token_ptr->number_value;
            send_param(constValue);
            result = getToken(&token_ptr, &indent_stack );
            return result;

        case TOKEN_DOUBLE:
            //<param> -> double
            constValue = make_const(token_ptr->dynamic_value,FLOAT);
            if(constValue == NULL)return INTERNAL_ERROR;
            constValue->dval = token_ptr->number_value;
            send_param(constValue);
            result = getToken(&token_ptr, &indent_stack );
            return result;
        case TOKEN_STRING:
            //<param> -> string
            constValue = make_const(token_ptr->dynamic_value,STRING);
            if(constValue == NULL)return INTERNAL_ERROR;
            constValue->sval = token_ptr->dynamic_value;
            send_param(constValue);
            result = getToken(&token_ptr, &indent_stack );
            return result;
        break;
        case KEYWORD_NONE:
            //<param> -> none
            constValue = make_const(token_ptr->dynamic_value,NIL);
            if(constValue == NULL)return INTERNAL_ERROR;
            constValue->sval = "nil";
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
    int result;

    switch(token_ptr->type)
    {
    //pravidlo <paramList2> -> epsilon
    case TOKEN_RIGHT_BRACKET:
        return TOKEN_OK;
    break;
    //pravidlo <paramList2> -> , <param> <paramList2>
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
    int result;

    switch(token_ptr->type)
    {
    //pravidlo <paramList> -> epsilon
    case TOKEN_RIGHT_BRACKET:
        return TOKEN_OK;
    break;
    //pravidlo <paramList> -> <param> <paramList2>
    case TOKEN_IDENTIFIER:
    case TOKEN_LEFT_BRACKET:
    case TOKEN_INT:
    case TOKEN_DOUBLE:
    case TOKEN_STRING:
    case KEYWORD_NONE:
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
    int result;
    paramCount = 0;
    //pravidlo funcCall -> id ( <paramList> )
    if(token_ptr->type != TOKEN_IDENTIFIER)return SYNTAX_ERROR;
    TokenPTR funcName = token_ptr;
    htab_item_t *funcInGlobalTable = htab_find(globalSymtable,funcName->dynamic_value);
    htab_item_t* funcInLocalTable = NULL;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    if(token_ptr->type != TOKEN_LEFT_BRACKET)return SYNTAX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    result = paramList();
    if(result != TOKEN_OK)return result;

    if(inFunDef == 0){
        //pokud nejsme ve funkci musime zkontrolovat definici funkce
        if(funcInGlobalTable == NULL){
            fprintf(stderr,"Not defined %s\n",funcName->dynamic_value);
            return SEMANTIC_UNDEF_VALUE_ERROR;
        }
        if(funcInGlobalTable->type != FUNC){
            fprintf(stderr,"Not function %s\n",funcName->dynamic_value);
            return SEMANTIC_UNDEF_VALUE_ERROR;
        }

        if(checkCompleteDefinition(funcInGlobalTable) == 0){
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
        if(funcInGlobalTable == NULL){
            //pridani do gloalni tabulky
            htab_insert(globalSymtable,funcName->dynamic_value,FUNC,GF,0,1,0);

            funcInGlobalTable = htab_find(globalSymtable,funcName->dynamic_value);
            if(funcInGlobalTable == NULL)return INTERNAL_ERROR;
            //zapiseme pocet paramatru, s kterymi byla volana
            funcInGlobalTable->ival = paramCount;

            }
            //vlozeni volani do lokalni tabulky
            if(htab_insert(localSymtable,funcName->dynamic_value,FUNC,LF,0,1,0)==INTERNAL_ERROR){

                return INTERNAL_ERROR;
            }
         if(funcInGlobalTable->type != FUNC){
            fprintf(stderr,"Not function %s\n",funcName->dynamic_value);
            return SEMANTIC_UNDEF_VALUE_ERROR;
        }
    }
    //kontrola spravneho poctu parametru
    // hodnota -2 znaci libovolny pocet parametru
    if(paramCount != funcInGlobalTable->ival && funcInGlobalTable->ival != -2){
        fprintf(stderr,"Wrong number of params in %s: %d\n",funcInGlobalTable->key,paramCount);
        return SEMANTIC_WRONG_PARAMETER_NUMBER_ERROR;
    }

    paramCount = 0;

    if(token_ptr->type != TOKEN_RIGHT_BRACKET)return SYNTAX_ERROR;

    //ukonceni volani funkce v generatoru
    if(func_call(&list,funcInGlobalTable)==INTERNAL_ERROR)return INTERNAL_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    return TOKEN_OK;
}

int assignment(){
    
    TokenPTR next_token;
    int result;
    htab_item_t expressionResult;
    int created = 0;

    //pravidlo id = neco

    if(token_ptr->type != TOKEN_IDENTIFIER)return SYNTAX_ERROR;

    htab_item_t *varInGlobalTable;
    htab_item_t *varInLocalTable;
    varInGlobalTable = htab_find(globalSymtable,token_ptr->dynamic_value);;

    if(inFunDef){
        varInLocalTable = htab_find(localSymtable,token_ptr->dynamic_value);
        if(varInGlobalTable == NULL && varInLocalTable == NULL){
            //definice nove promenne 
            htab_insert(localSymtable, token_ptr->dynamic_value,UNKNOWN,LF,0,0,1);
            created = 1;
            varInLocalTable = htab_find(localSymtable,token_ptr->dynamic_value);
            if(varInLocalTable == NULL)return INTERNAL_ERROR;
        }else if(varInGlobalTable != NULL && varInLocalTable == NULL){
            //kontrola redefinic
            if(varInGlobalTable->type == FUNC){
                fprintf(stderr,"Already defined as function\n");
                return SEMANTIC_UNDEF_VALUE_ERROR;
            }
            if(varInGlobalTable->reviewed > 1){
                fprintf(stderr,"Already used as global variable in this function\n");
                return SEMANTIC_UNDEF_VALUE_ERROR;
            }
            //definice nove promenne
            htab_insert(localSymtable, token_ptr->dynamic_value, UNKNOWN,LF,0,0,1);
            created = 1;
            varInLocalTable = htab_find(localSymtable,token_ptr->dynamic_value);
            if(varInLocalTable == NULL)return INTERNAL_ERROR;
        }else if(varInGlobalTable == NULL && varInLocalTable != NULL){
            //kontrola redefinic
            if(varInLocalTable->type == FUNC){
                fprintf(stderr,"Function call with same identifier\n");
                return SEMANTIC_UNDEF_VALUE_ERROR;
            }
        }
    }else{
        if(varInGlobalTable != NULL){
            //kontrola redefinic
            if(varInGlobalTable->type == FUNC){
                fprintf(stderr,"Already defined as function\n");
                return SEMANTIC_UNDEF_VALUE_ERROR;
            }
        }else{
            //vytvoreni nove promenne
            htab_insert(globalSymtable, token_ptr->dynamic_value, UNKNOWN,GF,0,0,1);
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
        //prirazeni vyrazu
        result = expression(&expressionResult);
        if(result != TOKEN_OK)return result;
         //kontrola typu
        if(expressionResult.type == BOOL){
            fprintf(stderr,"Cant assign bool to variable\n");
            return SEMANTIC_TYPE_COMPATIBILITY_ERROR;
        }
        if(inFunDef){
            varInLocalTable->type = expressionResult.type;
            if(created){
                //vytvoreni promenne
                if(generate_before_if(&list,varInLocalTable)==INTERNAL_ERROR)return INTERNAL_ERROR;
            }
            if(generate_instr(&list,MOVE,2,varInLocalTable, &expressionResult)==INTERNAL_ERROR)return INTERNAL_ERROR;//prirazeni hodnoty
        }else{
            varInGlobalTable->type = expressionResult.type;
            if(created){
                //vytvoreni promenne
                if(generate_before_if(&list,varInGlobalTable)==INTERNAL_ERROR)return INTERNAL_ERROR;
            }
            if(generate_instr(&list,MOVE,2,varInGlobalTable, &expressionResult)==INTERNAL_ERROR)return INTERNAL_ERROR;//prirazeni hodnoty
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
                if(created){
                    //vytvoreni promenne
                    if(generate_before_if(&list,varInLocalTable)==INTERNAL_ERROR)return INTERNAL_ERROR;
                }
                if(generate_save_return_value(&list, varInLocalTable)==INTERNAL_ERROR)return INTERNAL_ERROR;//prirazeni navratove hodnoty                
            }else{
                varInGlobalTable->type = UNKNOWN;
                if(created){
                    //vytvoreni promenne
                    if(generate_before_if(&list,varInGlobalTable)==INTERNAL_ERROR)return INTERNAL_ERROR;
                }
                if(generate_save_return_value(&list, varInGlobalTable)==INTERNAL_ERROR)return INTERNAL_ERROR;//prirazeni navratove hodnoty
            }
            return result;
        }else{
            //prirazujeme vyraz
            result = expression(&expressionResult);
            //kontrola navratoveho typu
            if(expressionResult.type == BOOL){
                fprintf(stderr,"Cant assign bool to variable\n");
                return SEMANTIC_TYPE_COMPATIBILITY_ERROR;
            }
            if(inFunDef){
                varInLocalTable->type = expressionResult.type;
                 if(created){
                //vytvoreni promenne
                if(generate_before_if(&list,varInLocalTable)==INTERNAL_ERROR)return INTERNAL_ERROR;
            }
            if(generate_instr(&list,MOVE,2,varInLocalTable, &expressionResult)==INTERNAL_ERROR)return INTERNAL_ERROR;//prirazeni hodnoty
            }else{
                varInGlobalTable->type = expressionResult.type;
                if(created){
                //vytvoreni promenne
                if(generate_before_if(&list,varInGlobalTable)==INTERNAL_ERROR)return INTERNAL_ERROR;
            }
            if(generate_instr(&list,MOVE,2,varInGlobalTable, &expressionResult)==INTERNAL_ERROR)return INTERNAL_ERROR;//prirazeni hodnoty
            }
            return result;
        }
    }

    return SYNTAX_ERROR;
}

int statWithId(){
    
    if(token_ptr->type != TOKEN_IDENTIFIER)return SYNTAX_ERROR;

    TokenPTR next_token;
    int result;
    htab_item_t expressionResult;

    result = preloadToken(&next_token, &indent_stack );
    if(result != TOKEN_OK)return result;

    switch (next_token->type)
    {
        //volani funkce
        case TOKEN_LEFT_BRACKET:
            result = funcCall();

            return result;
        break;

        //prirazeni do promenne
        case TOKEN_ASSIGN:
            result = assignment();
            return result;
        break;
        
        //jedna se o vyraz
        default:
            result = expression(&expressionResult);
            return result;
        break;
    }
    //pravidlo nenalezeno
    return SYNTAX_ERROR;
}

int stat(){
    int result;
    htab_item_t expressionResult;
    switch(token_ptr->type){
        //pravidlo: <stat> -> <statWithId> eol
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

        //pravidlo: <stat> -> <expression> eol
        case TOKEN_LEFT_BRACKET:
        case TOKEN_INT:
        case TOKEN_DOUBLE:
        case TOKEN_STRING:
        case KEYWORD_NONE:
            result = expression(&expressionResult);
            
            if(result != TOKEN_OK)return result;

            if(token_ptr->type == TOKEN_DEDENT)return result;
            if(token_ptr->type == TOKEN_EOF)return result;
            if(token_ptr->type != TOKEN_EOL)return SYNTAX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            return result;
        break;
        //pravidlo: <stat> -> while <xpression> : eol indent <stat> <statList> dedent
        case KEYWORD_WHILE:

            if(token_ptr->type != KEYWORD_WHILE)return SYNTAX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            //nagenerovani pocatku cyklu
            if(generate_while_start(&list)==INTERNAL_ERROR)return INTERNAL_ERROR;
            

            result = expression(&expressionResult);
            if(result != TOKEN_OK)return result;
            
            //nagenerovani podminky
            if(generate_condition_check(&list, &expressionResult,1)==INTERNAL_ERROR)return INTERNAL_ERROR;

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

            //nagenerovani konce cyklu
            generate_while_end(&list);
            
            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            return result;
        break;
        //pravidlo <stat> -> if <expression> : eol dedent <stat> <statList> indent else : dedent <stat> <statList> indent
        case KEYWORD_IF:
            
            if(token_ptr->type != KEYWORD_IF)return SYNTAX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            //nagenerovani pocatku if
            if(start_if_else(&list)==INTERNAL_ERROR)return INTERNAL_ERROR;

            result = expression(&expressionResult);
            if(result != TOKEN_OK)return result;

            //nagenerovani kontroly podminky
            if(generate_condition_check(&list, &expressionResult,0)==INTERNAL_ERROR)return INTERNAL_ERROR;

            if(token_ptr->type != TOKEN_COLON)return SYNTAX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            if(token_ptr->type != TOKEN_EOL)return SYNTAX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            if(token_ptr->type != TOKEN_INDENT)return SYNTAX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;
            
            //nagenerovani vetve if
            generate_if(&list);

            result = stat();
            if(result != TOKEN_OK)return result;

            result = statList();
            if(result != TOKEN_OK)return result;

            if(token_ptr->type != TOKEN_DEDENT)return SYNTAX_ERROR;

            result = getToken(&token_ptr, &indent_stack );
            if(result != TOKEN_OK)return result;

            if(token_ptr->type != KEYWORD_ELSE)return SYNTAX_ERROR;

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
            
            //nagenerovani vetve else
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
            if(inFunDef){//return nemuze existovat mimo funkci
                result = getToken(&token_ptr, &indent_stack );
                if(result != TOKEN_OK)return result;
                //return
                if(token_ptr->type == TOKEN_DEDENT || token_ptr->type == TOKEN_EOF){
                    //nagenerovani return
                    if(generate_return(&list)==INTERNAL_ERROR)return INTERNAL_ERROR;
                    return TOKEN_OK;
                }


                if(token_ptr->type == TOKEN_EOL){
                    //nagenerovani return
                    if(generate_return(&list)==INTERNAL_ERROR)return INTERNAL_ERROR;

                    result = getToken(&token_ptr, &indent_stack );
                    if(result != TOKEN_OK)return result;
                    return TOKEN_OK;
                }

                //return expression

                result = expression(&expressionResult);
                if(result != TOKEN_OK)return result;

                //ulozi vysledek vyrazu do navratove hodnoty funkce
                if(generate_save_to_return(&list,&expressionResult)==INTERNAL_ERROR)return INTERNAL_ERROR;
                //nagenerovani return
                if(generate_return(&list)==INTERNAL_ERROR)return INTERNAL_ERROR;

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
    
    inFunDef = 1;
    inFunDefHead = 1;
    paramCount = 0;
    int result;

    //pravidlo <funcDef> -> def id ( <paramList> ) : eol dedent <stat> <statList> indent
    if(token_ptr->type != KEYWORD_DEF)return SYNTAX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    if(token_ptr->type != TOKEN_IDENTIFIER)return SYNTAX_ERROR;

    TokenPTR funcName = token_ptr;
    htab_item_t* func = htab_find(globalSymtable,funcName->dynamic_value);
    int alreadyCalled = 0;

    if(func!= NULL){
        //kontrola redefinici
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
        //vlozeni do globalni tabulky
        htab_insert(globalSymtable,funcName->dynamic_value,FUNC,GF,0,1,1);

        func = htab_find(globalSymtable,funcName->dynamic_value);
        if(func == NULL)return INTERNAL_ERROR;
        func->ival = paramCount;

    }
    
    //inicializace lokalni tabulky
    htab_init(&(func->local_vars));
        if(func->local_vars == NULL)return INTERNAL_ERROR;
        localSymtable = func->local_vars;

    //zacatek funkce v generatoru
    if(generate_func_start(&list, func)==INTERNAL_ERROR)return INTERNAL_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    if(token_ptr->type != TOKEN_LEFT_BRACKET)return SYNTAX_ERROR;

    result = getToken(&token_ptr, &indent_stack );
    if(result != TOKEN_OK)return result;

    result = paramList();
    if(result != TOKEN_OK)return result;

    if(alreadyCalled){
        //kontrola zda nebyla volana s jinym poctem parametru
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

    if(generate_func_end(&list)==INTERNAL_ERROR)return INTERNAL_ERROR;

    inFunDef = 0;
    localSymtable = NULL;
    return TOKEN_OK;
}


int statList(){
    
    int result;
    switch(token_ptr->type){
        //pravidlo:     <statList> -> <stat> <statList>
        case TOKEN_IDENTIFIER:
        case TOKEN_LEFT_BRACKET:
        case KEYWORD_PASS:
        case TOKEN_INT:
        case TOKEN_DOUBLE:
        case TOKEN_STRING:
        case KEYWORD_WHILE:
        case KEYWORD_IF:
        case KEYWORD_RETURN:
        case KEYWORD_NONE:
            result = stat();
            if(result != TOKEN_OK)return result;

            result = statList();

            return result;
        break;

        //<statList> -> epsilon
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
    int result;
   
    switch(token_ptr->type){
        //pravidlo:     <program> -> eof
        case TOKEN_EOF:
            if(checkAllDefinitions(globalSymtable)){
                return TOKEN_OK;
            }
            return SEMANTIC_UNDEF_VALUE_ERROR;
        break;

        //pravidlo:     <program> -> <statList> <program>
        case TOKEN_IDENTIFIER:
        case TOKEN_LEFT_BRACKET:
        case KEYWORD_PASS:
        case TOKEN_INT:
        case TOKEN_DOUBLE:
        case TOKEN_STRING:
        case KEYWORD_WHILE:
        case KEYWORD_IF:
        case KEYWORD_NONE:
            result = statList();
            if(result != TOKEN_OK)return result;

            result = program();
            return result;
        break;

        //pravidlo:     <program> -> <funcDef> <program>
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

    //inicializace 
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
    if(generator_start(&list)==INTERNAL_ERROR)return INTERNAL_ERROR;


    //nahraje prvni token
    if(getToken(&token_ptr, &indent_stack) == LEX_ERROR){
        return LEX_ERROR;
    }
    
    //simulace pocatecniho neterminalu
    int result = program();

    if(result == TOKEN_OK){
        printInstructions(&list);
    }else{
        fprintf(stderr,"ERROR CODE: %d\n",result);
    }

    //uvolnovani pameti
    destroyStack(&indent_stack);
    htab_free(globalSymtable);
    DisposeList(&list);

    return result;
}
