#include <fstream>
#include "Graph.h"
#include <string>
#include <iostream>
#include <functional>
#include <climits>
#include <queue>
#include <list>
#include <vector>
#include <fstream>
#include "Vertex.h"
#include "Edge.h"
#define DEBUG 0

struct node {
	int vertex;
	double weight;
	node(int v, double w) : vertex(v), weight(w) { };
	node() { }
};

class CompareGreater {
	public:
		bool const operator()(node &nodeX, node &nodeY) {
			return (nodeX.weight > nodeY.weight) ;
		}
};


vector<double> weights;
int select_weight = 0;
vector<int> direction;
vector<int> event;
priority_queue<node, vector<node>, CompareGreater> Q;
vector< list<node> > adj;


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


void Graph::store(string filename)
{
  fstream outfile(filename, ios::out);
  int wDirection = 1;
  outfile << mNumVertices << " " << mNumEdges << endl;
  for(size_t i = 0; i < mVertexList.size(); ++i) {
    Vertex wVertex = mVertexList[i];
    vertex_type wType = wVertex.getType();
    outfile << wVertex.getId() << " " << wVertex.getName() \
            << " " << wType;

    if (wType == POI || wType == POI_AND_INTERSECTION) {
      outfile << " " << wVertex.getPoi();
    }
    outfile << endl;
  }

  for(size_t i = 0; i < mVertexList.size(); ++i) {
    for(size_t j = 0; j < mVertexList[i].mAdjacencyList.size(); ++j) {
        Edge wEdge = mVertexList[i].mAdjacencyList[j];

      outfile << wEdge.getSource().getId() << " "     \
              << wEdge.getDestination().getId() << " "  \
              << wDirection << " " << wEdge.getSpeed() << " " \
              << wEdge.getLength() << " " << wEdge.getEvent() \
              << endl;
    }
  }
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
    cout << "Node id = " << mVertexList[i].getId() << "|" << mVertexList[i].getName() << " has neighbours:" << endl;
    for (size_t j = 0; j < mVertexList[i].mAdjacencyList.size(); ++j) {
      Edge wEdge = mVertexList[i].mAdjacencyList[j];
				cout << "\t";
				wEdge.printInfo();
/*     cout << "\tNode id = " << wEdge.getDestination().getId() << \
        " w/ speed = " << wEdge.getSpeed() << " & length = " << \
        wEdge.getLength() << endl;*/
    }

  }
}


void Graph::readData() {
	mNumEdges = 0;
  weights.clear();
  adj.clear();
  Q = priority_queue<node, vector<node>, CompareGreater>();
  direction.clear();
  event.clear();


	for(vector<Vertex>::const_iterator i = mVertexList.begin(); i!=  mVertexList.end();++i) {
		for(vector<Edge>::const_iterator j = i->mAdjacencyList.begin(); j != i->mAdjacencyList.end();++j) {
			if(j->getEvent() != 1)
				mNumEdges++;
		}
  }
	adj.resize(mNumVertices);
	weights.resize(mNumVertices);
	for (int i = 0; i < mNumVertices; ++i) {
		weights.at(i) = INT_MAX;
	}

	for(vector<Vertex>::const_iterator i = mVertexList.begin(); i!=  mVertexList.end();++i) {
	  for(vector<Edge>::const_iterator j = i->mAdjacencyList.begin(); j != i->mAdjacencyList.end();++j) {
			if(select_weight == 1) {
				if(j->getEvent() != 1)
					adj[(j->getSource()).getId()].push_back(node((j-> getDestination()).getId(), (j->getLength())));
			} else {
				if(j->getEvent() != 1)
					adj[(j->getSource()).getId()].push_back(node((j-> getDestination()).getId(), (j->getLength()/j->getSpeed())));
			}
		}
  }
}
/*void Graph::printPath(vector<int> wPath, int j)
{
    // Base Case : If j is source
    if (wPath[j]==-1)
        return;

    printPath(wPath, wPath[j]);

    cout<<j<<" ";
}*/

vector<int> Graph::trip(Vertex v1,Vertex v2) {
	readData();

	node startNode;
	node endNode;
	startNode.vertex = v1.getId();
	endNode.vertex = v2.getId();
  vector<int> parent;
  //int parent[100];
  vector<int> path;
  parent.resize(mNumVertices);
  parent[startNode.vertex] = -1;
	startNode.weight = 0;
	endNode.weight = 0;
	node currentNode;
	weights[startNode.vertex] = 0;
	Q.push(startNode);
	int l = 0;
  vector<int> s;
  s.resize(mNumVertices);
	while (!Q.empty()) {

		currentNode = Q.top();

		Q.pop();
    //priority_queue<node, vector<node>, CompareGreater> Q;
    //Q = priority_queue<node, vector<node>, CompareGreater>();
#if DEBUG
		cout << "the current node is " << currentNode.vertex << endl;
    for(list<node>::iterator it = adj[currentNode.vertex].begin(); it != adj[currentNode.vertex].end(); ++it)
        cout<<"The adjacency list for"<<it->vertex<<endl;
#endif
		if (currentNode.weight <= weights[currentNode.vertex]) {
			//cout<<currentNode.vertex<<" ";
			//wPath.push_back(currentNode.vertex);

			for (list<node>::iterator it = adj[currentNode.vertex].begin(); it != adj[currentNode.vertex].end(); ++it) {
				if (weights[it->vertex] > weights[currentNode.vertex] + it->weight) {
					weights[it->vertex] = weights[currentNode.vertex] + it->weight;
          parent[it->vertex] = currentNode.vertex;
					Q.push(node((it->vertex), weights[it->vertex]));
				}
			}
		}

    if(currentNode.vertex == endNode.vertex) {
      l = 1;
      break;
    }
	}

	if(l == 0) {
		cout<<"Path could not be found"<<endl;
	} else {
    //cout<<startNode.vertex<<"    ";

    //printPath(wPath, endNode.vertex);
     s[0] = endNode.vertex;

     //cout<<"the array wpath is"<<endl;
     //for(int k = 0; k< wPath.size();k++)
		   //     cout << wPath[k]<<" ";
     int i = 1;
		while(s[i-1]!= -1)
    {



      s[i] = parent[s[i-1]];
      i++;


    }

    for(int j=i-2;j>=0;j--)
    {
      //cout<<s[j]<<" ";
      path.push_back(s[j]);
    }


    //int end = endNode.vertex;
		//double w = weights.at(end);
		//cout << endl<<"the shortest length of path to the node " << endNode.vertex << " is " << w << endl;
}

  return path;
}
void Graph::road(vector<Edge> iEdges) {
  mRoads.push_back(iEdges);
}
