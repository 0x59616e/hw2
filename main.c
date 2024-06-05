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

char *JNIHelper(ObjectType type, int isArray) {
  // 初始化結果字符串
  char *result;
  if (isArray) {
    result = (char *)malloc(2 * sizeof(char)); // 分配空間以容納 '[' 和 '\0'
    if (!result) {
      return NULL; // 分配失敗
    }
    strcpy(result, "[");
  } else {
    result = (char *)malloc(1 * sizeof(char)); // 分配空間以容納 '\0'
    if (!result) {
      return NULL; // 分配失敗
    }
    strcpy(result, "");
  }

  // 根據類型追加字符串
  const char *typeStr;
  switch (type) {
  case OBJECT_TYPE_INT:
    typeStr = "I";
    break;
  case OBJECT_TYPE_LONG:
    typeStr = "J";
    break;
  case OBJECT_TYPE_FLOAT:
    typeStr = "F";
    break;
  case OBJECT_TYPE_DOUBLE:
    typeStr = "D";
    break;
  case OBJECT_TYPE_BOOL:
    typeStr = "B";
    break;
  case OBJECT_TYPE_VOID:
    typeStr = "V";
    break;
  case OBJECT_TYPE_STR:
    typeStr = "Ljava/lang/String;";
    break;
  default:
    typeStr = "";
  }

  // 追加類型字符串
  result = (char *)realloc(result, (strlen(result) + strlen(typeStr) + 1) *
                                       sizeof(char));
  if (!result) {
    return NULL; // 分配失敗
  }
  strcat(result, typeStr);

  return result;
}

void genFunctionJNI(ObjectType returnType, char *funcName) {
  char *funcSig = (char *)malloc(2 * sizeof(char));
  if (!funcSig) {
    puts("ddd");
    return;
  }
  strcpy(funcSig, "(");

  while (functionParmQueue->size > 0) {
    ObjectPair *pair = (ObjectPair *)popQueue(functionParmQueue);
    char *paramSig = JNIHelper(pair->type, pair->isArray);
    funcSig = (char *)realloc(
        funcSig, (strlen(funcSig) + strlen(paramSig) + 1) * sizeof(char));
    if (!funcSig) {
      puts("ddd");
      return;
    }
    strcat(funcSig, paramSig);
    free(paramSig);
    free(pair);
  }

  funcSig = (char *)realloc(funcSig, (strlen(funcSig) + 2) * sizeof(char));
  if (!funcSig) {
    puts("ddd");
    return;
  }
  strcat(funcSig, ")");

  char *returnSig = JNIHelper(returnType, 0);
  funcSig = (char *)realloc(funcSig, (strlen(funcSig) + strlen(returnSig) + 1) *
                                         sizeof(char));
  if (!funcSig) {
    puts("ddd");
    return;
  }
  strcat(funcSig, returnSig);
  free(returnSig);

  strcpy(lastObject(scopeTable[scopeLevel - 1])->symbol->func_sig, funcSig);
  free(funcSig);
}

/*
void genFunctionJNI(ObjectType returnType, char *funcName) {
  string funcSig = "";
  funcSig += "(";
  while (functionParmQueue->size > 0) {
    ObjectPair *pair = (ObjectPair *)popQueue(functionParmQueue);

    funcSig += JNIHelper(pair->type, pair->isArray);
  }
  funcSig += ")";
  // TODO
  funcSig += JNIHelper(returnType, false);

  strcpy(lastObject(scopeTable[scopeLevel - 1])->symbol->func_sig,
         funcSig.c_str());
}
*/

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

void createFunction(ObjectType variableType, char *funcName) {
  printf("func: %s\n", funcName);
  /* printf("funcxxx: %s\n", objectTypeName[variableType]); */

  insertVariable(funcName, OBJECT_TYPE_FUNCTION);
  lastObject(scopeTable[scopeLevel])->tmpType = variableType;
  /* scopeTable[scopeLevel]->objectList.back()->tmpType = variableType; */
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
