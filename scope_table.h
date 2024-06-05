
#include "compiler_common.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  int32_t scopeLevel;
  Object **objectList;
  size_t objectListSize;
} Node;

Node *createNode(int scopeLevel);
void addObjectToList(Node *node, Object *obj);
Object *lastObject(Node *node);
