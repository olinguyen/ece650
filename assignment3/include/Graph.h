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
    int mNumEdges;
    int mNumPoi;
  public:
    vector<Vertex> mVertexList;
    Graph();
    Graph(int v, int e);
    void readData();
    Vertex& addVertex(vertex_type type, string name);
    Edge& addEdge(Vertex src, Vertex dst, bool directional, double speed, double length);
		Edge& addEdge(int iSrcId, int iDstId, bool directional, double speed, double length);
    void edgeEvent(Edge& e, bool event);
    Vertex vertex(PointOfInterest poi);
    void trip(Vertex src, Vertex dst); // finds shortest path
    void store(string filename);
    void retrieve(string filename);
    void printGraph();
    void printEdges();
};


#endif
