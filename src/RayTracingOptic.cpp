#include "RayTracingOptic.h"
#include <iostream>
#include <fstream>
#include <iomanip>

RayTracerOptic::RayTracerOptic(const MeshTopology& mesh) : m_mesh(mesh) {}

std::vector<int> RayTracerOptic::getFaceVertices(int faceIndex) const {
    std::vector<int> vList;
    const Face &f = m_mesh.faces[faceIndex];
    for (const EdgeRef &ref : f.edgeRefs) {
        vList.push_back(ref.orientation == 0 ? m_mesh.edges[ref.edgeIndex].v1 : m_mesh.edges[ref.edgeIndex].v2);
    }
    return vList;
}

bool RayTracerOptic::rayTriangleIntersect(const Ray& ray, const Eigen::Vector3d& v0,
                                          const Eigen::Vector3d& v1, const Eigen::Vector3d& v2,
                                          HitRecord& rec) const {

    const double EPSILON = 1e-8;

    Eigen::Vector3d edge1 = v1 - v0;

    Eigen::Vector3d edge2 = v2 - v0;


    Eigen::Vector3d h = ray.direction.cross(edge2);

    double a = edge1.dot(h);

    if (a > -EPSILON && a < EPSILON) return false;

    double f = 1.0 / a;
    Eigen::Vector3d s = ray.origin - v0;
    double u = f * s.dot(h);

    if (u < 0.0 || u > 1.0) return false;

    Eigen::Vector3d q = s.cross(edge1);
    double v = f * ray.direction.dot(q);

    if (v < 0.0 || u + v > 1.0) return false;

    double t = f * edge2.dot(q);

    if (t > EPSILON) {
        rec.t = t;
        rec.point = ray.origin + ray.direction * t;
        rec.normal = edge1.cross(edge2).normalized();

        //barycentric u, v for exact energy distribution
        rec.u = u;
        rec.v = v;
        return true;
    }
    return false;
}

bool RayTracerOptic::hitClosest(const Ray& ray, HitRecord& rec) const {
    bool hitAnything = false;
    double closestSoFar = 1e9;
    HitRecord tempRec;

    for (size_t i = 0; i < m_mesh.faces.size(); ++i) {
        std::vector<int> vList = getFaceVertices(i);
        if (vList.size() < 3) continue;

        for (size_t j = 1; j + 1 < vList.size(); ++j) {
            Eigen::Vector3d v0(m_mesh.vertices[vList[0]].x, m_mesh.vertices[vList[0]].y, m_mesh.vertices[vList[0]].z);
            Eigen::Vector3d v1(m_mesh.vertices[vList[j]].x, m_mesh.vertices[vList[j]].y, m_mesh.vertices[vList[j]].z);
            Eigen::Vector3d v2(m_mesh.vertices[vList[j+1]].x, m_mesh.vertices[vList[j+1]].y, m_mesh.vertices[vList[j+1]].z);

            if (rayTriangleIntersect(ray, v0, v1, v2, tempRec)) {
                if (tempRec.t < closestSoFar) {
                    closestSoFar = tempRec.t;
                    rec = tempRec;
                    rec.faceIndex = i;
                    rec.v0_idx = vList[0];
                    rec.v1_idx = vList[j];
                    rec.v2_idx = vList[j+1];
                    hitAnything = true;
                }
            }
        }
    }
    return hitAnything;
}

std::vector<Eigen::Vector3d> RayTracerOptic::simulateLightBeam(const Ray& initialRay, int maxBounces, std::vector<double>& nodalEnergies, double initialEnergy, double reflectivity) {
    std::vector<Eigen::Vector3d> path;
    path.push_back(initialRay.origin);

    Ray currentRay = initialRay;

    //user defined energy
    double currentEnergy = initialEnergy;


    for (int bounce = 0; bounce <= maxBounces; ++bounce) {
        HitRecord rec;

        if (hitClosest(currentRay, rec)) {
            path.push_back(rec.point);

            double w0 = 1.0 - rec.u - rec.v;
            double w1 = rec.u;
            double w2 = rec.v;

#pragma omp atomic
            nodalEnergies[rec.v0_idx] += w0 * currentEnergy;
#pragma omp atomic
            nodalEnergies[rec.v1_idx] += w1 * currentEnergy;
#pragma omp atomic
            nodalEnergies[rec.v2_idx] += w2 * currentEnergy;

            currentEnergy *= reflectivity;
            if (currentEnergy < (initialEnergy * 0.01)) break; //cutoff is ray falls below threshold weak

            Eigen::Vector3d I = currentRay.direction;
            Eigen::Vector3d N = rec.normal;
            if (I.dot(N) > 0) N = -N;

            thread_local std::mt19937 generator(std::random_device{}());
            std::uniform_real_distribution<double> dist(-1.0, 1.0);

            Eigen::Vector3d randomInSphere;
            do {
                randomInSphere = Eigen::Vector3d(dist(generator), dist(generator), dist(generator));
            } while (randomInSphere.squaredNorm() >= 1.0);

            Eigen::Vector3d R = (N + randomInSphere).normalized();
            Eigen::Vector3d newOrigin = rec.point + (N * 1e-4);

            currentRay = Ray(newOrigin, R);

        } else {
            path.push_back(currentRay.origin + currentRay.direction * 50.0);
            break;
        }
    }
    return path;
}

std::vector<std::vector<Eigen::Vector3d>> RayTracerOptic::simulateBeamSwarm(
    const Ray &centerRay, int numRays, double spreadAngleDegrees, int maxBounces, std::vector<double>& nodalEnergies, double initialEnergy, double reflectivity)
{
    std::vector<std::vector<Eigen::Vector3d>> allPaths(numRays);
    double spreadRadians = spreadAngleDegrees * (M_PI / 180.0);

    Eigen::Vector3d w = centerRay.direction.normalized();
    Eigen::Vector3d a = (std::abs(w.x()) > 0.9) ? Eigen::Vector3d(0, 1, 0) : Eigen::Vector3d(1, 0, 0);
    Eigen::Vector3d u = w.cross(a).normalized();
    Eigen::Vector3d v = w.cross(u).normalized();

    std::cout << "Shooting " << numRays << " rays across CPU threads...\n";

#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < numRays; ++i) {

        thread_local std::mt19937 generator(std::random_device{}());
        std::uniform_real_distribution<double> dist(-1.0, 1.0);

        double randU, randV, r2;
        do {
            randU = dist(generator);
            randV = dist(generator);
            r2 = randU * randU + randV * randV;
        } while (r2 >= 1.0);

        double scale = std::tan(spreadRadians);
        Eigen::Vector3d jitterDir = w + (u * randU * scale) + (v * randV * scale);

        Ray jitteredRay(centerRay.origin, jitterDir.normalized());

        //allPaths[i] = simulateLightBeam(jitteredRay, maxBounces, nodalEnergies);
        allPaths[i] = simulateLightBeam(jitteredRay, maxBounces, nodalEnergies, initialEnergy, reflectivity);
    }

    std::cout << "Ray transport computation complete!\n";


    exportEnergiesToCSV("energy_heatmap.csv", nodalEnergies);
    exportPathsToCSV("ray_paths.csv", allPaths);

    return allPaths;
}


//export the accumulated energy on each vertex
bool RayTracerOptic::exportEnergiesToCSV(const std::string& filename, const std::vector<double>& nodalEnergies) const {
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    file << "VertexID,X,Y,Z,Energy\n";
    for (size_t i = 0; i < m_mesh.vertices.size(); ++i) {
        //only log vertices that actually received energy to save file space
        if (nodalEnergies[i] > 0.0) {
            file << i << ","
                 << m_mesh.vertices[i].x << ","
                 << m_mesh.vertices[i].y << ","
                 << m_mesh.vertices[i].z << ","
                 << std::fixed << std::setprecision(6) << nodalEnergies[i] << "\n";
        }
    }
    file.close();
    return true;
}

//export the ray paths for 3D visualization
bool RayTracerOptic::exportPathsToCSV(const std::string& filename, const std::vector<std::vector<Eigen::Vector3d>>& allPaths) const {
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    file << "RayID,BounceIndex,X,Y,Z\n";
    for (size_t rayID = 0; rayID < allPaths.size(); ++rayID) {
        for (size_t bounce = 0; bounce < allPaths[rayID].size(); ++bounce) {
            file << rayID << ","
                 << bounce << ","
                 << std::fixed << std::setprecision(6) << allPaths[rayID][bounce].x() << ","
                 << allPaths[rayID][bounce].y() << ","
                 << allPaths[rayID][bounce].z() << "\n";
        }
    }
    file.close();
    return true;
}
