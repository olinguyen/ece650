#ifndef EDGE_H
#define EDGE_H

#include <string>
#include <iostream>

using namespace std;

class Vertex;

typedef enum {
  ACCIDENT = 0,
  DEBRIS   = 1,
  CLOSURE  = 2,
} event_type;

class Edge {
  private:
    Vertex mDestination;
    Vertex mSource;
    double mSpeed;
    double mLength; 

  public:
    Edge();
    Edge(Vertex iSrc, Vertex iDst, double iSpeed, double iLen);
    void printInfo();
    double getLength();
    double getSpeed();
    Vertex getDestination();
    Vertex getSource();
};


#endif
