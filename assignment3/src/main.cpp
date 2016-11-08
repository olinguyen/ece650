#include <iostream>
#include <vector>

#include "Vertex.h"
#include "Edge.h"

using namespace std;

int main(int argc, char** argv) {
  Vertex A("A", POI), B("B", POI);

  Edge e(A, B, 1.0, 1.0);
  e.printInfo();

  return 0;
}
