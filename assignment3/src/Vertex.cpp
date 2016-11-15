#include "Vertex.h"
#include "Edge.h"
#include <string.h>

Vertex::Vertex()
  : mName(""), mType(POI), mId(0)
{
}

Vertex::Vertex(vertex_type iType, string iName)
  : mName(iName), mType(iType)
{
}

string Vertex::getName()  {
  return mName;
}

void Vertex::printInfo() const{
	cout << "ID: " << mId << " ";

  cout << "Type: ";
  switch(mType) {
    case POI:
      cout << "POI [";
      cout << mPoi << "] ";
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

int Vertex::getId() const{
  return mId;
}

PointOfInterest Vertex::getPoi() {
  return mPoi;
}

void Vertex::setPoi(const PointOfInterest &iPoi) {
  mPoi = iPoi;
}

void Vertex::setId(int id) {
  mId = id;
}
