#include "Vertex.h"
#include "Edge.h"

Vertex::Vertex() {
}

Vertex::Vertex(vertex_type iType, string iName, int id)
  : mName(iName), mType(iType), mId(id)
{
}

string Vertex::getName() {
  return mName;
}

void Vertex::printInfo() {
  cout << "Type: ";
  switch(mType) {
    case POI:
      cout << "POI [";
      break;
    case INTERSECTION:
      cout << "Intersection : ";
      break;
    case POI_AND_INTERSECTION:
      cout << "Point-of-interest : ";
      break;
    default:
      cout << "Unknown type : ";
  }

  cout << mName << endl;
}

int Vertex::getId() {
  return mId;
}
