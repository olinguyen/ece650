#include <iostream>
#include <vector>
#include <fstream>
#include "Vertex.h"
#include "Edge.h"
#include "Graph.h"

using namespace std;


int main(int argc, char** argv) {
  Graph g;
  int number_of_vertices = 0;
  int number_of_edges = 0;
  g.addVertex(POI, "A");
  number_of_vertices++;
  g.addVertex(POI, "B");
  number_of_vertices++;
  g.addEdge((g.mVertexList.at(0)),(g.mVertexList.at(1)),10,15);
  number_of_edges++;
  fstream out("dijkstra.in", ios::out);
  out << number_of_vertices << " " << number_of_edges << endl;
  for(vector<Vertex>::const_iterator i = g. mVertexList.begin(); i!=  g.mVertexList.end();++i)
	for(vector<Edge>::const_iterator j = i->mAdjacencyList.begin(); j != i->mAdjacencyList.end();++j)
		out<<(j-> mDestination).mId<< " " << (j->mSource).mId << " " <<  j->mLength<<endl;
  return 0;
}
