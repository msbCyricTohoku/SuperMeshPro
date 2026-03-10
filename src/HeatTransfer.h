#pragma once
#include "MeshTopology.h"
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <vector>
#include <string>

struct ThermalProps {
    double thermalConductivity; //k (W/m*K)
    double thickness;           //t (mm)
};

struct ThermalBC {
    int vertexIndex;
    bool isFixedTemperature;
    double temperature;
    double heatFlux;
};

class HeatTransferSolver {
public:
    HeatTransferSolver(const MeshTopology& mesh, ThermalProps props);

    void addFixedTemperature(int vIdx, double temp);
    void addHeatFlux(int vIdx, double flux);

    bool solve();
    std::vector<double> getTemperatures() const { return m_temperatures; }

    bool exportToCSV(const std::string& filename) const;

private:
    const MeshTopology& m_mesh;
    ThermalProps m_props;
    std::vector<ThermalBC> m_bcs;
    std::vector<double> m_temperatures;

    Eigen::Matrix3d computeThermalStiffness(const Vertex& v1, const Vertex& v2, const Vertex& v3);
    std::vector<int> getFaceVertices(int faceIndex) const;

};
