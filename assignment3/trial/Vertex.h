#ifndef VERTEX_H
#define VERTEX_H

#include <string>
#include <iostream>
#include <vector>

using namespace std;

class Edge;

typedef enum {
  POI = 0,
  INTERSECTION = 1,
  POI_AND_INTERSECTION = 2
} vertex_type;

typedef string PointOfInterest;

class Vertex {
  private:
    vertex_type mType;
    string mName;
    
    PointOfInterest mPoi;
    
  public:
    
    vector<Edge> mAdjacencyList;
    int mId;
    Vertex();
    Vertex(vertex_type iType, string iName, int id);
    void printInfo();
    string getName();
    int getId();
};
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

#endif
