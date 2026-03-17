---
title: 'SuperMeshPro: A C++ Framework Unifying Topological Subdivision and Multi-Physics Simulation'
tags:
  - C++
  - surface subdivision
  - finite element method
  - heat transfer
  - ray tracing
  - multi-physics
  - Computer Aided Engineering (CAE)
authors:
  - name: Hiroshi Watabe
    affiliation: 1
  - name: Peter K.N. Yu
    affiliation: 2
  - name: Gary Tse
    affiliation: 3
  - name: Mehrdad Shahmohammadi Beni
    corresponding: true
    affiliation: "1, 2, 3"
affiliations:
  - name: Division of Radiation Protection and Nuclear Safety, Research Center for Accelerator and Radioisotope Science, Tohoku University, Sendai, Miyagi, Japan
    index: 1
  - name: Department of Physics, City University of Hong Kong, Kowloon Tong, Hong Kong
    index: 2
  - name: School of Nursing and Health Studies, Hong Kong Metropolitan University, Hong Kong
    index: 3
date: 15 March 2026
bibliography: paper.bib
---

# Summary

Computer Aided Design (CAD) [@1; @2] and Computer Aided Engineering (CAE) [@3] are used heavily in modern engineering and scientific computing. `SuperMeshPro` is an open-source, unified C++ framework designed to bridge the gap between topological subdivision modeling and robust scientific and engineering computing. The present framework employs advanced geometric processing such as Catmull-Clark, Doo-Sabin and Loop with Laplacian smoothing coupled with multi-physics solvers for nonlinear 6-DOF shell Finite Element Analysis (FEA), steady state heat transfer, and ray tracing modules. `SuperMeshPro` utilizes OpenMP and the Eigen library to accelerate user computing. The present model delivers an interactive interface and simulation capabilities for complex structures.

# State of the field

In CAD environments, Boundary Representation (B-Rep) [@4] and Non-Uniform Rational B-Splines (NURBS) [@5] constitute the primary mathematical foundation of engineered geometry. These representations are promising in capturing geometrical design and guaranteeing the required geometric continuity. To exploit such geometrical features in numerical simulation, most notably the Finite Element Method (FEM) [@6; @7; @8; @9; @10; @12], the continuous geometric domain must first be discretized into a high quality computational mesh. The mesh serves as the substrate on which the governing partial differential equations are approximated, allowing field variables such as stress, temperature, and displacement to be evaluated at discrete nodes. 

The analysis of thin plates and shells [@13; @14; @15; @16] is vital for structural engineering because these components dominate lightweight, load bearing systems such as aircraft fuselages, automotive body panels, ship hulls, and pressure vessels. Accurate simulation of such structures demands surface meshes that possess a high degree of smoothness and continuity; otherwise, numerical artifacts and instabilities can severely compromise the solutions.

Subdivision surface algorithms provide a mathematically robust means of converting coarse polygonal meshes into smooth limit surfaces. Widely adopted schemes including Catmull-Clark [@17], Doo-Sabin [@18], and Loop [@19] iteratively refine mesh topology while preserving, or even enhancing, geometric continuity. Despite their advantages, existing open-source toolchains typically treat subdivision as a form of a pre-processing step that is disconnected from downstream multi-physics solvers. Consequently, intermediate conversions may lead to potential loss of geometrical fidelity and subsequently numerical instabilities. 

# Statement of need

`SuperMeshPro` addresses the gap between geometric modeling and physical simulation by integrating surface subdivision-based mesh refinement directly within a unified multi-physics simulation environment. The framework combines topological mesh manipulation with solvers for structural, thermal, and optical analyses, thereby eliminating the need for separate, disconnected meshing pipelines. 

By operating on the subdivided mesh in real-time, `SuperMeshPro` supplies immediate feedback on mechanical integrity, heat transfer, and optical performance. This unified approach enables rapid, iterative exploration of mesh sharpness, subdivision depth, and material parameters, offering extensive control for numerical experiments. The need for an open-source, transparent, academically rigorous platform that couples high-continuity subdivision surfaces with multi-physics solvers is pertinent. `SuperMeshPro` fulfills this requirement, mitigating numerical artifacts and reducing pre-processing overhead.

# Software design

`SuperMeshPro` utilizes a custom `MeshTopology` data structure that represents vertices, edges, and faces, enabling rapid traversal for subdivision, smoothing, and physical area/volume calculations. The SuperMeshPro graphical user interface (GUI) is shown in \autoref{fig:smp}. 

![SuperMeshPro GUI in action.\label{fig:smp}](smp.pdf)

In addition, the simplified schematic diagram shown in \autoref{fig:fig5} presents the working principle of the modules in SuperMeshPro. The user input is fed into the core of the model, which is then fed into the geometrical module for subdivision. The subdivided topology then becomes available for physics engine analysis. The final results are presented over the supplied geometry in an OpenGL window.

![Schematic diagram showing interaction of different modules in SuperMeshPro model.\label{fig:fig5}](fig5.pdf)

Lastly, it needs to be noted that the present model takes the standard obj 3D file format and a custom file format as shown schematically in \autoref{fig:fig6}.

![Schematic diagram showing the SuperMeshPro custom file format.\label{fig:fig6}](fig6.pdf)

## Geometric Modeling and Topology

The framework natively implements three classical subdivision schemes to accommodate diverse topological inputs:

1. **Catmull-Clark** [@17]: Suitable for quad meshes. For a vertex $v$ with $n$ edges, the updated vertex position $\mathbf{v}'$ is calculated using the face centroid average $\mathbf{Q}$ and edge midpoint average $\mathbf{R}$:
$$\mathbf{v}' = \frac{\mathbf{Q} + 2\mathbf{R} + (n-3)\mathbf{v}}{n}$$

2. **Doo-Sabin** [@18]: Suitable for generating flatter and boxier meshes. The new vertex $\mathbf{v}'$ is computed as a weighted average of the face centroid $\mathbf{c}$, the current vertex $\mathbf{v}$, and its adjacent vertices ($\mathbf{v}_{prev}$, $\mathbf{v}_{next}$):
$$\mathbf{v}' = \frac{\mathbf{v}_{prev} + \mathbf{v}_{next} + 4\mathbf{v} + 2\mathbf{c}}{8}$$

3. **Loop** [@19]: Suitable for triangular meshes. This employs an intermediate weight $\alpha_n$ based on vertex valence to smoothly distribute positional updates among neighboring vertices.

In order to analyze surface quality, discrete Gaussian curvature has been evaluated at each vertex using the angle defect formulation $K_v = 2\pi - \sum_{i} \theta_i$, where $\theta_i$ represents the interior angles of the adjacent faces. Global mesh properties, such as polyhedral volume, have also been computed by employing the divergence theorem applied over triangulated surface faces. The heatmap of the curvature result from surface subdivision using three different methods is shown in \autoref{fig:fig1} for an open cube with four sharp vertices.

![Surface subdivision tests using Catmull-Clark, Doo-Sabin and Loop methods with curvature heatmap for an open cube with four sharp vertices.\label{fig:fig1}](fig1.pdf){ width=60% }

## Implemented Multi-Physics Solvers

* **Nonlinear Finite Element Analysis** (see \autoref{fig:fig2}): The FEA solver implements a 6-DOF flat shell element formulation, combining membrane, bending, and transverse shear stiffness to capture complex spatial deformations [@20; @21]. In order to capture large deformations, the solver employs a Newton-Raphson nonlinear loop, solving the iterative displacement increment $\delta \mathbf{U}$ using the residual out of balance force vector $\mathbf{R}_{res}$:
$$\mathbf{K}_T \delta \mathbf{U} = \mathbf{R}_{res} = \mathbf{F}_{ext} - \mathbf{F}_{int}$$

![FEA steel plate bending test using SuperMeshPo FEA analysis module.\label{fig:fig2}](fig2.pdf){ width=60% }

* **Steady State Heat Transfer** (see \autoref{fig:fig3}): Thermal conduction is governed by an isotropic conductivity matrix having Dirichlet boundary conditions enforced using the penalty method. The resulting symmetric positive system is solved efficiently using `Eigen::SimplicialLDLT`.

![Heat transfer module test on a subdivided open cube using Catmull-Clark method.\label{fig:fig3}](fig3.pdf){ width=60% }

* **Ray Tracing Optics** (see \autoref{fig:fig4}): The ray tracing method was implemented in a multithreaded fashion for particle transport. Ray triangle intersections were resolved using the Moller-Trumbore algorithm [@22]. The diffuse and specular reflections were computed by perturbing the ideal normal vector $\mathbf{N}$ with a bounded random vector $\mathbf{S}_{rand}$ to create a random scattering profile [@23]:
$$\mathbf{R}_{scatter} = \frac{\mathbf{N} + \mathbf{S}_{rand}}{||\mathbf{N} + \mathbf{S}_{rand}||}$$

![Ray tracing module test on a subdivided open cube using Catmull-Clark method.\label{fig:fig4}](fig4.pdf){ width=60% }

# Research impact statement

`SuperMeshPro` directly impacts the fields of structural engineering, thermal analysis, and computational optics by providing researchers and engineers with a seamless workflow that preserves geometric fidelity throughout the simulation. By unifying topological subdivision and multi-physics solvers, the software drastically reduces pre-processing overhead and the potential for numerical artifacts often introduced by standard export/import pipelines. Furthermore, because the subdivided and processed meshes can be easily exported, `SuperMeshPro` ensures high reproducibility for complex numerical experiments. 

# Status and Future Work

`SuperMeshPro` establishes a strong mathematical foundation and architectural feasibility for unified topology and physics simulations. As the framework continues to evolve, future development will focus on the following main areas:

* **Extensive Benchmarking**: Rigorous quantitative validation will be performed to benchmark the custom solvers against established analytical solutions and industry standard commercial software.
* **GPU Acceleration**: Integration of CUDA based GPU acceleration is planned to reduce execution time for heavy simulation workloads and sparse matrix factorizations.
* **Expanded Material Models**: The current nonlinear FEA solver relies on linear elastic material models and flat shell elements. Future developments will incorporate hyperelastic material models and plasticity to broaden the applicability of the present framework.
* **Advanced Transport Mechanisms**: The heat transfer and optics modules will be extended to include complex thermal boundary conditions, coupled radiative heat transfer, and high precision Monte Carlo ray tracing algorithms.

# AI usage disclosure

AI tools were used only for grammar correction, proofreading, and formatting the markdown structure of this manuscript to meet JOSS submission guidelines. Additionally, AI code assistance was utilized during software development to clean existing code, assist with refactoring, and improve the efficiency of specific functions.

# Acknowledgements

The present work was supported by the JSPS KAKENHI Grant Numbers 23K07087 and 25K11003 and City University of Hong Kong grant numbers 9220124 and 9229151.

# References
