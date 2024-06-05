
typedef struct QueueNode {
  void *data;
  struct QueueNode *next;
} QueueNode;

typedef struct Queue {
  QueueNode *head;
  QueueNode *tail;
  int size;
} Queue;

Queue *createQueue();
void pushQueue(Queue *queue, void *data);
void *popQueue(Queue *queue);
