/**
 * Předmět:  IFJ
 * Projekt:  Implementace překladače imperativního jazyka IFJ19
 * Varianta: Tým 018, varianta II
 * Soubor:   errors.h
 *
 *
 * Datum:    10.12.2019
 *
 * Autoři:   Matej Hockicko  <xhocki00@stud.fit.vutbr.cz>
 *           Tomáš Julina    <xjulin08@stud.fit.vutbr.cz>
 *           Tomáš Kantor    <xkanto14@stud.fit.vutbr.cz>
 *           Lukáš Kuchta	 <xkucht09@stud.fit.vutbr.cz>
 */


#define LEX_ERROR 1 // lexikální chyba (scanner)
#define SYNTAX_ERROR 2 // syntaktická chyba (parser)
#define SEMANTIC_UNDEF_VALUE_ERROR 3 // nedefinovaná funkce/proměnná, pokus o redefinici funkce
#define SEMANTIC_TYPE_COMPATIBILITY_ERROR 4 // chyba typové kompatibility v aritmetických/řetězcových operacích
#define SEMANTIC_WRONG_PARAMETER_NUMBER_ERROR 5 // špatný počet parametrů u volání funkcí
#define SEMANTIC_OTHER_ERROR 6 // ostatní sémantické operace
#define RUNTIME_DIVIDE_BY_ZERO_ERROR 9 // běhová chyba dělení nulou
#define INTERNAL_ERROR 99 // chyba alokace
