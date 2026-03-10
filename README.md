```text
  ____                         __  __           _      ____             
 / ___| _   _ _ __   ___ _ __|  \/  | ___  ___| |__  |  _ \ _ __ ___  
 \___ \| | | | '_ \ / _ \ '__| |\/| |/ _ \/ __| '_ \ | |_) | '__/ _ \ 
  ___) | |_| | |_) |  __/ |  | |  | |  __/\__ \ | | ||  __/| | | (_) |
 |____/ \__,_| .__/ \___|_|  |_|  |_|\___||___/_| |_||_|   |_|  \___/ 
             |_|
```
SuperMeshPro is a unified C++ framework designed to bridge the gap between topological subdivision modeling and robust numerical simulation. Built for precision and optimized for interactive workflows, it handles highly complex structures with ease.

---

## Features

* Advanced Subdivision & Smoothing: Catmull-Clark, Doo-Sabin, and Loop subdivision methods, and Laplacian smoothing.
* Robust Numerical Simulation: Nonlinear 6-DOF shell Finite Element Analysis (FEA) and steady state heat transfer modules.
* Ray Tracing: Ray tracing capabilities for high-quality visualization and processing.
* High-Performance Computation: OpenMP and Eigen library for rapid implementation.

---

## Installation

You can install SuperMeshPro by either downloading the pre-packaged Debian build or compiling directly from source.

### Option A: Install via .deb (Recommended for Debian/Ubuntu)
Head over to the **Releases** section of this repository and download the latest `.deb` file. Install it using:

```bash
sudo apt install ./SuperMeshPro_1.0.0_amd64.deb
```
This will automatically place the executable in your system path and create a desktop shortcut.

## Option B: Compile from Source

```bash
# Install required dependencies
sudo apt update
sudo apt install build-essential qt5-qmake qtbase5-dev \
                 libqt5widgets5 libqt5gui5 libqt5core5a \
                 libqt5opengl5-dev
```
Clone the repository and run the following build commands:

```bash
mkdir -p build_tmp
cd build_tmp
qmake ../SuperMeshPro.pro
make -j$(nproc)
```

Once compiled, you can run the binary directly from the build directory:

```bash
./SuperMeshPro
```

## Screenshot

![screen](./screenshot.png)

## License
SuperMeshPro is completely free and open-source software licensed under the GNU General Public License v3.0 (GPLv3).
