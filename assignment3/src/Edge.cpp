#include "Vertex.h"
#include "Edge.h"

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
