#include "FEASolver.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include "SubdivisionAlgorithms.h"
#include <fstream>
#include <iomanip>
#include <omp.h>
#include <Eigen/SparseLU>
#include <QMessageBox>
#include <QString>
#include <sstream>
#include <chrono>

FEASolver::FEASolver(MeshTopology& mesh, MaterialProps mat) : m_mesh(mesh), m_mat(mat) {}

void FEASolver::addConstraint(int vIdx, bool fx, bool fy, bool fz, bool frx, bool fry, bool frz) {
    m_bcs.push_back({vIdx, fx, fy, fz, frx, fry, frz, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0});
}

void FEASolver::addForce(int vIdx, double fx, double fy, double fz, double mx, double my, double mz) {
    m_bcs.push_back({vIdx, false, false, false, false, false, false, fx, fy, fz, mx, my, mz});
}

/*
Eigen::MatrixXd FEASolver::computeElementStiffness(const Vertex& v1, const Vertex& v2, const Vertex& v3) {
    Eigen::Vector3d p1(v1.x, v1.y, v1.z);
    Eigen::Vector3d p2(v2.x, v2.y, v2.z);
    Eigen::Vector3d p3(v3.x, v3.y, v3.z);

    Eigen::Vector3d Vx = (p2 - p1).normalized();
    Eigen::Vector3d Vz = Vx.cross(p3 - p1).normalized();
    Eigen::Vector3d Vy = Vz.cross(Vx).normalized();

    double x1 = 0.0, y1 = 0.0;
    double x2 = (p2 - p1).dot(Vx), y2 = 0.0;
    double x3 = (p3 - p1).dot(Vx), y3 = (p3 - p1).dot(Vy);
    double A = 0.5 * std::abs(x2 * y3);

    if (A < 1e-9) return Eigen::MatrixXd::Zero(18, 18);

    Eigen::MatrixXd Bm(3, 6);
    Bm << y2 - y3, 0.0,     y3 - y1, 0.0,     y1 - y2, 0.0,
        0.0,     x3 - x2, 0.0,     x1 - x3, 0.0,     x2 - x1,
        x3 - x2, y2 - y3, x1 - x3, y3 - y1, x2 - x1, y1 - y2;
    Bm /= (2.0 * A);

    Eigen::Matrix3d D;
    double E = m_mat.youngsModulus;
    double nu = m_mat.poissonRatio;
    D << 1.0, nu, 0.0, nu, 1.0, 0.0, 0.0, 0.0, (1.0 - nu)/2.0;
    D *= E / (1.0 - nu * nu);

    Eigen::MatrixXd Km = Bm.transpose() * D * Bm * A * m_mat.thickness;

    double dN1dx = Bm(0, 0), dN2dx = Bm(0, 2), dN3dx = Bm(0, 4);
    double dN1dy = Bm(1, 1), dN2dy = Bm(1, 3), dN3dy = Bm(1, 5);

    Eigen::MatrixXd Bb(3, 9);
    Bb.setZero();
    Bb(0, 2) = dN1dx;
    Bb(0, 5) = dN2dx;
    Bb(0, 8) = dN3dx;
    Bb(1, 1) = -dN1dy;
    Bb(1, 4) = -dN2dy;
    Bb(1, 7) = -dN3dy;
    Bb(2, 1) = -dN1dx;
    Bb(2, 2) = dN1dy;
    Bb(2, 4) = -dN2dx;
    Bb(2, 5) = dN2dy;
    Bb(2, 7) = -dN3dx;
    Bb(2, 8) = dN3dy;

    double t3 = std::pow(m_mat.thickness, 3);
    Eigen::Matrix3d Db = D * (t3 / 12.0);
    Eigen::MatrixXd K_bend = Bb.transpose() * Db * Bb * A;

    Eigen::MatrixXd Bs(2, 9);
    Bs.setZero();
    Bs(0, 0) = dN1dx;
    Bs(0, 3) = dN2dx;
    Bs(0, 6) = dN3dx;
    Bs(0, 2) = -1.0/3.0;
    Bs(0, 5) = -1.0/3.0;
    Bs(0, 8) = -1.0/3.0;
    Bs(1, 0) = dN1dy;
    Bs(1, 3) = dN2dy;
    Bs(1, 6) = dN3dy;
    Bs(1, 1) = 1.0/3.0;
    Bs(1, 4) = 1.0/3.0;
    Bs(1, 7) = 1.0/3.0;

    double G = E / (2.0 * (1.0 + nu));
    double k_shear = 5.0 / 6.0;

    //calculates a relaxation factor based on thickness squared vs area
    double alpha = (m_mat.thickness * m_mat.thickness) / (A + 1e-9);
    //if (alpha > 1.0) alpha = 1.0;
    //if (alpha < 0.005) alpha = 0.001;


    if (alpha < 1e-4) alpha = 1e-4;

    if (alpha > 0.015) alpha = 0.015;
    
   // double alpha = 0.001;
    //                                                                  shear matrix X by alpha
    Eigen::Matrix2d Ds = Eigen::Matrix2d::Identity() * (k_shear * G * m_mat.thickness * alpha);
    Eigen::MatrixXd K_shear = Bs.transpose() * Ds * Bs * A;

    Eigen::MatrixXd Kb = K_bend + K_shear;
    Eigen::MatrixXd K_local = Eigen::MatrixXd::Zero(18, 18);

    for (int i=0; i<3; ++i) {
        for (int j=0; j<3; ++j) {
            K_local(i*6, j*6)     = Km(i*2, j*2);
            K_local(i*6, j*6+1)   = Km(i*2, j*2+1);
            K_local(i*6+1, j*6)   = Km(i*2+1, j*2);
            K_local(i*6+1, j*6+1) = Km(i*2+1, j*2+1);

            for(int m=0; m<3; ++m) {
                for(int n=0; n<3; ++n) {
                    K_local(i*6+2+m, j*6+2+n) = Kb(i*3+m, j*3+n);
                }
            }
        }
        K_local(i*6+5, i*6+5) += E * m_mat.thickness * A * 1e-4;
    }

    Eigen::Matrix3d R;
    R << Vx.x(), Vx.y(), Vx.z(),
        Vy.x(), Vy.y(), Vy.z(),
        Vz.x(), Vz.y(), Vz.z();

    Eigen::MatrixXd T = Eigen::MatrixXd::Zero(18, 18);
    for(int i=0; i<6; ++i) T.block<3,3>(i*3, i*3) = R;

    return T.transpose() * K_local * T;
}
*/

Eigen::MatrixXd FEASolver::computeElementStiffness(const Vertex& v1, const Vertex& v2, const Vertex& v3) {
    Eigen::Vector3d p1(v1.x, v1.y, v1.z);
    Eigen::Vector3d p2(v2.x, v2.y, v2.z);
    Eigen::Vector3d p3(v3.x, v3.y, v3.z);

    //local coordinate
    Eigen::Vector3d Vx = (p2 - p1).normalized();
    Eigen::Vector3d Vz = Vx.cross(p3 - p1).normalized();
    Eigen::Vector3d Vy = Vz.cross(Vx).normalized();

    //mapping to 2D coord.
    double x1 = 0.0, y1 = 0.0;
    double x2 = (p2 - p1).dot(Vx), y2 = 0.0;
    double x3 = (p3 - p1).dot(Vx), y3 = (p3 - p1).dot(Vy);
    double A = 0.5 * std::abs(x2 * y3);

    if (A < 1e-9) return Eigen::MatrixXd::Zero(18, 18);

    //Bm membrane displacement matrix here
    Eigen::MatrixXd Bm(3, 6);
    Bm << y2 - y3, 0.0,     y3 - y1, 0.0,     y1 - y2, 0.0,
        0.0,     x3 - x2, 0.0,     x1 - x3, 0.0,     x2 - x1,
        x3 - x2, y2 - y3, x1 - x3, y3 - y1, x2 - x1, y1 - y2;
    Bm /= (2.0 * A);

    //D is the material matrix
    Eigen::Matrix3d D;
    double E = m_mat.youngsModulus;
    double nu = m_mat.poissonRatio;
    D << 1.0, nu, 0.0, nu, 1.0, 0.0, 0.0, 0.0, (1.0 - nu)/2.0;
    D *= E / (1.0 - nu * nu);

    //Km -- membrane stiffness
    Eigen::MatrixXd Km = Bm.transpose() * D * Bm * A * m_mat.thickness;

    //linear shape function inverse Jacobian
    double x21 = x2 - x1; double y21 = y2 - y1;
    double x31 = x3 - x1; double y31 = y3 - y1;
    double detJ = x21 * y31 - x31 * y21; //2 * A

    //shape function derivatives dN/dx (row 0), dN/dy (row 1)
    Eigen::MatrixXd dN(2, 3);
    dN(0, 0) = (y2 - y3) / detJ;  dN(0, 1) = (y3 - y1) / detJ;  dN(0, 2) = (y1 - y2) / detJ;
    dN(1, 0) = (x3 - x2) / detJ;  dN(1, 1) = (x1 - x3) / detJ;  dN(1, 2) = (x2 - x1) / detJ;

    //bending strain displacement matrix Bb
    //map to 9-DOF structure as [w1, thx1, thy1, w2, thx2, thy2, w3, thx3, thy3]
    Eigen::MatrixXd Bb = Eigen::MatrixXd::Zero(3, 9);
    for (int i = 0; i < 3; ++i) {
        Bb(0, i*3 + 1) = dN(0, i); //d(thx)/dx
        Bb(1, i*3 + 2) = dN(1, i); //d(thy)/dy
        Bb(2, i*3 + 1) = dN(1, i); //d(thx)/dy
        Bb(2, i*3 + 2) = dN(0, i); //d(thy)/dx
    }

    double thick3 = std::pow(m_mat.thickness, 3);
    Eigen::Matrix3d Db = D * (thick3 / 12.0);
    Eigen::MatrixXd K_bend = Bb.transpose() * Db * Bb * A;




       //MITC3 covariant shear strain formulation
       Eigen::MatrixXd B_tilde = Eigen::MatrixXd::Zero(2, 9);

    //tangential strain r-axis (Edge 1 -> 2)
    B_tilde(0, 0) = -1.0;
    B_tilde(0, 3) =  1.0;
    B_tilde(0, 1) = -0.5 * x21;
    B_tilde(0, 4) = -0.5 * x21;
    B_tilde(0, 2) = -0.5 * y21;
    B_tilde(0, 5) = -0.5 * y21;

    //tangential strain s-axis (Edge 1 -> 3)
    B_tilde(1, 0) = -1.0;
    B_tilde(1, 6) =  1.0;
    B_tilde(1, 1) = -0.5 * x31;
    B_tilde(1, 7) = -0.5 * x31;
    B_tilde(1, 2) = -0.5 * y31;
    B_tilde(1, 8) = -0.5 * y31;

    //mixed coupling vector from Edge 2 -> 3
    double x32 = x3 - x2;
    double y32 = y3 - y2;
    Eigen::VectorXd g3 = Eigen::VectorXd::Zero(9);
    g3(3) = -1.0;
    g3(6) =  1.0;
    g3(4) = -0.5 * x32;
    g3(7) = -0.5 * x32;
    g3(5) = -0.5 * y32;
    g3(8) = -0.5 * y32;

    //Eigen::VectorXd interpolator = (1.0 / 3.0) * (g3 - B_tilde.row(0).transpose() - B_tilde.row(1).transpose());
    //B_tilde.row(0) += interpolator.transpose();
    //B_tilde.row(1) += interpolator.transpose();


    //**************************************************************
    //c = g3 - g2 + g1
    Eigen::VectorXd c = g3 - B_tilde.row(1).transpose() + B_tilde.row(0).transpose();

    //apply the equal and opposite interpolations at the centroid (r = 1/3, s = 1/3)
    B_tilde.row(0) -= (1.0 / 3.0) * c.transpose();
    B_tilde.row(1) += (1.0 / 3.0) * c.transpose();
    //**************************************************************

    //inverse Jacobian
    Eigen::Matrix2d InvJ;
    InvJ(0, 0) =  y31 / detJ;   InvJ(0, 1) = -y21 / detJ;
    InvJ(1, 0) = -x31 / detJ;   InvJ(1, 1) =  x21 / detJ;

    Eigen::MatrixXd Bs = InvJ * B_tilde;

    //**************************************************************
    //shear stiffness integration
    double G_shear = E / (2.0 * (1.0 + nu));
    double k_shear = 5.0 / 6.0;

    Eigen::Matrix2d Ds = Eigen::Matrix2d::Identity() * (k_shear * G_shear * m_mat.thickness);
    Eigen::MatrixXd K_shear = Bs.transpose() * Ds * Bs * A;

    //combine plate fields
    Eigen::MatrixXd Kb = K_bend + K_shear;

    //**************************************************************
    //kinematic rotation field

    //mindlin plate theory uses DOFs [w, beta_x, beta_y]
    Eigen::MatrixXd T_plate = Eigen::MatrixXd::Zero(9, 9);
    for (int i = 0; i < 3; ++i) {
        T_plate(i*3 + 0, i*3 + 0) =  1.0; // w maps to w
        T_plate(i*3 + 1, i*3 + 2) =  1.0; // beta_x (XZ plane) maps to theta_y
        T_plate(i*3 + 2, i*3 + 1) = -1.0; // beta_y (YZ plane) maps to -theta_x
    }

    //transform the 9x9 plate stiffness matrix into shell rotation space
    Kb = T_plate.transpose() * Kb * T_plate;

    //local 18x18 matrix
    Eigen::MatrixXd K_local = Eigen::MatrixXd::Zero(18, 18);
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {

            K_local(i*6,     j*6)     = Km(i*2,     j*2);
            K_local(i*6,     j*6 + 1) = Km(i*2,     j*2 + 1);
            K_local(i*6 + 1, j*6)     = Km(i*2 + 1, j*2);
            K_local(i*6 + 1, j*6 + 1) = Km(i*2 + 1, j*2 + 1);

            for (int m = 0; m < 3; ++m) {
                for (int n = 0; n < 3; ++n) {
                    K_local(i*6 + 2 + m, j*6 + 2 + n) = Kb(i*3 + m, j*3 + n);
                }
            }
        }
        //prevent structural matrix singularity
        K_local(i*6 + 5, i*6 + 5) += E * m_mat.thickness * A * 1e-4;
    }
    //-------------------------------------------------------------------------
    //local matrix transforma to global coord system
    Eigen::Matrix3d R;
    R << Vx.x(), Vx.y(), Vx.z(),
        Vy.x(), Vy.y(), Vy.z(),
        Vz.x(), Vz.y(), Vz.z();

    Eigen::MatrixXd T = Eigen::MatrixXd::Zero(18, 18);
    for (int i = 0; i < 6; ++i) {
        T.block<3,3>(i*3, i*3) = R;
    }

    return T.transpose() * K_local * T;
}


//revised with newton raphson nonlinear solver
bool FEASolver::solve(bool useGravity, bool useNonLinear) {
    auto time_start_total = std::chrono::high_resolution_clock::now();

    int numNodes = m_mesh.vertices.size();
    if (numNodes == 0) return false;

    int dpn = 6;
    int dof = numNodes * dpn;

    m_displacements = Eigen::VectorXd::Zero(dof);
    m_history.clear();

    std::ostringstream logStream;

    //original coordiantes of vertices
    std::vector<double> origX(numNodes), origY(numNodes), origZ(numNodes);
    for (int i = 0; i < numNodes; ++i) {

        origX[i] = m_mesh.vertices[i].x;
        origY[i] = m_mesh.vertices[i].y;
        origZ[i] = m_mesh.vertices[i].z;

    }


    logStream << "Mode: 6-DOF Shell (Newton-Raphson)\n"
              << "Non-Linear Geometry: " << (useNonLinear ? "ON" : "OFF") << "\n"
              << "Gravity (-Y): " << (useGravity ? "ON" : "OFF") << "\n\n";

    std::cout << "\n--- Starting FEA ---\n" << logStream.str();

    std::cout << "\n--- Starting FEA ---\n"
              << "Mode: 6-DOF Shell (Newton-Raphson)\n"
              << "Non-Linear Geometry: " << (useNonLinear ? "ON" : "OFF") << "\n"
              << "Gravity (-Y): " << (useGravity ? "ON" : "OFF") << "\n";

    //build the Base External Force Vector (Total Loads)
    Eigen::VectorXd F_base = Eigen::VectorXd::Zero(dof);
    if (useGravity) {
        for (size_t fIdx = 0; fIdx < m_mesh.faces.size(); ++fIdx) {
            std::vector<int> vList = SubdivisionAlgorithms::getFaceVertices(m_mesh, fIdx);
            if (vList.size() < 3) continue;
            for (size_t i = 1; i + 1 < vList.size(); ++i) {

                int i1 = vList[0], i2 = vList[i], i3 = vList[i+1];
                Eigen::Vector3d p1(origX[i1], origY[i1], origZ[i1]);
                Eigen::Vector3d p2(origX[i2], origY[i2], origZ[i2]);
                Eigen::Vector3d p3(origX[i3], origY[i3], origZ[i3]);

                double Area = 0.5 * (p2 - p1).cross(p3 - p1).norm();
                double gravityForce = (Area * m_mat.thickness * m_mat.density * -9810.0);

                F_base(i1*dpn + 1) += gravityForce / 3.0;
                F_base(i2*dpn + 1) += gravityForce / 3.0;
                F_base(i3*dpn + 1) += gravityForce / 3.0;
            }
        }
    }
    for (const auto& bc : m_bcs) {
        int idx = bc.vertexIndex * dpn;
        F_base(idx) += bc.forceX; F_base(idx+1) += bc.forceY; F_base(idx+2) += bc.forceZ;
        F_base(idx+3) += bc.momentX; F_base(idx+4) += bc.momentY; F_base(idx+5) += bc.momentZ;
    }

    int steps = useNonLinear ? 10 : 1;
    int max_iters = useNonLinear ? 15 : 1;
    double tolerance = 1e-4;

    Eigen::VectorXd F_int_converged = Eigen::VectorXd::Zero(dof);
    Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;

    double total_assembly_ms = 0.0;
    double total_solve_ms = 0.0;
    long long non_zeros = 0;

    //nonlinear stepping load
    for (int step = 0; step < steps; ++step) {

        double loadFactor = (double)(step + 1) / steps;
        Eigen::VectorXd F_ext = F_base * loadFactor;
        Eigen::VectorXd Delta_U_step = Eigen::VectorXd::Zero(dof);
        Eigen::SparseMatrix<double> K_T(dof, dof);

        //newton raphson loop, check till eq. achieved
        for (int iter = 0; iter < max_iters; ++iter) {

            std::vector<Eigen::Triplet<double>> triplets;


        auto time_start_assembly = std::chrono::high_resolution_clock::now();

//tangent stiffness matrix (K_T) using deformed geometry at current index (current)
#pragma omp parallel
            {
                std::vector<Eigen::Triplet<double>> local_triplets;
#pragma omp for
                for (int fIdx = 0; fIdx < static_cast<int>(m_mesh.faces.size()); ++fIdx) {

                    std::vector<int> vList = SubdivisionAlgorithms::getFaceVertices(m_mesh, fIdx);

                    if (vList.size() < 3) continue;

                    for (size_t i = 1; i + 1 < vList.size(); ++i) {

                        int i1 = vList[0], i2 = vList[i], i3 = vList[i + 1];
                        std::vector<int> triIndices = {i1, i2, i3};

                        Eigen::MatrixXd Ke = computeElementStiffness(m_mesh.vertices[i1], m_mesh.vertices[i2], m_mesh.vertices[i3]);

                        for (int r = 0; r < 3; ++r) {
                            for (int c = 0; c < 3; ++c) {
                                for (int m = 0; m < dpn; ++m) {
                                    for (int n = 0; n < dpn; ++n) {

                                        local_triplets.push_back(Eigen::Triplet<double>(
                                            triIndices[r]*dpn + m, triIndices[c]*dpn + n, Ke(r*dpn+m, c*dpn+n)));

                                    }
                                }
                            }
                        }
                    }
                }
#pragma omp critical
                {
                    triplets.insert(triplets.end(), local_triplets.begin(), local_triplets.end());
                }
            }

            K_T.setFromTriplets(triplets.begin(), triplets.end());

            non_zeros = K_T.nonZeros(); // capture NNZ for RAM footprint

            //stop assembly timer
            auto time_end_assembly = std::chrono::high_resolution_clock::now();
            total_assembly_ms += std::chrono::duration<double, std::milli>(time_end_assembly - time_start_assembly).count();

            //current internal Force and the residual out of balance vector (R = F_ext - F_int)
            Eigen::VectorXd F_int_current = F_int_converged + (K_T * Delta_U_step);
            Eigen::VectorXd R = F_ext - F_int_current;

            //apply boundary conditions
            double penalty = m_mat.youngsModulus * 1e9;
            for (const auto& bc : m_bcs) {

                int idx = bc.vertexIndex * dpn;
                if (bc.fixX) { K_T.coeffRef(idx, idx) += penalty; R(idx) = 0.0; }
                if (bc.fixY) { K_T.coeffRef(idx+1, idx+1) += penalty; R(idx+1) = 0.0; }
                if (bc.fixZ) { K_T.coeffRef(idx+2, idx+2) += penalty; R(idx+2) = 0.0; }
                if (bc.fixRx) { K_T.coeffRef(idx+3, idx+3) += penalty; R(idx+3) = 0.0; }
                if (bc.fixRy) { K_T.coeffRef(idx+4, idx+4) += penalty; R(idx+4) = 0.0; }
                if (bc.fixRz) { K_T.coeffRef(idx+5, idx+5) += penalty; R(idx+5) = 0.0; }

            }

            auto time_start_solve = std::chrono::high_resolution_clock::now();
            //solve for iterative displacement increment (dU)
            solver.compute(K_T);
            if (solver.info() != Eigen::Success) {
                std::cerr << "Solver factorize failed. Check constraints.\n";
                return false;
            }

            Eigen::VectorXd dU = solver.solve(R);
            if (solver.info() != Eigen::Success) return false;


            auto time_end_solve = std::chrono::high_resolution_clock::now();
            total_solve_ms += std::chrono::duration<double, std::milli>(time_end_solve - time_start_solve).count();

            Delta_U_step += dU;
            m_displacements += dU;

            //update physical mesh geometry immediately so the next iteration re-calcs stiffness
            for (int i = 0; i < numNodes; ++i) {
                m_mesh.vertices[i].x = origX[i] + m_displacements(i * dpn);
                m_mesh.vertices[i].y = origY[i] + m_displacements(i * dpn + 1);
                m_mesh.vertices[i].z = origZ[i] + m_displacements(i * dpn + 2);
            }

            //convergence check: if the displacement change is tiny, we found equilibrium
            if (dU.norm() < tolerance * m_displacements.norm() || dU.norm() < 1e-7) {
                break;
            }
        }

        //lock in the converged internal forces for the next load step
        F_int_converged += K_T * Delta_U_step;

        double maxDisp = 0.0;
        for (int i = 0; i < numNodes; ++i) {
            double nodeDisp = std::sqrt(std::pow(m_displacements(i*dpn), 2) + std::pow(m_displacements(i*dpn+1), 2) + std::pow(m_displacements(i*dpn+2), 2));
            if (nodeDisp > maxDisp) maxDisp = nodeDisp;
        }

        logStream << "Step " << step + 1 << "/" << steps
                  << " Completed. Max Disp: " << maxDisp << " mm\n";

        m_history.push_back({loadFactor, maxDisp});
        std::cout << "Step " << step + 1 << "/" << steps << " Completed. Max Disp: " << maxDisp << " mm\n";

    }

    //restore the mesh
    for (int i = 0; i < numNodes; ++i) {
        m_mesh.vertices[i].x = origX[i];
        m_mesh.vertices[i].y = origY[i];
        m_mesh.vertices[i].z = origZ[i];
    }


    auto time_end_total = std::chrono::high_resolution_clock::now();
    double total_time_ms = std::chrono::duration<double, std::milli>(time_end_total - time_start_total).count();
    double ram_mb = (non_zeros * 12.0) / (1024.0 * 1024.0); // 12 bytes per sparse non-zero entry

    logStream << "\n--- Performance Metrics ---\n"
              << "Degrees of Freedom: " << dof << "\n"
              << "Matrix Non-Zeros: " << non_zeros << " (~" << std::fixed << std::setprecision(2) << ram_mb << " MB)\n"
              << "Assembly Time: " << total_assembly_ms << " ms\n"
              << "Solver Time: " << total_solve_ms << " ms\n"
              << "Total Wall Time: " << total_time_ms << " ms\n";

    QMessageBox::information(nullptr, "FEA Solver Results",
                             QString::fromStdString(logStream.str()));

    return true;
}



std::vector<double> FEASolver::getRawStresses() {
    std::vector<double> nodalStress(m_mesh.vertices.size(), 0.0);
    std::vector<int> nodalCounts(m_mesh.vertices.size(), 0);
    if (m_displacements.size() == 0) return nodalStress;

    //int dpn = 6;
    double E = m_mat.youngsModulus;
    double nu = m_mat.poissonRatio;
    double t = m_mat.thickness;

    Eigen::Matrix3d D;
    D << 1.0, nu, 0.0, nu, 1.0, 0.0, 0.0, 0.0, (1.0 - nu)/2.0;
    D *= E / (1.0 - nu * nu);

    for (size_t fIdx = 0; fIdx < m_mesh.faces.size(); ++fIdx) {
        std::vector<int> vList = SubdivisionAlgorithms::getFaceVertices(m_mesh, fIdx);
        if (vList.size() < 3) continue;

        for (size_t i = 1; i + 1 < vList.size(); ++i) {
            int i1 = vList[0], i2 = vList[i], i3 = vList[i + 1];

            Eigen::Vector3d p1(m_mesh.vertices[i1].x, m_mesh.vertices[i1].y, m_mesh.vertices[i1].z);
            Eigen::Vector3d p2(m_mesh.vertices[i2].x, m_mesh.vertices[i2].y, m_mesh.vertices[i2].z);
            Eigen::Vector3d p3(m_mesh.vertices[i3].x, m_mesh.vertices[i3].y, m_mesh.vertices[i3].z);

            Eigen::Vector3d Vx = (p2 - p1).normalized();
            Eigen::Vector3d Vz = Vx.cross(p3 - p1).normalized();
            Eigen::Vector3d Vy = Vz.cross(Vx).normalized();

            double x1 = 0.0, y1 = 0.0;
            double x2 = (p2 - p1).dot(Vx), y2 = 0.0;
            double x3 = (p3 - p1).dot(Vx), y3 = (p3 - p1).dot(Vy);
            double A = 0.5 * std::abs(x2 * y3);
            if (A < 1e-9) continue;

            Eigen::MatrixXd Bm(3, 6);
            Bm << y2 - y3, 0.0,     y3 - y1, 0.0,     y1 - y2, 0.0,
                0.0,     x3 - x2, 0.0,     x1 - x3, 0.0,     x2 - x1,
                x3 - x2, y2 - y3, x1 - x3, y3 - y1, x2 - x1, y1 - y2;
            Bm /= (2.0 * A);

            Eigen::Matrix3d R;
            R << Vx.x(), Vx.y(), Vx.z(), Vy.x(), Vy.y(), Vy.z(), Vz.x(), Vz.y(), Vz.z();

            Eigen::VectorXd U_global(18);
            for(int d=0; d<6; ++d) {
                U_global(d)    = m_displacements(i1*6 + d);
                U_global(6+d)  = m_displacements(i2*6 + d);
                U_global(12+d) = m_displacements(i3*6 + d);
            }

            Eigen::MatrixXd T = Eigen::MatrixXd::Zero(18, 18);
            for(int k=0; k<3; ++k) {
                T.block<3,3>(k*6, k*6) = R;
                T.block<3,3>(k*6 + 3, k*6 + 3) = R;
            }

            Eigen::VectorXd U_local = T * U_global;

            Eigen::VectorXd Ue_m(6);
            Ue_m << U_local(0), U_local(1), U_local(6), U_local(7), U_local(12), U_local(13);
            Eigen::Vector3d total_strain = Bm * Ue_m;

            Eigen::VectorXd Ue_b(9);
            Ue_b << U_local(2), U_local(3), U_local(4), U_local(8), U_local(9), U_local(10), U_local(14), U_local(15), U_local(16);

            Eigen::MatrixXd Bb(3, 9);
            Bb.setZero();
            Bb(0, 2) = Bm(0, 0);
            Bb(0, 5) = Bm(0, 2);
            Bb(0, 8) = Bm(0, 4);
            Bb(1, 1) = -Bm(1, 1);
            Bb(1, 4) = -Bm(1, 3);
            Bb(1, 7) = -Bm(1, 5);
            Bb(2, 1) = -Bm(0, 0);
            Bb(2, 2) = Bm(1, 1);
            Bb(2, 4) = -Bm(0, 2);
            Bb(2, 5) = Bm(1, 3);
            Bb(2, 7) = -Bm(0, 4);
            Bb(2, 8) = Bm(1, 5);


            //heatmap
            Eigen::MatrixXd T_plate = Eigen::MatrixXd::Zero(9, 9);
            for (int k = 0; k < 3; ++k) {
                T_plate(k*3 + 0, k*3 + 0) =  1.0;
                T_plate(k*3 + 1, k*3 + 2) =  1.0;
                T_plate(k*3 + 2, k*3 + 1) = -1.0;
            }
            Bb = Bb * T_plate;
            //********************************

            total_strain += (t / 2.0) * (Bb * Ue_b);

            Eigen::Vector3d stress = D * total_strain;
            double vonMises = std::sqrt(stress(0)*stress(0) - stress(0)*stress(1) + stress(1)*stress(1) + 3.0*stress(2)*stress(2));

            nodalStress[i1] += vonMises; nodalCounts[i1]++;
            nodalStress[i2] += vonMises; nodalCounts[i2]++;
            nodalStress[i3] += vonMises; nodalCounts[i3]++;
        }
    }

    for (size_t i = 0; i < nodalStress.size(); ++i) {
        if (nodalCounts[i] > 0) nodalStress[i] /= nodalCounts[i];
    }
    return nodalStress;
}

std::vector<double> FEASolver::calculateVonMisesStresses() {
    std::vector<double> stresses = getRawStresses();
    if (stresses.empty()) return stresses;

    double maxStress = *std::max_element(stresses.begin(), stresses.end());
    if (maxStress > 1e-9) {
        for (double& val : stresses) val /= maxStress;
    }
    return stresses;
}

bool FEASolver::exportValidationDataCSV(const std::string& filepath) {
    std::ofstream out(filepath);
    if (!out.is_open()) return false;

    out << "Node_ID,Orig_X,Orig_Y,Orig_Z,Disp_X,Disp_Y,Disp_Z,Disp_Mag,VonMises_MPa\n";

    int dpn = 6;
    int numNodes = m_mesh.vertices.size();
    std::vector<double> rawNodalStress = getRawStresses();

    out << std::fixed << std::setprecision(6);
    for (int i = 0; i < numNodes; ++i) {
        double vMises = rawNodalStress[i];
        double ux = m_displacements(i*dpn);
        double uy = m_displacements(i*dpn + 1);
        double uz = m_displacements(i*dpn + 2);
        double umag = std::sqrt(ux*ux + uy*uy + uz*uz);

        double origX = m_mesh.vertices[i].x;
        double origY = m_mesh.vertices[i].y;
        double origZ = m_mesh.vertices[i].z;

        out << i << "," << origX << "," << origY << "," << origZ << ","
            << ux << "," << uy << "," << uz << "," << umag << "," << vMises << "\n";
    }

    out.close();
    return true;
}
