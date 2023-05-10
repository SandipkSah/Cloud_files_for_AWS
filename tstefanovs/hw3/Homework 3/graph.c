/*The graph implementation has been found on the internet as it is not the main objective of the assignment*/
#include "graph.h"
#include <stdio.h>

// Function to create a new node in the adjacency list
Node *createNode(int dest)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->dest = dest;
    newNode->next = NULL;
    return newNode;
}

// Function to create a new graph with V vertices
Graph *initGraph(int V)
{
    Graph *graph = (Graph *)malloc(sizeof(Graph));
    graph->V = V;
    graph->adjList = (List *)malloc(V * sizeof(List));
    // Initialize the adjacency list for each vertex
    for (int i = 1; i <= V; i++)
    {
        graph->adjList[i].head = NULL;
        graph->adjList[i].size = 0;
    }
    return graph;
}

// Function to add an edge to the graph
void addEdge(Graph *graph, int node1, int node2)
{
    // Add edge from node1 to node2
    Node *newNode = createNode(node2);
    newNode->next = graph->adjList[node1].head;
    graph->adjList[node1].head = newNode;
    // Add edge from node2 to node1
    newNode = createNode(node1);
    newNode->next = graph->adjList[node2].head;
    graph->adjList[node2].head = newNode;
    // Increment the size of the adjacency list for both nodes
    graph->adjList[node1].size++;
    graph->adjList[node2].size++;
}

Graph *createGraph1()
{
    //graph from p3.2
    Graph *graph = initGraph(8);

    addEdge(graph, 1, 2);
    addEdge(graph, 1, 6);
    addEdge(graph, 1, 7);
    addEdge(graph, 2, 3);
    addEdge(graph, 2, 4);
    addEdge(graph, 3, 4);
    addEdge(graph, 4, 5);
    addEdge(graph, 5, 6);
    addEdge(graph, 5, 7);
    addEdge(graph, 6, 7);

    return graph;
}

Graph *createGraph2()
{
    // graph from slides
    Graph *graph = initGraph(6);
    addEdge(graph, 2, 4);
    addEdge(graph, 2, 5);
    addEdge(graph, 4, 5);
    addEdge(graph, 1, 2);
    addEdge(graph, 1, 3);
    addEdge(graph, 1, 4);
    addEdge(graph, 2, 3);

    return graph;
}
