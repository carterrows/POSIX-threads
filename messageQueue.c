// Assignment 2 - CIS*3110
// Carter Rows - 1170615

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pthread.h>

#include "cartersUtilities.h"

MessageQueue *create_message_queue() {
    MessageQueue *q = (MessageQueue*)malloc(sizeof(MessageQueue));
    q->head = q->tail = NULL;
    pthread_mutex_init(&q->mutex, NULL);
    
    //Initialize the condition variable
    pthread_cond_init(&q->cond, NULL);
    return q;
}

void send_message(MessageQueue *q, TopThree *found, char *fileName, int totalIncorrect) {
    MessageNode *node = (MessageNode*)malloc(sizeof(MessageNode));
    strncpy(node->msg.found.word1, found->word1, MAX_WORD_LEN);
    strncpy(node->msg.found.word2, found->word2, MAX_WORD_LEN);
    strncpy(node->msg.found.word3, found->word3, MAX_WORD_LEN);
    strncpy(node->msg.fileName, fileName, MAX_WORD_LEN);
    node->msg.found.count1 = found->count1;
    node->msg.found.count2 = found->count2;
    node->msg.found.count3 = found->count3;
    node->msg.totalIncorrect = totalIncorrect;
    node->next = NULL;

    // critical section
    pthread_mutex_lock(&q->mutex);
    if (q->tail != NULL) {
        q->tail->next = node;       // append after tail
        q->tail = node;
    } else {
        q->tail = q->head = node;   // first node
    }
    //Signal the consumer thread waiting on this condition variable
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mutex);
    //fprintf(stderr, "Worker %d enqueues the message, signals cond variable, and unlocks mutex\n", sender);
}

int get_message(MessageQueue *q, Message *msg_out) {
    int success = 0;
    
    // critical section
    pthread_mutex_lock(&q->mutex);
    
    //Wait for a signal telling us that there's something on the queue
    //If we get woken up but the queue is still null, we go back to sleep
    while(q->head == NULL){
        //fprintf(stderr,"Message queue is empty and getMessage goes to sleep (pthread_cond_wait)\n");
        pthread_cond_wait(&q->cond, &q->mutex);
        //fprintf(stderr,"getMessage is woken up - someone signalled the condition variable\n");
    }
    
    //By the time we get here, we know q->head is not null, so it's all good
    MessageNode *oldHead = q->head;
    *msg_out = oldHead->msg;    // copy out the message
    q->head = oldHead->next;
    if (q->head == NULL) {
        q->tail = NULL;         // last node removed
    }
    free(oldHead);
    success = 1;
    
    //Release lock
    pthread_mutex_unlock(&q->mutex);

    return success;
}
