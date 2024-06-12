#ifndef MAIN_H
#define MAIN_H
#include "compiler_common.h"

extern FILE *yyin;
extern bool compileError;
int yyparse();
int yylex();
int yylex_destroy();

#define VAR_FLAG_DEFAULT 0
#define VAR_FLAG_ARRAY 0b00000001
#define VAR_FLAG_POINTER 0b00000010

void pushScope();
void dumpScope();

void pushMainFunctionParm();

void pushExpression(Object *out);
Object *popExpression();
void setCurrentInitVarType(ObjectType variableType);
void addVarToSymbolTable(char *variableName, ObjectType variableType);
void addVarToSymbolTable(char *variableName);
void pushFunInParm(Object *variable);
void createFunction(ObjectType variableType, char *funcName);
void createMainFunction();

Object *findVariable(char *variableName);
Object *createVariable(ObjectType variableType, char *variableName,
                       int variableFlag);

void dumpToStdout();

#endif
