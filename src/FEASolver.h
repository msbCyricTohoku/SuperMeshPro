#pragma once
#include "MeshTopology.h"
#include <vector>
#include <Eigen/Sparse>
#include <Eigen/Dense>

struct MaterialProps {
    double youngsModulus;
    double poissonRatio;
    double thickness;
    double density;
};

struct BoundaryCondition {
    int vertexIndex;
    bool fixX, fixY, fixZ, fixRx, fixRy, fixRz;
    double forceX, forceY, forceZ, momentX, momentY, momentZ;
};

struct StepData {
    double appliedForceFraction;
    double maxDisplacement;
};

class FEASolver {
public:
    FEASolver(MeshTopology& mesh, MaterialProps mat);

    void addConstraint(int vIdx, bool fx, bool fy, bool fz, bool frx=false, bool fry=false, bool frz=false);
    void addForce(int vIdx, double fx, double fy, double fz, double mx=0, double my=0, double mz=0);

    bool solve(bool useGravity, bool useNonLinear);

    //returns 0.0 to 1.0 mapped values for heatmap
    std::vector<double> calculateVonMisesStresses();

    Eigen::VectorXd getDisplacements() const { return m_displacements; }

    bool exportValidationDataCSV(const std::string& filepath);
    std::vector<StepData> getForceDisplacementCurve() const { return m_history; }

    std::vector<StepData> m_history;

private:
    MeshTopology& m_mesh;
    MaterialProps m_mat;
    std::vector<BoundaryCondition> m_bcs;
    Eigen::VectorXd m_displacements;

    //18x18 shell element matrix
    Eigen::MatrixXd computeElementStiffness(const Vertex& v1, const Vertex& v2, const Vertex& v3);

    //stress
    std::vector<double> getRawStresses();
};
