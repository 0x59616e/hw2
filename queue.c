#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

Queue *createQueue() {
  Queue *queue = (Queue *)malloc(sizeof(Queue));
  queue->size = 0;
  queue->head = NULL;
  queue->tail = NULL;
  return queue;
}

void pushQueue(Queue *queue, void *data) {
  QueueNode *newNode = (QueueNode *)malloc(sizeof(QueueNode));
  newNode->data = data;
  newNode->next = NULL;
  if (queue->size == 0) {
    queue->head = newNode;
    queue->tail = newNode;
  } else {
    queue->tail->next = newNode;
    queue->tail = newNode;
  }
  queue->size++;
}

void *popQueue(Queue *queue) {
  if (queue->size == 0) {
    return NULL;
  } else if (queue->size == 1) {
    void *tmp = queue->head->data;
    queue->head = NULL;
    queue->tail = NULL;
    queue->size--;
    return tmp;
  }
  void *tmp = queue->head->data;
  queue->head = queue->head->next;
  queue->size--;
  return tmp;
}
