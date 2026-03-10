#include "HeatTransfer.h"
#include <iostream>
#include <fstream>
#include <iomanip>

HeatTransferSolver::HeatTransferSolver(const MeshTopology& mesh, ThermalProps props)
    : m_mesh(mesh), m_props(props) {}

void HeatTransferSolver::addFixedTemperature(int vIdx, double temp) {
    m_bcs.push_back({vIdx, true, temp, 0.0});
}

void HeatTransferSolver::addHeatFlux(int vIdx, double flux) {
    m_bcs.push_back({vIdx, false, 0.0, flux});
}

std::vector<int> HeatTransferSolver::getFaceVertices(int faceIndex) const {
    std::vector<int> vList;
    const Face &f = m_mesh.faces[faceIndex];
    for (const EdgeRef &ref : f.edgeRefs) {
        vList.push_back(ref.orientation == 0 ? m_mesh.edges[ref.edgeIndex].v1 : m_mesh.edges[ref.edgeIndex].v2);
    }
    return vList;
}

Eigen::Matrix3d HeatTransferSolver::computeThermalStiffness(const Vertex& v1, const Vertex& v2, const Vertex& v3) {
    Eigen::Vector3d p1(v1.x, v1.y, v1.z);
    Eigen::Vector3d p2(v2.x, v2.y, v2.z);
    Eigen::Vector3d p3(v3.x, v3.y, v3.z);

    //pojection of triangles to 2D plane
    Eigen::Vector3d Vx = (p2 - p1).normalized();
    Eigen::Vector3d Vz = Vx.cross(p3 - p1).normalized();
    Eigen::Vector3d Vy = Vz.cross(Vx).normalized();

    double x1 = 0.0, y1 = 0.0;
    double x2 = (p2 - p1).dot(Vx), y2 = 0.0;
    double x3 = (p3 - p1).dot(Vx), y3 = (p3 - p1).dot(Vy);

    double Area = 0.5 * std::abs(x2 * y3);
    if (Area < 1e-9) return Eigen::Matrix3d::Zero();

    //thermal gradient matrix (B) - 2x3
    Eigen::MatrixXd B(2, 3);
    B << y2 - y3, y3 - y1, y1 - y2,
        x3 - x2, x1 - x3, x2 - x1;
    B /= (2.0 * Area);

    //conductivity matrix (D) - isotropic
    Eigen::Matrix2d D = Eigen::Matrix2d::Identity() * m_props.thermalConductivity;

    //Kt = B^T * D * B * Area * thickness
    return B.transpose() * D * B * Area * m_props.thickness;
}

bool HeatTransferSolver::solve() {
    int numNodes = m_mesh.vertices.size();
    if (numNodes == 0) return false;

    //1 dof per node (for temperature)
    m_temperatures.assign(numNodes, 0.0);

    std::vector<Eigen::Triplet<double>> triplets;
    Eigen::VectorXd F = Eigen::VectorXd::Zero(numNodes);

    //global stiffness matrix
    for (size_t fIdx = 0; fIdx < m_mesh.faces.size(); ++fIdx) {
        std::vector<int> vList = getFaceVertices(fIdx);
        if (vList.size() < 3) continue;

        for (size_t i = 1; i + 1 < vList.size(); ++i) {
            int i1 = vList[0], i2 = vList[i], i3 = vList[i + 1];

            Eigen::Matrix3d Ke = computeThermalStiffness(m_mesh.vertices[i1], m_mesh.vertices[i2], m_mesh.vertices[i3]);

            std::vector<int> indices = {i1, i2, i3};
            for (int r = 0; r < 3; ++r) {
                for (int c = 0; c < 3; ++c) {
                    triplets.push_back(Eigen::Triplet<double>(indices[r], indices[c], Ke(r, c)));
                }
            }
        }
    }

    //for numerical stabilization
    double stabilization = m_props.thermalConductivity * 1e-6;
    for (int i = 0; i < numNodes; ++i) {
        triplets.push_back(Eigen::Triplet<double>(i, i, stabilization));
    }

    Eigen::SparseMatrix<double> K(numNodes, numNodes);
    K.setFromTriplets(triplets.begin(), triplets.end());

    //boundary conditions (penalty method)
    double penalty = m_props.thermalConductivity * 1e9;

    for (const auto& bc : m_bcs) {
        if (bc.isFixedTemperature) {
            K.coeffRef(bc.vertexIndex, bc.vertexIndex) += penalty;
            F(bc.vertexIndex) += bc.temperature * penalty;
        } else {
            F(bc.vertexIndex) += bc.heatFlux;
        }
    }

    //solver here
    Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver;
    solver.compute(K);
    if (solver.info() != Eigen::Success) {
        std::cerr << "Thermal Solver Matrix Factorization Failed.\n";
        return false;
    }

    Eigen::VectorXd T = solver.solve(F);
    if (solver.info() != Eigen::Success) {
        std::cerr << "Thermal Solver Solve Failed.\n";
        return false;
    }

    for (int i = 0; i < numNodes; ++i) {
        m_temperatures[i] = T(i);
        //std::cout << T(i) << std::endl;
    }

    exportToCSV("thermal_results.csv");

    return true;
}


bool HeatTransferSolver::exportToCSV(const std::string& filename) const {
    if (m_temperatures.empty()) {
        std::cerr << "No temperature data to export. Run solve() first.\n";
        return false;
    }

    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << "\n";
        return false;
    }

    file << "Node_ID,X,Y,Z,Temperature\n";

    file << std::fixed << std::setprecision(6);

    for (size_t i = 0; i < m_mesh.vertices.size(); ++i) {
        const Vertex& v = m_mesh.vertices[i];
        file << i << ","
             << v.x << ","
             << v.y << ","
             << v.z << ","
             << m_temperatures[i] << "\n";
    }

    file.close();
    std::cout << "Successfully exported thermal results to " << filename << "\n";
    return true;
}
