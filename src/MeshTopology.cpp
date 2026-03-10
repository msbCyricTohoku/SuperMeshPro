#include "MeshTopology.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream> //for string streams (parsing OBJ lines)
#include <string>  //for string manipulation

using namespace std;

//here face is added
void Vertex::addFace(int index) {
  if (find(faceIndices.begin(), faceIndices.end(), index) ==
      faceIndices.end()) {
    faceIndices.push_back(index);
    sort(faceIndices.begin(), faceIndices.end());
  }
}

void Vertex::removeFace(int index) {
  auto it = find(faceIndices.begin(), faceIndices.end(), index);
  if (it != faceIndices.end()) {
    faceIndices.erase(it);
  }
}

void Face::addEdgeRef(int edgeIdx, int orientation, int adjFaceIdx,
                      int adjEdgeIdx) {
  edgeRefs.push_back({edgeIdx, orientation, adjFaceIdx, adjEdgeIdx});
}


void MeshTopology::addVertex(double x, double y, double z, int sharpness) {
  vertices.emplace_back(x, y, z, sharpness);
}

void MeshTopology::addEdge(int v1, int v2) {
  edges.emplace_back(v1, v2);
  int newEdgeIdx = edges.size() - 1;
  vertices[v1].edgeIndices.push_back(newEdgeIdx);
  vertices[v2].edgeIndices.push_back(newEdgeIdx);
}

int MeshTopology::findCommonFace(const Vertex &a, const Vertex &b,
                                 int excludeFace) const {
  for (int fA : a.faceIndices) {
    if (fA == excludeFace)
      continue;
    for (int fB : b.faceIndices) {
      if (fA == fB)
        return fA;
    }
  }
  return -1;
}

int MeshTopology::findEdge(int v1, int v2) const {
  const auto &edgesA = vertices[v1].edgeIndices;
  const auto &edgesB = vertices[v2].edgeIndices;

  for (int eA : edgesA) {
    for (int eB : edgesB) {
      if (eA == eB)
        return eA;
    }
  }
  return edges.size();
}

int MeshTopology::searchFaceEdge(Face &f, int edgeIdx, int targetFaceIdx,
                                 int targetEdgeIdx) {
  for (int i = f.edgeRefs.size() - 1; i >= 0; --i) {
    if (f.edgeRefs[i].edgeIndex == edgeIdx) {
      f.edgeRefs[i].adjFaceIndex = targetFaceIdx;
      f.edgeRefs[i].adjEdgeIndex = targetEdgeIdx;
      return i;
    }
  }
  return -1;
}

void MeshTopology::copyTo(MeshTopology &dst) const {
  dst.meshName = meshName;
  dst.vertices = vertices;
  dst.edges = edges;
  dst.faces = faces;
}

//checked now okay it seems
bool MeshTopology::loadFromFile(const std::string &filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    vertices.clear();
    edges.clear();
    faces.clear();

    bool isOBJ = (filepath.length() >= 4 && filepath.substr(filepath.length() - 4) == ".obj");

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#' || line[0] == '\r') continue;

        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            double x, y, z;
            iss >> x >> y >> z;
            addVertex(x, y, z);
        }
        else if (type == "f") {
            std::vector<int> vIndices;
            std::string vData;
            while (iss >> vData) {

                size_t slashPos = vData.find('/');
                int idx = std::stoi(vData.substr(0, slashPos));
                vIndices.push_back(isOBJ ? idx - 1 : idx);
            }

            if (vIndices.size() < 3) continue;

            Face newFace;
            int faceIdx = faces.size();
            for (size_t j = 0; j < vIndices.size(); ++j) {
                int curr = vIndices[j];
                int next = vIndices[(j + 1) % vIndices.size()];

                int commonFace = findCommonFace(vertices[curr], vertices[next], faceIdx);
                vertices[curr].addFace(faceIdx);
                vertices[next].addFace(faceIdx);

                int edgeIdx = findEdge(curr, next);
                int orient = 0;

                if (edgeIdx == (int)edges.size()) addEdge(curr, next);
                else orient = (edges[edgeIdx].v1 == curr) ? 0 : 1;

                int adjEdgeIdx = (commonFace != -1) ? searchFaceEdge(faces[commonFace], edgeIdx, faceIdx, j) : -1;

                edges[edgeIdx].faceIndices.push_back(faceIdx);
                newFace.addEdgeRef(edgeIdx, orient, commonFace, adjEdgeIdx);
            }
            faces.push_back(newFace);
        }
        else if (type == "e") { //this is for sharpe edge definition in custom txt
            int v1, v2;
            iss >> v1 >> v2;
            int eIdx = findEdge(v1, v2);
            if (eIdx < (int)edges.size()) edges[eIdx].sharpness = 1;
        }
        else if (type == "s") { //this is for sharpe vertex definition in custom txt
            int vIdx, sVal;
            iss >> vIdx >> sVal;
            if (vIdx < (int)vertices.size()) vertices[vIdx].sharpness = sVal;
        }
    }
    return true;
}


void MeshTopology::normalize() {
    if (vertices.empty()) return;

    //detemine the bounding box limits
    double minX = vertices[0].x, maxX = vertices[0].x;
    double minY = vertices[0].y, maxY = vertices[0].y;
    double minZ = vertices[0].z, maxZ = vertices[0].z;

    for (const Vertex& v : vertices) {
        if (v.x < minX) minX = v.x;
        if (v.x > maxX) maxX = v.x;
        if (v.y < minY) minY = v.y;
        if (v.y > maxY) maxY = v.y;
        if (v.z < minZ) minZ = v.z;
        if (v.z > maxZ) maxZ = v.z;
    }

    //calculate the center of the bounding box
    double cx = (minX + maxX) / 2.0;
    double cy = (minY + maxY) / 2.0;
    double cz = (minZ + maxZ) / 2.0;

    //longest dimension (aka span)
    double spanX = maxX - minX;
    double spanY = maxY - minY;
    double spanZ = maxZ - minZ;

    double maxSpan = spanX;
    if (spanY > maxSpan) maxSpan = spanY;
    if (spanZ > maxSpan) maxSpan = spanZ;

    if (maxSpan == 0.0) return; //avoid division by zero

    //calculate the scale factor to make the largest side exactly 4.0 units
    double scale = 4.0 / maxSpan;

    for (Vertex& v : vertices) {
        v.x = (v.x - cx) * scale;
        v.y = (v.y - cy) * scale;
        v.z = (v.z - cz) * scale;
    }
}
