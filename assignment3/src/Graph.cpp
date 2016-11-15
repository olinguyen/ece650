#include <fstream>
#include "Graph.h"

#define DEBUG 1

Graph::Graph() 
  : mNumVertices(0)
{
}

Graph::Graph(int v, int e) 
  : mNumVertices(v)
  , mNumEdges(e)
{
}

Vertex& Graph::addVertex(vertex_type type, string name)
{
  Vertex v(type, name);
  mVertexList.push_back(v);
  return mVertexList.back();
}

Edge& Graph::addEdge(Vertex src, Vertex dst, bool directional, \
                    double speed, double length)
{
  int srcIndex = src.getId(); 
  int dstIndex = dst.getId();

  Edge e(src, dst, speed, length);
  mVertexList[srcIndex].mAdjacencyList.push_back(e);

  if (!directional) {
    Edge e2(dst, src, speed, length);
    mVertexList[dstIndex].mAdjacencyList.push_back(e2);
  }
	return mVertexList[srcIndex].mAdjacencyList.back();
}

Edge& Graph::addEdge(int iSrcId, int iDstId, bool directional, \
                    double speed, double length)
{
	Vertex& src = mVertexList[iSrcId];	
	Vertex& dst = mVertexList[iDstId];	

  Edge e(src, dst, speed, length);
  src.mAdjacencyList.push_back(e);

  if (!directional) {
    Edge e2(dst, src, speed, length);
    dst.mAdjacencyList.push_back(e2);
  }
	return src.mAdjacencyList.back();
}

void Graph::edgeEvent(Edge& e, bool event)
{
	e.setEvent(event);
}

Vertex Graph::vertex(PointOfInterest iPoi)
{
  for(size_t i = 0; i < mVertexList.size(); ++i) {
    if (mVertexList[i].getPoi() == iPoi) {
      return mVertexList[i];
    }
  }
  Vertex v;
  cout << "Could not find vertex with point-of-interest " << iPoi \
       << endl;
  return v;
}

void Graph::trip(Vertex src, Vertex dst)
{
  
}

void Graph::store(string filename)
{
}

void Graph::retrieve(string filename)
{
  fstream infile(filename, ios::in);
  int wNodeXId, wNodeYId, wDirection, wType, wEvent;
  double wLength, wSpeed;
  string wNodeName, wPoi;

  infile >> mNumVertices >> mNumEdges;

#if DEBUG
  cout << "Vertices: " << mNumVertices \
       << " , Edges: " << mNumEdges << endl;
#endif

  for (int i = 0; i < mNumVertices; ++i) {
    int wNodeId;
    infile >> wNodeId >> wNodeName >> wType;
    //cout << "ID: " << wNodeId << " " << wNodeName << " " << wType << endl;
    if (wType == POI || wType == POI_AND_INTERSECTION) {
      infile >> wPoi;
      Vertex& v = addVertex(POI, wNodeName);
      v.setPoi(wPoi);
      v.setId(wNodeId);
    } else {
      Vertex& v = addVertex(INTERSECTION, wNodeName);
      v.setId(wNodeId);
    }
  }
  for (int i = 0; i < mNumEdges; ++i) {
		infile >> wNodeXId >> wNodeYId >> wDirection \
				   >> wSpeed >> wLength >> wEvent;
		//printf("%d %d %d %.2f %.2f %d\n", wNodeXId, wNodeYId, wDirection, wSpeed, wLength, wEvent);
		Edge& e = addEdge(wNodeXId, wNodeYId, wDirection, \
											wSpeed, wLength);
		//e.printInfo();
		edgeEvent(e, wEvent);
  }
}

void Graph::printEdges() {
}

void Graph::printGraph()
{
  for (int i = 0; i < mNumVertices; ++i) {
    cout << "Node id = " << i << "|" << mVertexList[i].getName() << " has neighbours:" << endl;
    for (size_t j = 0; j < mVertexList[i].mAdjacencyList.size(); ++j) {
      Edge wEdge = mVertexList[i].mAdjacencyList[j];
				cout << "\t";
				wEdge.printInfo();
//      cout << "\tNode id = " << wEdge.getDestination().getId() << \
        " w/ speed = " << wEdge.getSpeed() << " & length = " << \
        wEdge.getLength() << endl;
    }

  }
}
