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
  public:
    Vertex mDestination;
    Vertex mSource;
    double mSpeed;
    double mLength; 

  //public:
    Edge();
    Edge(Vertex iSrc, Vertex iDst, double iSpeed, double iLen);
    void printInfo();
    double getLength();
    double getSpeed();
    Vertex getDestination();
    Vertex getSource();
};

Edge::Edge(Vertex iSrc, Vertex iDst, double iSpeed, double iLen)
  : mDestination(iDst),
    mSource(iSrc),
    mSpeed(iSpeed),
    mLength(iLen)
{
}


double Edge::getLength() {
  return mLength;
}

double Edge::getSpeed() {
  return mSpeed;
}

Vertex Edge::getDestination() {
  return mDestination;
}

Vertex Edge::getSource() {
  return mSource;
}

void Edge::printInfo() {
  cout << mSource.getName() << " -> " \
       << mDestination.getName() << " ";

  cout << "Speed: " << mSpeed << " ";
  cout << "Length: " << mLength << endl;

}
#endif
