#pragma once
#include "MeshTopology.h"
#include <Eigen/Dense>
#include <vector>
#include <random>

struct Ray {
    Eigen::Vector3d origin;
    Eigen::Vector3d direction;

    Ray(const Eigen::Vector3d& o, const Eigen::Vector3d& d)
        : origin(o), direction(d.normalized()) {}
};

struct HitRecord {
    double t;
    Eigen::Vector3d point;
    Eigen::Vector3d normal;
    int faceIndex;
    int v0_idx, v1_idx, v2_idx;
    double u, v; //barycentric coordinates added here
};

class RayTracerOptic {
public:
    RayTracerOptic(const MeshTopology& mesh);

    //std::vector<Eigen::Vector3d> simulateLightBeam(const Ray& initialRay, int maxBounces, std::vector<double>& nodalEnergies);

   // std::vector<std::vector<Eigen::Vector3d>> simulateBeamSwarm(
    //    const Ray& centerRay, int numRays, double spreadAngleDegrees, int maxBounces, std::vector<double>& nodalEnergies);

    std::vector<Eigen::Vector3d> simulateLightBeam(const Ray& initialRay, int maxBounces, std::vector<double>& nodalEnergies, double initialEnergy, double reflectivity);

    std::vector<std::vector<Eigen::Vector3d>> simulateBeamSwarm(
        const Ray& centerRay, int numRays, double spreadAngleDegrees, int maxBounces, std::vector<double>& nodalEnergies, double initialEnergy, double reflectivity);

    //export the accumulated energy on each vertex to a CSV file
    bool exportEnergiesToCSV(const std::string& filename, const std::vector<double>& nodalEnergies) const;

    //export the ray paths (bounces) for 3D visualization to a CSV file
    bool exportPathsToCSV(const std::string& filename, const std::vector<std::vector<Eigen::Vector3d>>& allPaths) const;

private:
    const MeshTopology& m_mesh;

    bool rayTriangleIntersect(const Ray& ray, const Eigen::Vector3d& v0,
                              const Eigen::Vector3d& v1, const Eigen::Vector3d& v2,
                              HitRecord& rec) const;

    bool hitClosest(const Ray& ray, HitRecord& rec) const;
    std::vector<int> getFaceVertices(int faceIndex) const;
};
