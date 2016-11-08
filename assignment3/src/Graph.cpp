#include "Graph.h"

Graph::Graph() {
}

Graph::Graph(int v) 
  : mNumVertices(v)
{
}

void Graph::addEdge(Vertex src, Vertex dst, double speed, double length)
{
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
