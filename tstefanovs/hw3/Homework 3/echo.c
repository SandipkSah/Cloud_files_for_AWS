#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "echo.h"
#include "graph.c"

// function to add a token to the buffer
void createToken(int src, int dest, char *data, token_t **buffer, pthread_mutex_t *token, int *first, pthread_mutex_t *first_mutex)
{
    // create the new token
    token_t *newToken = (token_t *)malloc(sizeof(token_t));
    // allocate memory for the data
    newToken->data = (char *)malloc(strlen(data) + 1);
    // copy the data to the new memory
    strcpy(newToken->data, data);
    // set the next pointer to NULL
    newToken->next = NULL;
    // set the source of the token
    newToken->src = src;

    pthread_mutex_lock(first_mutex);
    if (first[dest] == 0)
    {
        first[dest] = 1;
        buffer[dest] = newToken;
        newToken->next = NULL;
        pthread_mutex_unlock(&token[dest]);
    }
    else
    {
        pthread_mutex_lock(&token[dest]);

        newToken->next = buffer[dest];
        buffer[dest] = newToken;
        pthread_mutex_unlock(&token[dest]);
    }
    pthread_mutex_unlock(first_mutex);
}

// function to receive a token from the buffer
received_token_t *receiveToken(int dest, token_t **buffer)
{
    if (buffer[dest] == NULL)
    {
        return NULL;
    }
    else
    {
        token_t *temp = buffer[dest];
        buffer[dest] = buffer[dest]->next;
        received_token_t *received = (received_token_t *)malloc(sizeof(received_token_t));
        received->data = temp->data;
        received->src = temp->src;
        free(temp);
        return received;
    }
}

// initiator function
void *initiator_work(void *arg)
{
    threadargs_t *args = (threadargs_t *)arg;
    process_t *process = args->process;
    pthread_mutex_t *buffer_mutex = args->buffer_mutex;
    int *flags = args->flags;
    pthread_mutex_t *flags_mutex = args->flags_mutex;
    token_t **buffer = args->buffer;

    // send messages to all neighbors
    Node *iter = process->neigh;
    while (iter)
    {
        printf("Initiator %d:\t  Sending token to %d\n", process->id, iter->dest);
        createToken(process->id, iter->dest, "p1", buffer, buffer_mutex, flags, flags_mutex);
        iter = iter->next;
    }

    // receive messages from all neighbors
    while (process->rec < process->neigh_size)
    {
        pthread_mutex_lock(&buffer_mutex[process->id]);
        received_token_t *data = receiveToken(process->id, buffer);
        if (data)
        {
            printf("Initiator %d:\t  Received token from %d\n", process->id, data->src);
            process->rec++;
            free(data);
        }
        pthread_mutex_unlock(&buffer_mutex[process->id]);
    }

    // decide
    printf("Initiator %d:\t  Deciding\n", process->id);
    return 0;
}

// non-initiator function
void *nonInitiator_work(void *arg)
{
    threadargs_t *args = (threadargs_t *)arg;
    process_t *process = args->process;
    pthread_mutex_t *buffer_mutex = args->buffer_mutex;
    int *flags = args->flags;
    pthread_mutex_t *flags_mutex = args->flags_mutex;
    token_t **buffer = args->buffer;

    // receive the first token and set the parent
    printf("Non-initiator %d:  Waiting for first token\n", process->id);
    pthread_mutex_lock(&buffer_mutex[process->id]);
    received_token_t *data = receiveToken(process->id, buffer);
    if (data)
    {
        printf("Non-initiator %d:  Received token from parent %d\n", process->id, data->src);
        process->parent = data->src;
        process->rec++;
        free(data);
    }
    pthread_mutex_unlock(&buffer_mutex[process->id]);

    int sent_count = 0;
    // send the token to all neighbors except the parent
    while (sent_count < process->neigh_size)
    {
        if (process->neigh->dest != process->parent)
        {
            printf("Non-initiator %d:  Sending token to %d\n", process->id, process->neigh->dest);
            createToken(process->id, process->neigh->dest, "token", buffer, buffer_mutex, flags, flags_mutex);
        }
        process->neigh = process->neigh->next;
        sent_count++;
    }

    // receive messages from all neighbors except the parent
    while (process->rec < process->neigh_size)
    {
        pthread_mutex_lock(&buffer_mutex[process->id]);
        received_token_t *data = receiveToken(process->id, buffer);
        if (data)
        {
            printf("Non-initiator %d:  Received token from %d\n", process->id, data->src);
            process->rec++;
            free(data);
        }
        pthread_mutex_unlock(&buffer_mutex[process->id]);
    }

    // send the token to the parent
    printf("Non-initiator %d:  Sending token to parent %d\n", process->id, process->parent);
    createToken(process->id, process->parent, "p1", buffer, buffer_mutex, flags, flags_mutex);

    // end
    printf("Non-initiator %d:  Ending\n", process->id);

    return 0;
}

void echo(Graph *graph, int initiator_id)
{
    pthread_t threads[graph->V];
    process_t processes[graph->V];

    // create the buffer for the messages
    token_t **buffer = (token_t **)malloc(graph->V * sizeof(token_t *));
    for (int i = 0; i < graph->V; i++)
    {
        buffer[i] = NULL;
    }

    int flags[graph->V];
    for (int i = 0; i < graph->V; i++)
    {
        flags[i] = 0;
    }

    pthread_mutex_t flags_mutex = PTHREAD_MUTEX_INITIALIZER;

    // define and initialize the mutexes
    pthread_mutex_t buffer_mutex[graph->V];
    for (int i = 0; i < graph->V; i++)
    {
        int e = pthread_mutex_init(&buffer_mutex[i], NULL);
        if (e != 0)
        {
            fprintf(stderr, "pthread_mutex_init(): %s\n", strerror(e));
            exit(EXIT_FAILURE);
        }
        pthread_mutex_lock(&buffer_mutex[i]);
    }

    for (int i = 1; i < graph->V; i++)
    {
        processes[i].id = i;
        processes[i].parent = 0;
        processes[i].rec = 0;
        processes[i].neigh = graph->adjList[i].head;
        processes[i].neigh_size = graph->adjList[i].size;

        threadargs_t *args = (threadargs_t *)malloc(sizeof(threadargs_t));
        args->process = &processes[i];
        args->buffer_mutex = buffer_mutex;
        args->buffer = buffer;
        args->flags = flags;
        args->flags_mutex = &flags_mutex;

        if (i == initiator_id)
        {
            pthread_create(&threads[i], NULL, initiator_work, args);
        }
        else
        {
            pthread_create(&threads[i], NULL, nonInitiator_work, args);
        }
    }

    // Join the threads
    for (int i = 1; i < graph->V; i++)
    {
        pthread_join(threads[i], NULL);
    }
    printf("\n");
    // print the rec and parent values
    for (int i = 1; i < graph->V; i++)
    {
        printf("Node %d: rec = %d out of %d messages, parent = %d\n", i, processes[i].rec, processes[i].neigh_size, processes[i].parent);
    }
}