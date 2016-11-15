#include <iostream>
#include <vector>

#include "Vertex.h"
#include "Edge.h"
#include "Graph.h"

using namespace std;


int main(int argc, char** argv) {
  Graph g;
  
  /*
  Vertex src = g.addVertex(POI, "A");
  Vertex dst = g.addVertex(POI, "B");

  g.addEdge(src, dst, 100.0, 100.0);

  g.printGraph();
  */
  //g.retrieve("dijkstra.in");


 // Vertex src = g.addVertex(POI, "A");
 //Vertex dst = g.addVertex(POI, "B");

//	g.printGraph();
 // g.retrieve("dijkstra.in");
  g.retrieve("dijkstra.in");
  Vertex v1 = g.vertex("DC");
  Vertex v2 = g.vertex("LIB");
//g.printGraph();
  g.trip(v1,v2);

  return 0;
}
