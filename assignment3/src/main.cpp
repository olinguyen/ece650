#include <iostream>
#include <vector>
#include <assert.h>

#include "Vertex.h"
#include "Edge.h"
#include "Graph.h"

using namespace std;


bool test_roadmap(string input, vector<int> array) {
    Graph g;

    g.retrieve(input);

    Vertex v1 = g.vertex("DC");
    Vertex v2 = g.vertex("LIB");

    vector<int> output = g.trip(v1,v2);
    for(size_t i = 0; i < output.size(); ++i) {
      cout << output[i] << " ";
    }
    cout << endl;

    return (output == array);

}


int main(int argc, char** argv) {
  vector<int> normal_expected{0, 2, 4, 1, 5};
  assert(test_roadmap("normal.in", normal_expected));

  vector<int> short_expected{0, 1, 5};
  assert(test_roadmap("short_path.in", short_expected));

  vector<int> closure_expected{0, 2, 1, 5};
  assert(test_roadmap("closure.in", closure_expected));

  vector<int> long_expected{0, 2, 4, 3};
  assert(test_roadmap("path2.in", long_expected));

  return 0;
}
