#pragma once

#include <string>
#include <vector>

//edge within a specific face
struct EdgeRef {
  int edgeIndex = -1;
  int orientation = 0; // 0 for forward, 1 for backward
  int adjFaceIndex = -1;
  int adjEdgeIndex = -1;
};

class Vertex {
public:
  //double x = 0.0, y = 0.0, z = 0.0;
  //double nx = 0.0, ny = 0.0, nz = 0.0;

  double x, y, z;
  double nx, ny, nz;

  //0 = smooth, 1 = dart, 2 = crease, 3/4 = corner/cusp
  int sharpness = 0;

  std::vector<int> faceIndices;
  std::vector<int> edgeIndices;

  bool isSelected = false;
  bool isAnchored = false; //for fixed boundary condition

  Vertex() = default;
  Vertex(double _x, double _y, double _z, int _sharpness = 0)
      : x(_x), y(_y), z(_z), sharpness(_sharpness), isSelected(false) {}

  void addFace(int index);
  void removeFace(int index);
};

class Edge {
public:
  int v1 = -1;
  int v2 = -1;
  int sharpness = 0; //0 = smooth, >=1 = crease
  int midpointIndex = -1;

  std::vector<int> faceIndices;

  Edge() = default;
  Edge(int _v1, int _v2) : v1(_v1), v2(_v2) {}
};

class Face {
public:
  int centerVertexIndex = -1;
  std::vector<EdgeRef> edgeRefs;

  Face() = default;
  void addEdgeRef(int edgeIdx, int orientation, int adjFaceIdx, int adjEdgeIdx);
};

class MeshTopology {
public:
  std::string meshName;
  std::vector<Vertex> vertices;
  std::vector<Edge> edges;
  std::vector<Face> faces;

  MeshTopology() = default;

  bool loadFromFile(const std::string &filepath);
  void normalize();
  void copyTo(MeshTopology &destination) const;

  //topology functions
  int findCommonFace(const Vertex &a, const Vertex &b, int excludeFace) const;
  int findEdge(int v1, int v2) const;
  int searchFaceEdge(Face &f, int edgeIdx, int targetFaceIdx,
                     int targetEdgeIdx);

  void addVertex(double x, double y, double z, int sharpness = 0);
  void addEdge(int v1, int v2);
};
