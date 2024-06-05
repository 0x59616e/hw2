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
%type <object_val> Expression LogicOrExpression LogicAndExpression BitwiseOrExpression BitwiseXorExpression BitwiseAndExpression EquExpression RelationalExpression ShiftExpression AdditiveExpression MultiExpression CastExpression UnaryExpression LogicalNotExpression FuncCallExpression PrimaryExpression ScalarExpression LvalueExpression



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
    : DefineVariableStmtList ';'
    | FunctionDefStmt
;

/* Function */
FunctionDefStmt
    : VARIABLE_T IDENT { createFunction($<var_type>1, $<s_var>2); } '(' { pushScope(); } FunctionParameterStmtList ')' {genFunctionJNI($<var_type>1, $<s_var>2);} '{' StmtList '}' { dumpScope(); }
;
FunctionParameterStmtList 
    : FunctionParameterStmtList ',' FunctionParameterStmt
    | FunctionParameterStmt
    | /* Empty function parameter */
;
FunctionParameterStmt
    : VARIABLE_T IDENT { pushFunctionParm($<var_type>1, $<s_var>2, VAR_FLAG_DEFAULT); }
    | VARIABLE_T IDENT '[' ']' { pushFunctionParm($<var_type>1, $<s_var>2, VAR_FLAG_ARRAY); }
;

/* Scope */
StmtList 
    : StmtList Stmt
    | Stmt
;
Stmt
    : ';' {printf(";;;;\n");}
    | GeneralStmt ';'
    | AssignmentStmt ';'
    | IfStmt
    | DefineVariableStmt ';'
    | WhileStmt
    | ForStmt
;

IfStmt
    : IF '(' Expression ')' { puts("IF"); pushScope(); } '{' StmtList '}' { dumpScope(); } ElseStmt
    | IF '(' Expression ')' { puts("IF"); } Stmt
;

ElseStmt
    : ELSE { puts("ELSE"); pushScope(); } '{' StmtList '}' { dumpScope();}
    | /* Empty else */
;

WhileStmt
    : WHILE { puts("WHILE"); } '(' Expression ')' { pushScope(); } '{' StmtList '}' { dumpScope();}
;

ForStmt
    : FOR { puts("FOR"); pushScope(); } '(' ForForeStmt ')' '{' StmtList '}' { dumpScope();}

ForForeStmt
    : DefineVariableStmt ':' Expression {
      defineVariableHelper($3.type);
    }
    | ForInitStmt ';' Expression ';' AssignmentStmt

ForInitStmt
    : DefineVariableStmt
    | AssignmentStmt
    | 
;

DefineVariableStmt
    : VARIABLE_T DefineVariableStmtList { defineVariableHelper($<var_type>1); };

DefineVariableStmtList
    : DefineVariableStmtList ',' RPARTDefineVariableStmt
    | RPARTDefineVariableStmt


RPARTDefineVariableStmt
    : IDENT { insertVariable($<s_var>1, OBJECT_TYPE_UNDEFINED); }
    | IDENT VAL_ASSIGN Expression { insertVariable($<s_var>1, $3.type); }
    | IDENT '[' ScalarExpression ']' '[' ScalarExpression ']' {
      insertVariable($<s_var>1, OBJECT_TYPE_UNDEFINED);
    }
    | IDENT '[' ScalarExpression ']' VAL_ASSIGN '{' ExpressionList '}' {
      int cnt = 0;
      ObjectType type = OBJECT_TYPE_UNDEFINED;
      Object *obj = NULL;
      while (obj = popExpression()) {
        type = obj->type > type ? obj->type : type;
        cnt++;
      }
      printf("create array: %d\n", cnt);
      insertVariable($<s_var>1, type);
    }
;

GeneralStmt
    : COUT CoutParmListStmt { stdoutPrint(); }
    | RETURN Expression { puts("RETURN"); }
    | RETURN { puts("RETURN"); }
    | BREAK { puts("BREAK"); }
    | CONTINUE { puts("CONTINUE"); }
    | FuncCallExpression
;

CoutParmListStmt
    : CoutParmListStmt SHL Expression { pushFunInParm(&$<object_val>3); }
    | SHL Expression { pushFunInParm(&$<object_val>2); }
;


AssignmentStmt
    : LvalueExpression VAL_ASSIGN Expression { printf("EQL_ASSIGN\n"); }
    | LvalueExpression ADD_ASSIGN Expression { printf("ADD_ASSIGN\n"); }
    | LvalueExpression SUB_ASSIGN Expression { printf("SUB_ASSIGN\n"); }
    | LvalueExpression MUL_ASSIGN Expression { printf("MUL_ASSIGN\n"); }
    | LvalueExpression DIV_ASSIGN Expression { printf("DIV_ASSIGN\n"); }
    | LvalueExpression REM_ASSIGN Expression { printf("REM_ASSIGN\n"); }
    | LvalueExpression SHR_ASSIGN Expression { printf("SHR_ASSIGN\n"); }
    | LvalueExpression SHL_ASSIGN Expression { printf("SHL_ASSIGN\n"); }
    | LvalueExpression BAN_ASSIGN Expression { printf("BAN_ASSIGN\n"); }
    | LvalueExpression BOR_ASSIGN Expression { printf("BOR_ASSIGN\n"); }
    | LvalueExpression BXO_ASSIGN Expression { printf("BXO_ASSIGN\n"); }
    | LvalueExpression INC_ASSIGN { printf("INC_ASSIGN\n"); }
    | LvalueExpression DEC_ASSIGN { printf("DEC_ASSIGN\n"); }
;

/* expression */

ExpressionList
    : ExpressionList ',' Expression { pushExpression(&$3); }
    | Expression { pushExpression(&$1); }
    | /* Empty expression list */
;

Expression 
    : LogicOrExpression { $$ = $1; }
;

LogicOrExpression
    : LogicOrExpression LOR LogicAndExpression { puts("LOR"); $$ = $3; }
    | LogicAndExpression { $$ = $1; }
;

LogicAndExpression
    : LogicAndExpression LAN BitwiseOrExpression { puts("LAN"); $$ = $3; }
    | BitwiseOrExpression { $$ = $1; }
;

BitwiseOrExpression
    : BitwiseOrExpression BOR BitwiseXorExpression { puts("BOR"); $$ = $3; }
    | BitwiseXorExpression { $$ = $1; }
;

BitwiseXorExpression
    : BitwiseXorExpression BXO BitwiseAndExpression { puts("BXO"); $$ = $3; }
    | BitwiseAndExpression { $$ = $1; }
;

BitwiseAndExpression
    : BitwiseAndExpression BAN EquExpression { puts("BAN"); $$ = $3; }
    | EquExpression { $$ = $1; }
;

EquExpression
    : EquExpression EQL RelationalExpression { puts("EQL"); $$ = $3; }
    | EquExpression NEQ RelationalExpression { puts("NEQ"); $$ = $3; }
    | RelationalExpression { $$ = $1; }
;

RelationalExpression
    : RelationalExpression GTR ShiftExpression { puts("GTR"); $$ = $3; }
    | RelationalExpression LES ShiftExpression { puts("LES"); $$ = $3; }
    | RelationalExpression GEQ ShiftExpression { puts("GEQ"); $$ = $3; }
    | RelationalExpression LEQ ShiftExpression { puts("LEQ"); $$ = $3; }
    | ShiftExpression { $$ = $1; }
;

ShiftExpression
    : ShiftExpression SHR AdditiveExpression { puts("SHR"); $$ = $3; }
    | AdditiveExpression { $$ = $1; }

AdditiveExpression
    : AdditiveExpression ADD MultiExpression { puts("ADD"); $$ = $3; }
    | AdditiveExpression SUB MultiExpression { puts("SUB"); $$ = $3; }
    | MultiExpression { $$ = $1; }
;

MultiExpression
    : MultiExpression MUL CastExpression { puts("MUL"); $$ = $3; }
    | MultiExpression DIV CastExpression { puts("DIV"); $$ = $3; }
    | MultiExpression REM CastExpression { puts("REM"); $$ = $3; }
    | CastExpression { $$ = $1; }
;

CastExpression
    : '(' VARIABLE_T ')' UnaryExpression {
      castingVariableHelper($<var_type>2);
      $$.type = $<var_type>2;
    }
    | UnaryExpression { $$ = $1; }
;

UnaryExpression
    : SUB UnaryExpression { puts("NEG"); $$ = $2; }
    | NOT UnaryExpression { puts("NOT"); $$ = $2; }
    | LogicalNotExpression { $$ = $1; }
;

LogicalNotExpression
    : BNT LogicalNotExpression { puts("BNT"); $$ = $2; }
    | BAN LogicalNotExpression { puts("BAN"); $$ = $2; }
    | FuncCallExpression { $$ = $1; }
    | PrimaryExpression { $$ = $1; }
;

FuncCallExpression
    : IDENT '(' ExpressionList ')' {
      Object *obj = findVariable($1);
      printf("IDENT (name=%s, address=%d)\n", $1, obj->symbol->addr);
      printf("call: %s%s\n", obj->symbol->name, obj->symbol->func_sig);
      $$.type = obj->tmpType;
    }
;

PrimaryExpression
    : '(' Expression ')' { $$ = $2; }
    | ScalarExpression
    | STR_LIT { printf("STR_LIT \"%s\"\n", $1); $$.type = OBJECT_TYPE_STR; }
    | LvalueExpression
;

ScalarExpression
    : INT_LIT {
      printf("INT_LIT %d\n", $1);
      $$.type = OBJECT_TYPE_INT; 
      $$.value = *(uint64_t*)&$1;
    }
    | FLOAT_LIT {
      printf("FLOAT_LIT %f\n", $1);
      $$.type = OBJECT_TYPE_FLOAT;
      $$.value = *(uint64_t*)&$1;
    }
    | BOOL_LIT {
      printf("BOOL_LIT %s\n", $1 ? "TRUE" : "FALSE");
      $$.type = OBJECT_TYPE_BOOL;
      $$.value = *(uint64_t*)&$1;
    }
;

LvalueExpression
    : IDENT { 
      Object *obj = findVariable($1);
      if (obj == NULL) {
        printf("IDENT (name=%s, address=-1)\n", $1);
        $$.type = OBJECT_TYPE_STR;
      } else {
        printf("IDENT (name=%s, address=%d)\n", $1, obj->symbol->addr);
        $$.type = obj->type;
      }
    }
    | IDENT '[' Expression ']' {
      Object *obj = findVariable($1);
      printf("IDENT (name=%s, address=%d)\n", $1, obj->symbol->addr);
      $$.type = obj->type;
    }
    | IDENT '[' Expression ']' '[' Expression ']' { 
      Object *obj = findVariable($1);
      printf("IDENT (name=%s, address=%d)\n", $1, obj->symbol->addr);
      $$.type = obj->type;
    }
%%
/* C code section */
