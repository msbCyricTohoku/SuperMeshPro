#include "SubdivisionAlgorithms.h"
#include <cmath>
#include <iostream>
#include <map>
#include <algorithm>
#include <vector>
#include <omp.h> //for openmp easier compared to mpi

constexpr double PI = 3.14159265358979323846;


static void addEdgeToFace(MeshTopology &mesh, Face &face, int faceIdx, int vA, int vB, int sharpness = 0) {
    int eIdx = mesh.findEdge(vA, vB);

    if (eIdx == static_cast<int>(mesh.edges.size())) {
        mesh.addEdge(vA, vB);
        mesh.edges.back().sharpness = sharpness;
    } else if (sharpness > mesh.edges[eIdx].sharpness) {
        mesh.edges[eIdx].sharpness = sharpness;
    }

    int orientation = (mesh.edges[eIdx].v1 == vA) ? 0 : 1;
    face.addEdgeRef(eIdx, orientation, -1, -1);

    auto& adjFaces = mesh.edges[eIdx].faceIndices;
    if (std::find(adjFaces.begin(), adjFaces.end(), faceIdx) == adjFaces.end()) {
        adjFaces.push_back(faceIdx);
    }

    mesh.vertices[vA].addFace(faceIdx);
    mesh.vertices[vB].addFace(faceIdx);
}

std::vector<int> SubdivisionAlgorithms::getFaceVertices(const MeshTopology &mesh, int faceIdx) {
    std::vector<int> vertices;
    const Face &f = mesh.faces[faceIdx];
    vertices.reserve(f.edgeRefs.size());

    for (const EdgeRef &ref : f.edgeRefs) {
        vertices.push_back(ref.orientation == 0 ? mesh.edges[ref.edgeIndex].v1
                                                : mesh.edges[ref.edgeIndex].v2);
    }
    return vertices;
}

//-----------------------------------------------------------------------------
//Catmull-Clark
//-----------------------------------------------------------------------------
void SubdivisionAlgorithms::catmullClark(const MeshTopology &oldMesh,
                                         MeshTopology &newMesh,
                                         MeshTopology &limitMesh) {
    (void)limitMesh;

    const int facePtsStart = 0;
    const int edgePtsStart = static_cast<int>(oldMesh.faces.size());
    const int vertPtsStart = edgePtsStart + static_cast<int>(oldMesh.edges.size());

    //generate face points (average of all vertices in a face)
    for (size_t i = 0; i < oldMesh.faces.size(); ++i) {
        std::vector<int> polyVerts = getFaceVertices(oldMesh, i);
        double fX = 0.0, fY = 0.0, fZ = 0.0;

        for (int vIdx : polyVerts) {
            fX += oldMesh.vertices[vIdx].x;
            fY += oldMesh.vertices[vIdx].y;
            fZ += oldMesh.vertices[vIdx].z;
        }

        double invCount = 1.0 / polyVerts.size();
        newMesh.addVertex(fX * invCount, fY * invCount, fZ * invCount, 0);
    }

    //generate edge points
    for (const Edge &e : oldMesh.edges) {
        double midX = (oldMesh.vertices[e.v1].x + oldMesh.vertices[e.v2].x) * 0.5;
        double midY = (oldMesh.vertices[e.v1].y + oldMesh.vertices[e.v2].y) * 0.5;
        double midZ = (oldMesh.vertices[e.v1].z + oldMesh.vertices[e.v2].z) * 0.5;

        //smooth edges connected to exactly two faces
        if (e.sharpness == 0 && e.faceIndices.size() == 2) {
            Vertex fPt1 = newMesh.vertices[facePtsStart + e.faceIndices[0]];
            Vertex fPt2 = newMesh.vertices[facePtsStart + e.faceIndices[1]];

            double fMidX = (fPt1.x + fPt2.x) * 0.5;
            double fMidY = (fPt1.y + fPt2.y) * 0.5;
            double fMidZ = (fPt1.z + fPt2.z) * 0.5;

            newMesh.addVertex((midX + fMidX) * 0.5, (midY + fMidY) * 0.5, (midZ + fMidZ) * 0.5, 0);
        } else {
            //crease/boundary edges
            newMesh.addVertex(midX, midY, midZ, (e.sharpness >= 1) ? 2 : 0);
        }
    }

    //generate vertex points
    for (size_t i = 0; i < oldMesh.vertices.size(); ++i) {
        const Vertex &v = oldMesh.vertices[i];

        if (v.sharpness == 0 || v.sharpness == 1) { //smooth or dart
            double fSumX = 0, fSumY = 0, fSumZ = 0;
            double eSumX = 0, eSumY = 0, eSumZ = 0;

            for (int fIdx : v.faceIndices) {
                fSumX += newMesh.vertices[facePtsStart + fIdx].x;
                fSumY += newMesh.vertices[facePtsStart + fIdx].y;
                fSumZ += newMesh.vertices[facePtsStart + fIdx].z;
            }

            for (int eIdx : v.edgeIndices) {
                const Edge &e = oldMesh.edges[eIdx];
                eSumX += (oldMesh.vertices[e.v1].x + oldMesh.vertices[e.v2].x) * 0.5;
                eSumY += (oldMesh.vertices[e.v1].y + oldMesh.vertices[e.v2].y) * 0.5;
                eSumZ += (oldMesh.vertices[e.v1].z + oldMesh.vertices[e.v2].z) * 0.5;
            }

            double valFaces = static_cast<double>(v.faceIndices.size());
            double valEdges = static_cast<double>(v.edgeIndices.size());

            //V_new = (F/n + 2*E/n + (n-3)*V) / n
            double invN = 1.0 / valEdges;
            double Fx = fSumX / valFaces, Fy = fSumY / valFaces, Fz = fSumZ / valFaces;
            double Ex = eSumX * invN, Ey = eSumY * invN, Ez = eSumZ * invN;

            double newX = (Fx + 2.0 * Ex + (valEdges - 3.0) * v.x) * invN;
            double newY = (Fy + 2.0 * Ey + (valEdges - 3.0) * v.y) * invN;
            double newZ = (Fz + 2.0 * Ez + (valEdges - 3.0) * v.z) * invN;

            newMesh.addVertex(newX, newY, newZ, v.sharpness);

        } else if (v.sharpness == 2) { //crease
            double creaseX = v.x * 0.75;
            double creaseY = v.y * 0.75;
            double creaseZ = v.z * 0.75;
            int sharpNeighbors = 0;

            for (int eIdx : v.edgeIndices) {
                if (oldMesh.edges[eIdx].sharpness >= 1) {
                    int nIdx = (oldMesh.edges[eIdx].v1 == static_cast<int>(i)) ? oldMesh.edges[eIdx].v2 : oldMesh.edges[eIdx].v1;
                    creaseX += oldMesh.vertices[nIdx].x * 0.125;
                    creaseY += oldMesh.vertices[nIdx].y * 0.125;
                    creaseZ += oldMesh.vertices[nIdx].z * 0.125;
                    sharpNeighbors++;
                }
            }

            if (sharpNeighbors == 2) {
                newMesh.addVertex(creaseX, creaseY, creaseZ, 2);
            } else {
                newMesh.addVertex(v.x, v.y, v.z, 2);
            }
        } else { //le corner (sharpness == 3)
            newMesh.addVertex(v.x, v.y, v.z, v.sharpness);
        }
    }

    for (size_t fIdx = 0; fIdx < oldMesh.faces.size(); ++fIdx) {
        const Face &f = oldMesh.faces[fIdx];
        int faceCenterIdx = facePtsStart + static_cast<int>(fIdx);
        std::vector<int> polyVerts = getFaceVertices(oldMesh, fIdx);
        int sides = static_cast<int>(f.edgeRefs.size());

        for (int i = 0; i < sides; ++i) {
            int oldVertIdx = polyVerts[i];
            int e1Idx = f.edgeRefs[(i - 1 + sides) % sides].edgeIndex;
            int e2Idx = f.edgeRefs[i].edgeIndex;

            int edgePt1Idx = edgePtsStart + e1Idx;
            int edgePt2Idx = edgePtsStart + e2Idx;
            int newVertIdx = vertPtsStart + oldVertIdx;

            Face newQuad;
            int nextFaceId = static_cast<int>(newMesh.faces.size());

            addEdgeToFace(newMesh, newQuad, nextFaceId, faceCenterIdx, edgePt1Idx, 0);
            addEdgeToFace(newMesh, newQuad, nextFaceId, edgePt1Idx, newVertIdx, oldMesh.edges[e1Idx].sharpness);
            addEdgeToFace(newMesh, newQuad, nextFaceId, newVertIdx, edgePt2Idx, oldMesh.edges[e2Idx].sharpness);
            addEdgeToFace(newMesh, newQuad, nextFaceId, edgePt2Idx, faceCenterIdx, 0);

            newMesh.faces.push_back(newQuad);
        }
    }
}

//-----------------------------------------------------------------------------
//Doo-Sabin
//-----------------------------------------------------------------------------
void SubdivisionAlgorithms::dooSabin(const MeshTopology &oldMesh, MeshTopology &newMesh) {
    using FaceVertPair = std::pair<int, int>;
    std::map<FaceVertPair, int> faceVertMap;

    for (size_t fIdx = 0; fIdx < oldMesh.faces.size(); ++fIdx) {

        std::vector<int> polyVerts = getFaceVertices(oldMesh, fIdx);

        int sides = static_cast<int>(polyVerts.size());

        double cx = 0, cy = 0, cz = 0;
        for (int vIdx : polyVerts) {
            cx += oldMesh.vertices[vIdx].x;
             cy += oldMesh.vertices[vIdx].y;
            cz += oldMesh.vertices[vIdx].z;
        }
        cx /= sides; cy /= sides; cz /= sides;

        for (int j = 0; j < sides; ++j) {
            int prevV = polyVerts[(j - 1 + sides) % sides];
            int currV = polyVerts[j];
            int nextV = polyVerts[(j + 1) % sides];

            //1/8(Prev) + 1/8(Next) + 1/2(Curr) + 1/4(Center)
            double nx = (oldMesh.vertices[prevV].x + oldMesh.vertices[nextV].x) * 0.125 +
                        (oldMesh.vertices[currV].x * 0.5) + (cx * 0.25);
            double ny = (oldMesh.vertices[prevV].y + oldMesh.vertices[nextV].y) * 0.125 +
                        (oldMesh.vertices[currV].y * 0.5) + (cy * 0.25);
            double nz = (oldMesh.vertices[prevV].z + oldMesh.vertices[nextV].z) * 0.125 +
                        (oldMesh.vertices[currV].z * 0.5) + (cz * 0.25);

            newMesh.addVertex(nx, ny, nz, oldMesh.vertices[currV].sharpness);
            faceVertMap[{static_cast<int>(fIdx), currV}] = static_cast<int>(newMesh.vertices.size() - 1);
        }
    }

    for (size_t fIdx = 0; fIdx < oldMesh.faces.size(); ++fIdx) {
        Face fFace;

    int nextFaceId = static_cast<int>(newMesh.faces.size());

        std::vector<int> polyVerts = getFaceVertices(oldMesh, fIdx);

        for (size_t j = 0; j < polyVerts.size(); ++j) {
            int nA = faceVertMap[{static_cast<int>(fIdx), polyVerts[j]}];
            int nB = faceVertMap[{static_cast<int>(fIdx), polyVerts[(j + 1) % polyVerts.size()]}];
            addEdgeToFace(newMesh, fFace, nextFaceId, nA, nB, 0);
        }
        newMesh.faces.push_back(fFace);
    }


    for (const Edge &e : oldMesh.edges) {
        if (e.faceIndices.size() == 2) {

            int fA = e.faceIndices[0], fB = e.faceIndices[1];
            int nA1 = faceVertMap[{fA, e.v1}], nA2 = faceVertMap[{fA, e.v2}];
            int nB2 = faceVertMap[{fB, e.v2}], nB1 = faceVertMap[{fB, e.v1}];

            Face eFace;

            int nextFaceId = static_cast<int>(newMesh.faces.size());

            addEdgeToFace(newMesh, eFace, nextFaceId, nA1, nA2, 0);
            addEdgeToFace(newMesh, eFace, nextFaceId, nA2, nB2, 0);
            addEdgeToFace(newMesh, eFace, nextFaceId, nB2, nB1, 0);
            addEdgeToFace(newMesh, eFace, nextFaceId, nB1, nA1, 0);

            newMesh.faces.push_back(eFace);

        }
    }

    for (size_t i = 0; i < oldMesh.vertices.size(); ++i) {
        const Vertex &v = oldMesh.vertices[i];
        if (v.faceIndices.empty()) continue;

        std::vector<int> cycle;
        int currFace = v.faceIndices[0], currEdge = -1;

        for (int eIdx : v.edgeIndices) {
            auto& fInds = oldMesh.edges[eIdx].faceIndices;
            if (std::find(fInds.begin(), fInds.end(), currFace) != fInds.end()) {
                currEdge = eIdx;
                break;
            }
        }
        if (currEdge == -1) continue;

        int startEdge = currEdge;
        do {
            cycle.push_back(currFace);
            const Edge &e = oldMesh.edges[currEdge];
            int nextFace = -1;

            for (int fIdx : e.faceIndices) {
                if (fIdx != currFace) { nextFace = fIdx; break; }
            }
            if (nextFace == -1) break;

            currFace = nextFace;
            int nextEdge = -1;
            for (int eIdx : v.edgeIndices) {
                auto& fInds = oldMesh.edges[eIdx].faceIndices;
                if (eIdx != currEdge && std::find(fInds.begin(), fInds.end(), currFace) != fInds.end()) {
                    nextEdge = eIdx; break;
                }
            }
            currEdge = nextEdge;
        } while (currEdge != -1 && currEdge != startEdge);

        if (cycle.size() >= 3) {
            Face vFace;
            int nextFaceId = static_cast<int>(newMesh.faces.size());
            for (size_t k = 0; k < cycle.size(); ++k) {
                int nA = faceVertMap[{cycle[k], static_cast<int>(i)}];
                int nB = faceVertMap[{cycle[(k + 1) % cycle.size()], static_cast<int>(i)}];
                addEdgeToFace(newMesh, vFace, nextFaceId, nA, nB, 0);
            }
            newMesh.faces.push_back(vFace);
        }
    }

    for (Edge &e : newMesh.edges) {

        if (newMesh.vertices[e.v1].sharpness == 2 && newMesh.vertices[e.v2].sharpness == 2) {

          e.sharpness = 1;
        }
    }
}

//-----------------------------------------------------------------------------
//Loop
//-----------------------------------------------------------------------------
void SubdivisionAlgorithms::loop(const MeshTopology &oldMesh,
                                 MeshTopology &newMesh,
                                 MeshTopology &limitMesh) {
    (void)limitMesh;

    //ensure input geometry is triangulated
    MeshTopology triMesh;
    triMesh.meshName = oldMesh.meshName;
    for (const Vertex &v : oldMesh.vertices) {
        triMesh.addVertex(v.x, v.y, v.z, v.sharpness);
    }

    auto getOldSharpness = [&](int a, int b) {
        int eIdx = oldMesh.findEdge(a, b);
        return (eIdx < static_cast<int>(oldMesh.edges.size())) ? oldMesh.edges[eIdx].sharpness : 0;
    };

    for (size_t fIdx = 0; fIdx < oldMesh.faces.size(); ++fIdx) {
        std::vector<int> polyVerts = getFaceVertices(oldMesh, fIdx);
        for (size_t i = 1; i + 1 < polyVerts.size(); ++i) {
            Face newTri;
            int nextFaceId = static_cast<int>(triMesh.faces.size());
            int v0 = polyVerts[0], v1 = polyVerts[i], v2 = polyVerts[i + 1];

            addEdgeToFace(triMesh, newTri, nextFaceId, v0, v1, getOldSharpness(v0, v1));
            addEdgeToFace(triMesh, newTri, nextFaceId, v1, v2, getOldSharpness(v1, v2));
            addEdgeToFace(triMesh, newTri, nextFaceId, v2, v0, getOldSharpness(v2, v0));
            triMesh.faces.push_back(newTri);
        }
    }

    //vertex points calc
    for (size_t i = 0; i < triMesh.vertices.size(); ++i) {
        const Vertex &v = triMesh.vertices[i];
        if (v.sharpness == 0 || v.sharpness == 1) {
            int n = static_cast<int>(v.edgeIndices.size());
            if (n == 0) continue;

            //Loop beta weighting
            double temp = 0.375 + 0.25 * std::cos(2.0 * PI / n);
            double beta = (0.625 - (temp * temp)) / n;
            double centerWeight = 1.0 - (n * beta);

            double nx = centerWeight * v.x;
            double ny = centerWeight * v.y;
            double nz = centerWeight * v.z;

            for (int eIdx : v.edgeIndices) {

                int neighbor = (triMesh.edges[eIdx].v1 == static_cast<int>(i)) ? triMesh.edges[eIdx].v2 : triMesh.edges[eIdx].v1;

                nx += beta * triMesh.vertices[neighbor].x;

                ny += beta * triMesh.vertices[neighbor].y;

                nz += beta * triMesh.vertices[neighbor].z;

            }
            newMesh.addVertex(nx, ny, nz, v.sharpness);

        } else {
            newMesh.addVertex(v.x, v.y, v.z, v.sharpness);

        }
    }


    int edgeVertOffset = static_cast<int>(newMesh.vertices.size());

    for (const Edge &e : triMesh.edges) {

        Vertex v1 = triMesh.vertices[e.v1], v2 = triMesh.vertices[e.v2];

        if (e.sharpness != 1 && e.faceIndices.size() == 2) {
            auto getOpposite = [&](int fIdx, int ea, int eb) {
                for (int fv : getFaceVertices(triMesh, fIdx)) {
                    if (fv != ea && fv != eb) return fv;
                }
                return 0;
            };

            int v3 = getOpposite(e.faceIndices[0], e.v1, e.v2);
            int v4 = getOpposite(e.faceIndices[1], e.v1, e.v2);

            //smooth edge here as 3/8 -->endpoints, 1/8 --> opposites
            double ePtX = (v1.x + v2.x) * 0.375 + (triMesh.vertices[v3].x + triMesh.vertices[v4].x) * 0.125;
            double ePtY = (v1.y + v2.y) * 0.375 + (triMesh.vertices[v3].y + triMesh.vertices[v4].y) * 0.125;
            double ePtZ = (v1.z + v2.z) * 0.375 + (triMesh.vertices[v3].z + triMesh.vertices[v4].z) * 0.125;

            newMesh.addVertex(ePtX, ePtY, ePtZ, 0);
        } else {

            //crease
            newMesh.addVertex((v1.x + v2.x) * 0.5, (v1.y + v2.y) * 0.5, (v1.z + v2.z) * 0.5, (e.sharpness >= 1) ? 2 : 0);
        }
    }

    //mesh assemble (1 to 4 split per triangle)
    for (size_t i = 0; i < triMesh.faces.size(); ++i) {
        const Face &f = triMesh.faces[i];
        std::vector<int> tV = getFaceVertices(triMesh, static_cast<int>(i));

        int m0 = edgeVertOffset + f.edgeRefs[0].edgeIndex;
        int m1 = edgeVertOffset + f.edgeRefs[1].edgeIndex;
        int m2 = edgeVertOffset + f.edgeRefs[2].edgeIndex;

        int s0 = triMesh.edges[f.edgeRefs[0].edgeIndex].sharpness;
        int s1 = triMesh.edges[f.edgeRefs[1].edgeIndex].sharpness;
        int s2 = triMesh.edges[f.edgeRefs[2].edgeIndex].sharpness;

        auto createTri = [&](int a, int b, int c, int sharpAB, int sharpBC, int sharpCA) {
            Face newTri;
            int nextFaceId = static_cast<int>(newMesh.faces.size());
            addEdgeToFace(newMesh, newTri, nextFaceId, a, b, sharpAB);
            addEdgeToFace(newMesh, newTri, nextFaceId, b, c, sharpBC);
            addEdgeToFace(newMesh, newTri, nextFaceId, c, a, sharpCA);
            newMesh.faces.push_back(newTri);
        };

        createTri(tV[0], m0, m2, s0, 0, s2);
        createTri(tV[1], m1, m0, s1, 0, s0);
        createTri(tV[2], m2, m1, s2, 0, s1);
        createTri(m0, m1, m2, 0, 0, 0); // Center triangle
    }
}

//-----------------------------------------------------------------------------
//Laplacian Smoothening
//-----------------------------------------------------------------------------
void SubdivisionAlgorithms::laplacianSmooth(MeshTopology &mesh, double factor, int iterations) {
    for (int iter = 0; iter < iterations; ++iter) {
        //temp array for pos.
        std::vector<double> newX(mesh.vertices.size());

        std::vector<double> newY(mesh.vertices.size());

        std::vector<double> newZ(mesh.vertices.size());


    #pragma omp parallel for
        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
            const Vertex &v = mesh.vertices[i];

            //sharp mechanical corners/creases completely rigid
            if (v.sharpness >= 2) {
                newX[i] = v.x; newY[i] = v.y; newZ[i] = v.z;
                continue;
            }

            double cx = 0, cy = 0, cz = 0;
            int n = 0;

            //find the centroid of all connected neighbor vertices
            for (int eIdx : v.edgeIndices) {

                const Edge &e = mesh.edges[eIdx];

                int neighbor = (e.v1 == (int)i) ? e.v2 : e.v1;

                cx += mesh.vertices[neighbor].x;

                cy += mesh.vertices[neighbor].y;

                cz += mesh.vertices[neighbor].z;

                n++;
            }

            if (n > 0) {

                //move the vertex towards the centroid by the smoothing factor
                newX[i] = v.x + factor * ((cx / n) - v.x);

                newY[i] = v.y + factor * ((cy / n) - v.y);

                newZ[i] = v.z + factor * ((cz / n) - v.z);

            } else {
                newX[i] = v.x; newY[i] = v.y; newZ[i] = v.z;
            }
        }

        //update
        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        mesh.vertices[i].x = newX[i];
             mesh.vertices[i].y = newY[i];
            mesh.vertices[i].z = newZ[i];
        }
    }
}
