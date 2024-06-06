/* #include <iostream> */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler_common.h"
#include "main.h"
#include "queue.c"
#include "scope_table.c"
#include <queue>
#include <utility>

#define debug printf("%s:%d: ############### debug\n", __FILE__, __LINE__)

#define toupper(_char) (_char - (char)32)
using namespace std;

const char *objectTypeName[] = {
    "undefined", "auto",   "void", "char",   "int",      "long",
    "float",     "double", "bool", "string", "function",
};

Node *scopeTable[1 << 10];

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

void pushFunctionParm(ObjectType variableType, char *variableName,
                      int variableFlag) {
  pushQueue(functionParmQueue,
            new ObjectPair{variableType, variableFlag == VAR_FLAG_ARRAY});
  /* functionParmQueue.push( */
  /*     make_pair(variableType, variableFlag == VAR_FLAG_ARRAY)); */
  insertVariable(variableName, variableType);
}

void pushExpression(Object *out) { pushQueue(expressionQueue, (void *)out); }

Object *popExpression() {
  if (expressionQueue->size == 0)
    return NULL;

  return (Object *)popQueue(expressionQueue);

  /* if (expressionQueue.empty()) */
  /*   return NULL; */
  /* Object *tmp = expressionQueue.front(); */
  /* expressionQueue.pop(); */
  /* return tmp; */
}

void defineVariableHelper(ObjectType variableType) {
  for (int i = 0; i < scopeTable[scopeLevel]->objectListSize; i++) {
    Object *x = scopeTable[scopeLevel]->objectList[i];
    if (x->type == OBJECT_TYPE_UNDEFINED || x->type == OBJECT_TYPE_AUTO) {
      x->type = variableType;
    }
  }
  /* for (Object *x : scopeTable[scopeLevel]->objectList) { */
  /*   if (x->type == OBJECT_TYPE_UNDEFINED || x->type == OBJECT_TYPE_AUTO) { */
  /*     x->type = variableType; */
  /*   } */
  /* } */
}

void castingVariableHelper(ObjectType variableType) {
  // function's return type is in tmpType
  // Note: casting Type is also in
  printf("Cast to %s\n", objectTypeName[variableType]);
  lastObject(scopeTable[scopeLevel])->tmpType = variableType;
  /* scopeTable[scopeLevel]->objectList.back()->tmpType = variableType; */
}

void insertVariable(char *variableName, ObjectType variableType) {
  SymbolData *symbolData = new SymbolData{
      .name = variableName,
      .index = (int32_t)scopeTable[scopeLevel]->objectListSize,
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

  addObjectToList(scopeTable[scopeLevel], object);
  /* scopeTable[scopeLevel]->objectList.push_back(object); */
  printf("> Insert `%s` (addr: %d) to scope level %d\n", variableName,
         !scopeLevel ? -1 : variableAddress++, scopeLevel);
}

void pushScope() {
  /* Node *node = new Node{ */
  /*     .scopeLevel = ++scopeLevel, */
  /* }; */
  Node *node = createNode(++scopeLevel);

  scopeTable[scopeLevel] = node;

  /* scopeTable.push_back(node); */
  printf("> Create symbol table (scope level %d)\n", scopeLevel);
}

void dumpScope() {
  printf("\n");
  printf("> Dump symbol table (scope level: %d)\n", scopeLevel);
  printf("Index     Name                Type      Addr      Lineno    Func_sig "
         " \n");
  for (int i = 0; i < scopeTable[scopeLevel]->objectListSize; i++) {
    Object *x = scopeTable[scopeLevel]->objectList[i];
    printf("%-9d %-19s %-9s %-9ld %-9d %-10s\n", x->symbol->index,
           x->symbol->name, objectTypeName[x->type], x->symbol->addr,
           x->symbol->lineno, x->symbol->func_sig);
  }
  /* for (Object *x : scopeTable[scopeLevel]->objectList) { */
  /*   printf("%-9d %-19s %-9s %-9ld %-9d %-10s\n", x->symbol->index, */
  /*          x->symbol->name, objectTypeName[x->type], x->symbol->addr, */
  /*          x->symbol->lineno, x->symbol->func_sig); */
  /* } */
  free(scopeTable[scopeLevel]->objectList);
  free(scopeTable[scopeLevel]);

  /* scopeTable.pop_back(); */
  scopeLevel--;
}

Object *createVariable(ObjectType variableType, char *variableName,
                       int variableFlag) {
  return NULL;
}

void createMainFunction() {
  printf("func: main\n");
  insertVariable((char *)"main", OBJECT_TYPE_FUNCTION);
  Object *tmp = lastObject(scopeTable[0]);
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
    for (int j = 0; j < scopeTable[i]->objectListSize; j++) {
      Object *x = scopeTable[i]->objectList[j];
      if (strcmp(x->symbol->name, variableName) == 0) {
        return x;
      }
    }
    /* for (Object *x : scopeTable[i]->objectList) { */
    /*   if (strcmp(x->symbol->name, variableName) == 0) { */
    /*     return x; */
    /*   } */
    /* } */
  }
  return NULL;
}

void pushFunInParm(Object *variable) {

  pushQueue(stdoutQueue, (void *)objectTypeName[variable->type]);
  /* stdoutQueue.push(objectTypeName[variable->type]); */

  /* printf("pushFunInParm: %s\n", variable->symbol->name); */
  /* insert_variable(variable->symbol->name, variable->type); */
}

void stdoutPrint() {
  printf("cout ");
  /* cout << "cout "; */

  while (stdoutQueue->size > 0) {
    printf("%s%s", (char *)popQueue(stdoutQueue),
           (stdoutQueue->size == 1 ? "\n" : " "));
    /* printf("stdoutQueue size: %d\n", stdoutQueue->size); */
    /* cout << (char *)popQueue(stdoutQueue) << (!stdoutQueue->size ? "" : " ");
     */
  }
  /* while (stdoutQueue.size() > 0) { */
  /*   cout << stdoutQueue.front() << (stdoutQueue.size() == 1 ? "" : " "); */
  /*   stdoutQueue.pop(); */
  /* } */
  /* printf("\n"); */
  /* cout << endl; */
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
