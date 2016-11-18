#include <iostream>
#include <vector>
#include <assert.h>

#include "Vertex.h"
#include "Edge.h"
#include "Graph.h"

using namespace std;


int main(int argc, char** argv) {
  {
    Graph g;
    g.retrieve("short_path.in");
    Vertex v1 = g.vertex("DC");
    Vertex v2 = g.vertex("LIB");
    static const int arr_closure[] = {0, 2, 4, 5};
    vector<int> ground_truth_closure(arr_closure, arr_closure + sizeof(arr_closure) / sizeof(arr_closure[0]) );
    vector<int> output = g.trip(v1,v2);
    assert(output == ground_truth_closure);
  }

  {
    Graph g;
    g.retrieve("longer_path.in");
    Vertex v1 = g.vertex("DC");
    Vertex v2 = g.vertex("LIB");
    static const int arr_closure[] = {0, 2, 4, 5};
    vector<int> ground_truth_closure(arr_closure, arr_closure + sizeof(arr_closure) / sizeof(arr_closure[0]) );
    vector<int> output = g.trip(v1,v2);
    assert(output == ground_truth_closure);
  }

  {
    Graph g;
    g.retrieve("normal.in");

    Vertex v1 = g.vertex("DC");
    Vertex v2 = g.vertex("LIB");

    static const int arr[] = {0, 2, 4, 1, 5};
    vector<int> ground_truth(arr, arr + sizeof(arr) / sizeof(arr[0]) );
    vector<int> output = g.trip(v1,v2);
    assert(output == ground_truth);
  }

  // Test w/ road closure

  {
    Graph g;
    g.retrieve("closure.in");
    Vertex v1 = g.vertex("DC");
    Vertex v2 = g.vertex("LIB");
    static const int arr_closure[] = {0, 2, 4, 5};
    vector<int> ground_truth_closure(arr_closure, arr_closure + sizeof(arr_closure) / sizeof(arr_closure[0]) );
    vector<int> output = g.trip(v1,v2);
    assert(output == ground_truth_closure);
  }

  //g.store("dijkstra.out");

  return 0;
}
