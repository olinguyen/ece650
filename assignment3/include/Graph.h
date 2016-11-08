#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <iostream>
#include <vector>

#include "Vertex.h"
#include "Edge.h"

using namespace std;

class Graph {
  private:
    int mNumVertices;
    vector<Vertex> mVertexList;
  public:
    Graph();
    Graph(int v);

    void addVertex(vertex_type type, string name);
    void addEdge(Vertex src, Vertex dst, double speed, double length);
    void edgeEvent(Edge e, event_type event);
    Vertex vertex(PointOfInterest poi);
    void trip(Vertex src, Vertex dst); // finds shortest path
    void store(string filename);
    void retrieve(string filename);
};


#endif
