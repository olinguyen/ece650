#include <iostream>
#include <vector>
#include <assert.h>

#include "Vertex.h"
#include "Edge.h"
#include "Graph.h"

using namespace std;


int main(int argc, char** argv) {
  Graph g;

 //	g.printGraph();
  g.retrieve("dijkstra.in");

  Vertex v1 = g.vertex("DC");
  Vertex v2 = g.vertex("LIB");

  static const int arr[] = {0, 2, 4, 1, 5};
  vector<int> ground_truth(arr, arr + sizeof(arr) / sizeof(arr[0]) );
  vector<int> output = g.trip(v1,v2);
  assert(output == ground_truth);

  g.store("dijkstra.out");

  return 0;
}
