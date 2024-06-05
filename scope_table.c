#include "scope_table.h"
#include "compiler_common.h"
#include <stdio.h>
#include <stdlib.h>

Node *createNode(int scopeLevel) {
  Node *node = (Node *)malloc(sizeof(Node));
  node->scopeLevel = scopeLevel;
  node->objectList = NULL;
  node->objectListSize = 0;
  return node;
}

void addObjectToList(Node *node, Object *obj) {
  Object **newList = (Object **)realloc(
      node->objectList, (node->objectListSize + 1) * sizeof(Object *));

  node->objectList = newList;
  node->objectList[node->objectListSize] = obj;

  node->objectListSize++;
}

Object *lastObject(Node *node) {
  if (node->objectListSize == 0) {
    return NULL;
  }
  return node->objectList[node->objectListSize - 1];
}
