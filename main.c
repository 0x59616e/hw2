/* #include <iostream> */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler_common.h"
#include "main.h"
#include "queue.c"
#include <queue>
#include <utility>

#define debug printf("%s:%d: ############### debug\n", __FILE__, __LINE__)

#define toupper(_char) (_char - (char)32)
using namespace std;

const char *objectTypeName[] = {
    "undefined", "auto",   "void", "char",   "int",      "long",
    "float",     "double", "bool", "string", "function",
};

typedef struct {
  Object *objects[100] = {NULL};
  int32_t size = 0;
} SymbolTable;

SymbolTable symbolTable[100];

/* queue<string> stdoutQueue; */
Queue *stdoutQueue = createQueue();
Queue *expressionQueue = createQueue();
Queue *functionParmQueue = createQueue();
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
  pushQueue(functionParmQueue, new ObjectPair{OBJECT_TYPE_FUNCTION, true});
  insertVariable((char *)"argv", OBJECT_TYPE_STR);
}

void pushExpression(Object *out) { pushQueue(expressionQueue, (void *)out); }

Object *popExpression() {
  if (expressionQueue->size == 0)
    return NULL;

  return (Object *)popQueue(expressionQueue);
}

void insertVariable(ObjectType variableType) {
  for (int i = 0; i < symbolTable[scopeLevel].size; i++) {
    Object *x = symbolTable[scopeLevel].objects[i];
    if (x->type == OBJECT_TYPE_UNDEFINED) {
      x->type = variableType;
    }
  }
}

void insertVariable(char *variableName, ObjectType variableType) {
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
           x->symbol->name, objectTypeName[x->type], x->symbol->addr,
           x->symbol->lineno, x->symbol->func_sig);
  }

  symbolTable[scopeLevel].size = 0;

  scopeLevel--;
}

void createMainFunction() {
  printf("func: main\n");
  insertVariable((char *)"main", OBJECT_TYPE_FUNCTION);
  Object *tmp = symbolTable[0].objects[symbolTable[0].size - 1];
  tmp->tmpType = OBJECT_TYPE_FUNCTION;
  char *funcProto = (char *)"([Ljava/lang/String;)I";
  strcpy(tmp->symbol->func_sig, funcProto);
}

void debugPrintInst(char instc, Object *a, Object *b, Object *out) {}

bool objectExpression(char op, Object *dest, Object *val, Object *out) {
  return false;
}

bool objectExpBinary(char op, Object *a, Object *b, Object *out) {
  return false;
}

bool objectExpBoolean(char op, Object *a, Object *b, Object *out) {
  return false;
}

bool objectExpAssign(char op, Object *dest, Object *val, Object *out) {
  return false;
}

bool objectValueAssign(Object *dest, Object *val, Object *out) { return false; }

bool objectNotBinaryExpression(Object *dest, Object *out) { return false; }

bool objectNegExpression(Object *dest, Object *out) { return false; }
bool objectNotExpression(Object *dest, Object *out) { return false; }

bool objectIncAssign(Object *a, Object *out) { return false; }

bool objectDecAssign(Object *a, Object *out) { return false; }

bool objectCast(ObjectType variableType, Object *dest, Object *out) {
  return false;
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
  pushQueue(stdoutQueue, (void *)objectTypeName[variable->type]);
}

void stdoutPrint() {
  printf("cout ");
  /* cout << "cout "; */

  while (stdoutQueue->size > 0) {
    printf("%s%s", (char *)popQueue(stdoutQueue),
           (stdoutQueue->size == 1 ? "\n" : " "));
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
