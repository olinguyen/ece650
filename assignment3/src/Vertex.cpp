#include "Vertex.h"
#include "Edge.h"

Vertex::Vertex() {
}

Vertex::Vertex(string iName, vertex_type iType)
  : mName(iName), mType(iType)
{
}

string Vertex::getName() {
  return mName;
}

void Vertex::printInfo() {
  cout << "Type: ";
  switch(mType) {
    case POI:
      cout << "Point-of-interest:";
      break;
    case INTERSECTION:
      cout << "Intersection:";
      break;
    case POI_AND_INTERSECTION:
      cout << "Point-of-interest:";
      break;
    default:
      cout << "Unknown type:";
  }

  cout << mName << endl;

}
