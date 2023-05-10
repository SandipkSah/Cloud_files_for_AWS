#include "graph.h"
#include <pthread.h>

#ifndef ECHO_H
#define ECHO_H

typedef struct Process
{
    int id;
    int parent;
    int rec;
    Node *neigh;
    int neigh_size;
} process_t;

typedef struct Token
{
    char *data;
    int src;
    struct Token *next;
} token_t;

typedef struct ReceivedToken
{
    char *data;
    int src;
} received_token_t;

typedef struct ThreadArgs
{
    process_t *process;
    pthread_mutex_t* buffer_mutex;
    int *flags;
    pthread_mutex_t *flags_mutex;
    token_t **buffer;
} threadargs_t;

// function to add a message to the buffer
void createToken(int src, int dest, char *data, token_t **buffer, pthread_mutex_t *buffer_mutex, int *flags, pthread_mutex_t *flags_mutex);

// function to receive a message from the buffer
received_token_t *receiveToken(int dest, token_t **buffer);

// initiator function
void *initiator_work(void *arg);

// non-initiator function
void *nonInitiator_work(void *arg);

// function for running the echo algorithm on a multithreaded environment
void echo(Graph *graph, int initiator);

#endif // ECHO_H