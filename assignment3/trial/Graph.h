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
    
  public:
    Graph();
    Graph(int v);
    vector<Vertex> mVertexList;
    void addVertex(vertex_type type, string name);
    void addEdge(Vertex src, Vertex dst, double speed, double length);
    void edgeEvent(Edge e, event_type event);
    Vertex vertex(PointOfInterest poi);
    void trip(Vertex src, Vertex dst); // finds shortest path
    void store(string filename);
    void retrieve(string filename);
};

Graph::Graph() 
  : mNumVertices(0)
{
}

Graph::Graph(int v) 
  : mNumVertices(v)
{
}

void Graph::addVertex(vertex_type type, string name)
{
  Vertex v(type, name, mNumVertices);
  mVertexList.push_back(v);
  mNumVertices++;
}

void Graph::addEdge(Vertex src, Vertex dst, double speed, double length)
{
  int srcIndex = src.getId(); 

  Edge e(src, dst, speed, length);
  mVertexList[srcIndex].mAdjacencyList.push_back(e);
}

void Graph::edgeEvent(Edge e, event_type event)
{
}

Vertex Graph::vertex(PointOfInterest poi)
{
  Vertex v;
  return v;
}

void Graph::trip(Vertex src, Vertex dst)
{
  
}

void Graph::store(string filename)
{
}

void Graph::retrieve(string filename)
{
}
#endif
