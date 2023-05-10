// Implementation of the Echo Algorithm, using multiple threads

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "echo.c"
#include "graph.h"


int main()
{
    // Create the graph
    Graph *graph_1 = createGraph1();
    Graph *graph_2 = createGraph2();

    // Run the echo algorithms
    printf("\n\n================================ START OF GRAPH 1 ECHO ALGORITHM ================================\n\n\n");
    echo(graph_1, 1);
    printf("\n\n================================ START OF GRAPH 2 ECHO ALGORITHM ================================\n\n\n");
    echo(graph_2, 1);

    return 0;
}