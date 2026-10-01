# Grandma Maritxu's GPS

A C++17 project that implements **Dijkstra's algorithm** to calculate the minimum-cost path between different towns or locations in a graph.

The program reads a text file containing the graph definition and a list of route queries, processes each query, and generates an output file with the minimum cost, the path followed, and the execution time. :chatgpt-content-reference{index="0"}

## Objective

The goal of the project is to build a simple route-finding system capable of answering questions such as:

> What is the minimum-cost path between an origin and a destination?

The route network is represented as an **undirected weighted graph**, where:

- each node represents a town or location;
- each edge represents a route between two locations;
- the edge weight represents the cost of travelling along that route.

The shortest path is calculated using **Dijkstra's algorithm**.

---

## Program input

The program receives a text file as an argument:

```bash
./maritxu grafo.txt
