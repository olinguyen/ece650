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
    int mId;
    PointOfInterest mPoi;
    
  public:
    vector<Edge> mAdjacencyList;

    Vertex();
    Vertex(vertex_type iType, string iName, int id);
    void printInfo();
    string getName();
    int getId();
};


#endif
