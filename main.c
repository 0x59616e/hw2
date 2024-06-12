#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler_common.h"
#include "main.h"

using namespace std;

const char *typeSpecifier[] = {
    "undefined", "auto",   "void", "char",   "int",      "long",
    "float",     "double", "bool", "string", "function",
};

typedef struct {
  Object *objects[100] = {NULL};
  int32_t size = 0;
} SymbolTable;

typedef struct {
  void *data[2000];
  int head = 0;
  int tail = 0;
} Queue;

SymbolTable symbolTable[100];

Queue stdoutQueue;
Queue functionParmQueue;

/* queue<Object *> expressionQueue; */
typedef struct ObjectPair {
  ObjectType type;
  bool isArray;
} ObjectPair;
/* queue<pair<ObjectType, bool>> functionParmQueue; */

char *yyInputFileName;
bool compileError;

int indent = 0;
int scopeLevel = -1;
int funcLineNo = 0;
int variableAddress = 0;
ObjectType variableIdentType;

void pushMainFunctionParm() {
  functionParmQueue.data[functionParmQueue.tail++] =
      new ObjectPair{OBJECT_TYPE_FUNCTION, true};
  addVarToSymbolTable((char *)"argv", OBJECT_TYPE_STR);
}

ObjectType currentInitVarType;

void setCurrentInitVarType(ObjectType variableType) {
  currentInitVarType = variableType;
  for (int i = 0; i < symbolTable[scopeLevel].size; i++) {
    Object *x = symbolTable[scopeLevel].objects[i];
    if (x->type == OBJECT_TYPE_UNDEFINED) {
      x->type = variableType;
    }
  }
}

void addVarToSymbolTable(char *variableName, ObjectType variableType) {
  SymbolData *symbolData = new SymbolData{
      .name = variableName,
      .index = symbolTable[scopeLevel].size,
      .addr = !scopeLevel ? -1 : variableAddress,
      .lineno = yylineno,
      .func_sig = new char[50],
      .func_var = 0,
  };
  symbolData->func_sig[0] = '-';
  symbolData->func_sig[1] = '\0';
  Object *object = new Object{
      .type = variableType,
      .tmpType = variableType,
      .value = 0,
      .symbol = symbolData,
  };

  symbolTable[scopeLevel].objects[symbolTable[scopeLevel].size++] = object;

  printf("> Insert `%s` (addr: %d) to scope level %d\n", variableName,
         !scopeLevel ? -1 : variableAddress++, scopeLevel);
}

void pushScope() {
  printf("> Create symbol table (scope level %d)\n", ++scopeLevel);
}

void dumpScope() {
  printf("\n");
  printf("> Dump symbol table (scope level: %d)\n", scopeLevel);
  printf("Index     Name                Type      Addr      Lineno    Func_sig "
         " \n");
  for (int i = 0; i < symbolTable[scopeLevel].size; i++) {
    Object *x = symbolTable[scopeLevel].objects[i];
    printf("%-9d %-19s %-9s %-9ld %-9d %-10s\n", x->symbol->index,
           x->symbol->name, typeSpecifier[x->type], x->symbol->addr,
           x->symbol->lineno, x->symbol->func_sig);
  }

  symbolTable[scopeLevel].size = 0;

  scopeLevel--;
}

void createMainFunction() {
  printf("func: main\n");
  addVarToSymbolTable((char *)"main", OBJECT_TYPE_FUNCTION);
  Object *tmp = symbolTable[0].objects[symbolTable[0].size - 1];
  tmp->tmpType = OBJECT_TYPE_FUNCTION;
  char *funcProto = (char *)"([Ljava/lang/String;)I";
  strcpy(tmp->symbol->func_sig, funcProto);
}

Object *findVariable(char *variableName) {
  for (int i = scopeLevel; i >= 0; i--) {
    for (int j = 0; j < symbolTable[i].size; j++) {
      Object *x = symbolTable[i].objects[j];
      if (strcmp(x->symbol->name, variableName) == 0) {
        return x;
      }
    }
  }
  return NULL;
}

void pushFunInParm(Object *variable) {
  stdoutQueue.data[stdoutQueue.tail++] = (void *)typeSpecifier[variable->type];
}

void dumpToStdout() {
  printf("cout ");

  while (stdoutQueue.head != stdoutQueue.tail) {
    printf("%s%s", (char *)stdoutQueue.data[stdoutQueue.head++],
           (stdoutQueue.tail == stdoutQueue.head + 1 ? "\n" : " "));
  }
}

int main(int argc, char *argv[]) {
  if (argc == 2) {
    yyin = fopen(yyInputFileName = argv[1], "r");
  } else {
    yyin = stdin;
  }
  if (!yyin) {
    printf("file `%s` doesn't exists or cannot be opened\n", yyInputFileName);
    exit(1);
  }

  // Start parsing
  yyparse();
  printf("Total lines: %d\n", yylineno);
  fclose(yyin);

  yylex_destroy();
  return 0;
}
