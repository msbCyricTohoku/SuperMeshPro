#pragma once
#include "MeshTopology.h"

class SubdivisionAlgorithms {
public:
  static void catmullClark(const MeshTopology &oldMesh, MeshTopology &newMesh,
                           MeshTopology &limitMesh);
  static void dooSabin(const MeshTopology &oldMesh, MeshTopology &newMesh);
  static void loop(const MeshTopology &oldMesh, MeshTopology &newMesh,
                   MeshTopology &limitMesh);
  //this relaxes mesh laplacian smooth
  static void laplacianSmooth(MeshTopology &mesh, double factor, int iterations);

  static std::vector<int> getFaceVertices(const MeshTopology &mesh,
                                          int faceIdx);

private:

};
