/* Definition section */
%{
    #include "compiler_common.h"
    #include "compiler_util.h"
    #include "main.h"

    int yydebug = 1;
%}

/* Variable or self-defined structure */
%union {
    ObjectType var_type;

    bool b_var;
    int i_var;
    float f_var;
    char *s_var;

    Object object_val;
}

/* Token without return */
%token COUT ENDL
%token SHR SHL BAN BOR BNT BXO ADD SUB MUL DIV REM NOT GTR LES GEQ LEQ EQL NEQ LAN LOR
%token VAL_ASSIGN ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN REM_ASSIGN BAN_ASSIGN BOR_ASSIGN BXO_ASSIGN SHR_ASSIGN SHL_ASSIGN INC_ASSIGN DEC_ASSIGN
%token IF ELSE FOR WHILE RETURN BREAK CONTINUE

/* Token with return, which need to sepcify type */
%token <var_type> VARIABLE_T
%token <s_var> IDENT
%token <b_var> BOOL_LIT
%token <i_var> INT_LIT
%token <f_var> FLOAT_LIT
%token <s_var> STR_LIT

/* Nonterminal with return, which need to sepcify type */
%type <object_val> expression logical_or_expression logical_and_expression bitwise_or_expression bitwise_xor_expression bitwise_and_expression equality_expression relational_expression shift_expression additive_expression multiplicative_expression unary_expression logical_not_expression primary_expression



%left ADD SUB
%left MUL DIV REM

/* Yacc will start at this nonterminal */
%start Program

%%
/* Grammar section */

Program
    : { pushScope(); } GlobalStmtList { dumpScope(); }
    | /* Empty file */
;

GlobalStmtList 
    : GlobalStmtList GlobalStmt
    | GlobalStmt
;

GlobalStmt
    : declaration ';'
    | FunctionDefStmt
;

/* Function */
FunctionDefStmt
    : VARIABLE_T IDENT { createMainFunction(); } '(' { pushScope(); } parameter ')' compound_statement { dumpScope(); }
;

parameter
    : VARIABLE_T IDENT '[' ']' { pushMainFunctionParm(); }
;

compound_statement
    : '{' '}'
    | '{' block_item_list '}'
;

block_item_list
    : block_item
    | block_item_list block_item
;

block_item
    : statement
;

/* Scope */

statement
    : ';'
    | COUT CoutParmListStmt { dumpToStdout(); }
    | RETURN expression { puts("RETURN"); }
    | RETURN { puts("RETURN"); }
    | assignment_expression ';'
    | declaration ';'
;

declaration
    : VARIABLE_T { setCurrentInitVarType($<var_type>1); } init_declaration_list;

init_declaration_list
    : init_declaration_list ',' init_declarator
    | init_declarator


init_declarator
    : IDENT { addVarToSymbolTable($<s_var>1); }
    | IDENT VAL_ASSIGN expression { addVarToSymbolTable($<s_var>1, $3.type); }
;


CoutParmListStmt
    : CoutParmListStmt SHL expression { pushFunInParm(&$<object_val>3); }
    | SHL expression { pushFunInParm(&$<object_val>2); }
;


assignment_expression
    : unary_expression VAL_ASSIGN expression { printf("EQL_ASSIGN\n"); }
    | unary_expression ADD_ASSIGN expression { printf("ADD_ASSIGN\n"); }
    | unary_expression SUB_ASSIGN expression { printf("SUB_ASSIGN\n"); }
    | unary_expression MUL_ASSIGN expression { printf("MUL_ASSIGN\n"); }
    | unary_expression DIV_ASSIGN expression { printf("DIV_ASSIGN\n"); }
    | unary_expression REM_ASSIGN expression { printf("REM_ASSIGN\n"); }
    | unary_expression SHR_ASSIGN expression { printf("SHR_ASSIGN\n"); }
    | unary_expression SHL_ASSIGN expression { printf("SHL_ASSIGN\n"); }
    | unary_expression BAN_ASSIGN expression { printf("BAN_ASSIGN\n"); }
    | unary_expression BOR_ASSIGN expression { printf("BOR_ASSIGN\n"); }
    | unary_expression BXO_ASSIGN expression { printf("BXO_ASSIGN\n"); }
    | unary_expression INC_ASSIGN { printf("INC_ASSIGN\n"); }
    | unary_expression DEC_ASSIGN { printf("DEC_ASSIGN\n"); }
;

/* expression */

expressions
    : expressions ',' expression { pushExpression(&$3); }
    | expression { pushExpression(&$1); }
    |
;

expression 
    : logical_or_expression { $$ = $1; }
;

logical_or_expression
    : logical_or_expression LOR logical_and_expression { puts("LOR"); $$ = $3; }
    | logical_and_expression { $$ = $1; }
;

logical_and_expression
    : logical_and_expression LAN bitwise_or_expression { puts("LAN"); $$ = $3; }
    | bitwise_or_expression { $$ = $1; }
;

bitwise_or_expression
    : bitwise_or_expression BOR bitwise_xor_expression { puts("BOR"); $$ = $3; }
    | bitwise_xor_expression { $$ = $1; }
;

bitwise_xor_expression
    : bitwise_xor_expression BXO bitwise_and_expression { puts("BXO"); $$ = $3; }
    | bitwise_and_expression { $$ = $1; }
;

bitwise_and_expression
    : bitwise_and_expression BAN equality_expression { puts("BAN"); $$ = $3; }
    | equality_expression { $$ = $1; }
;

equality_expression
    : equality_expression EQL relational_expression { puts("EQL"); $$ = $3; }
    | equality_expression NEQ relational_expression { puts("NEQ"); $$ = $3; }
    | relational_expression { $$ = $1; }
;

relational_expression
    : relational_expression GTR shift_expression { puts("GTR"); $$ = $3; }
    | relational_expression LES shift_expression { puts("LES"); $$ = $3; }
    | relational_expression GEQ shift_expression { puts("GEQ"); $$ = $3; }
    | relational_expression LEQ shift_expression { puts("LEQ"); $$ = $3; }
    | shift_expression { $$ = $1; }
;

shift_expression
    : shift_expression SHR additive_expression { puts("SHR"); $$ = $3; }
    | additive_expression { $$ = $1; }

additive_expression
    : additive_expression ADD multiplicative_expression { puts("ADD"); $$ = $3; }
    | additive_expression SUB multiplicative_expression { puts("SUB"); $$ = $3; }
    | multiplicative_expression { $$ = $1; }
;

multiplicative_expression
    : multiplicative_expression MUL unary_expression { puts("MUL"); $$ = $3; }
    | multiplicative_expression DIV unary_expression { puts("DIV"); $$ = $3; }
    | multiplicative_expression REM unary_expression { puts("REM"); $$ = $3; }
    | unary_expression { $$ = $1; }
;

unary_expression
    : SUB unary_expression { puts("NEG"); $$ = $2; }
    | NOT unary_expression { puts("NOT"); $$ = $2; }
    | logical_not_expression { $$ = $1; }
;

logical_not_expression
    : BNT unary_expression { puts("BNT"); $$ = $2; }
    | BAN unary_expression { puts("BAN"); $$ = $2; }
    | primary_expression { $$ = $1; }
;

primary_expression
    : '(' expression ')' { $$ = $2; }
    | INT_LIT {
      printf("INT_LIT %d\n", $1);
      $$.type = OBJECT_TYPE_INT; 
      $$.value = *(unsigned long long*)&$1;
    }
    | FLOAT_LIT {
      printf("FLOAT_LIT %f\n", $1);
      $$.type = OBJECT_TYPE_FLOAT;
      $$.value = *(unsigned long long*)&$1;
    }
    | BOOL_LIT {
      printf("BOOL_LIT %s\n", $1 ? "TRUE" : "FALSE");
      $$.type = OBJECT_TYPE_BOOL;
      $$.value = *(unsigned long long*)&$1;
    }
    | STR_LIT { printf("STR_LIT \"%s\"\n", $1); $$.type = OBJECT_TYPE_STR; }
    | IDENT { 
      Object *obj = findVariable($1);
      if (obj == NULL) {
        printf("IDENT (name=%s, address=-1)\n", $1);
        $$.type = OBJECT_TYPE_STR;
      } else {
        printf("IDENT (name=%s, address=%d)\n", $1, obj->symbol->addr);
        $$.type = obj->type;
      }
    }
;


%%
/* C code section */
