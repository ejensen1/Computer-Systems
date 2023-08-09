#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <semaphore.h>

#include "queue.h"

struct queue {
    int size;
    int head;
    int tail;
    //int count;
    void **elements;
    sem_t full_slots;
    sem_t empty_slots;
};

queue_t *queue_new(int size) {

    queue_t *q = (queue_t *) malloc(sizeof(queue_t));
    q->elements = (void **) calloc(size, sizeof(void *));
    q->size = size;
    q->head = 0;
    q->tail = 0;

    sem_init(&q->empty_slots, 0, size);
    sem_init(&q->full_slots, 0, 0);

    return q;
}

void queue_delete(queue_t **q) {
    sem_destroy(&((*q)->full_slots));
    sem_destroy(&((*q)->empty_slots));
    free((*q)->elements);
    q = NULL;
    return;
}

bool queue_push(queue_t *q, void *elem) {
    if (!q) {
        return 0;
    }
    sem_wait(&q->empty_slots);
    q->elements[q->head] = elem;
    q->head = (q->head + 1) % q->size;
    sem_post(&q->full_slots);
    return 1;
}

bool queue_pop(queue_t *q, void **elem) {
    if (!q) {
        return 0;
    }
    sem_wait(&q->full_slots);
    *elem = q->elements[q->tail];
    q->tail = (q->tail + 1) % q->size;
    sem_post(&q->empty_slots);
    return 1;
}
