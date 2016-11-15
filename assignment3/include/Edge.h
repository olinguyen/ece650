#ifndef EDGE_H
#define EDGE_H

#include <string>
#include <iostream>

using namespace std;

class Vertex;

class Edge {
  private:
    Vertex mDestination;
    Vertex mSource;
		int mDstId;
		int mSrcId;
    double mSpeed;
    double mLength; 
		bool mEvent; // True = closure on road

  public:
    Edge();
    Edge(Vertex iSrc, Vertex iDst, double iSpeed, double iLen);
    Edge(int iSrc, int iDst, double iSpeed, double iLen);
    void printInfo();
    double getLength();
    double getSpeed();
    Vertex getDestination();
    Vertex getSource();
		bool getEvent();
		void setEvent(bool iEvent);
};


#endif
