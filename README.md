[![License](https://img.shields.io/badge/License-BSD--3-brightgreen.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![Github Actions](https://github.com/lanl-ansi/ARMO/actions/workflows/cmake.yml/badge.svg)](https://github.com/lanl-ansi/ARMO/actions/workflows/cmake.yml)
[![Code Coverage](https://codecov.io/gh/lanl-ansi/ARMO/branch/master/graph/badge.svg)](https://codecov.io/gh/lanl-ansi/ARMO)

<p align="center">
  <a href="https://github.com/hhijazi/ARMO">
    <img src="lidar-icon.png" width="120" alt="ARMO">
  </a>
</p>
<H1 align="center"> ARMO </H1>
<H3 align="center"> Alignment and Registration via Mathematical Optimization </H3>

In airborne light detection and ranging (LiDAR) systems, misalignments between the LiDAR-scanner and the inertial navigation system (INS) mounted on an unmanned aerial vehicle (UAV)'s frame can lead to inaccurate 3D point clouds. Determining the orientation-offset, or boresight error, is key to many LiDAR-based applications. In this work, we introduce a mixed-integer quadratically constrained quadratic program (MIQCQP) that can globally solve this misalignment problem. We also propose a nested spatial branch and bound (nsBB) algorithm that improves computational performance. The nsBB relies on novel preprocessing steps that progressively reduce the problem size. In addition, an adaptive grid search (aGS) allowing us to obtain quick heuristic solutions is presented. Our algorithms are open-source, multi-threaded and multi-machine compatible.

<H2 align="center"> EXAMPLES </H2>

Before             |  After
:-------------------------:|:-------------------------:
![](https://raw.githubusercontent.com/coin-or/Gravity/Align/data_sets/LiDAR/Cars_before.png)  |  ![](https://raw.githubusercontent.com/coin-or/Gravity/Align/data_sets/LiDAR/Cars_after.png)

Fig1. Boresight alignment on the Car [data-set](datasets)

Before             |  After
:-------------------------:|:-------------------------:
![](https://raw.githubusercontent.com/coin-or/Gravity/Align/data_sets/LiDAR/Tent_before.png)  |  ![](https://raw.githubusercontent.com/coin-or/Gravity/Align/data_sets/LiDAR/Tent_after.png)

Fig2. Boresight alignment on the Tent [data-set](datasets)

Before             |  After
:-------------------------:|:-------------------------:
![](https://raw.githubusercontent.com/coin-or/Gravity/Align/data_sets/LiDAR/Truck_before.png)  |  ![](https://raw.githubusercontent.com/coin-or/Gravity/Align/data_sets/LiDAR/Truck_after.png)

Fig3. Boresight alignment on the Truck [data-set](datasets)

<H2> INSTALL </H2>

Currently tested on MacOS and Linux, upcoming support for Windows.

ARMO's adaptive grid search (aGS) algorithm only requires Cmake.

Download and install it from here: http://www.cmake.org (Version 3.2 or better)

Then , simply run `cd ARMO`, `cmake` and then `make`.

For running the MIQCQP and the nsBB algorithm see below.

<H3> CMAKE OPTIONS </H3>

To run nsBB set `-DGurobi=ON -DGJK=ON -DEigen3=ON` while calling `cmake`. To run the MPI version of nsBB also set `-DOpenMPI=ON` while calling `cmake`. 

To run gurobi set `-DGurobi=ON` while calling `cmake`. 

For nsBB ONLY

Please install [openGJK](https://github.com/MattiaMontanari/openGJK) 

`export GJK_ROOT_DIR="/Users/yourname/Dev/openGJK"`

Please download and extract (`tar -xf archivename`) [Eigen3](https://gitlab.com/libeigen/eigen/-/releases/3.3.9) Eigen3 is header-only and does not need installation 

`export EIGEN3_ROOT_DIR ="/Users/yourname/Dev/eigen-3.3.9"`

For nsBB and Gurobi

Please install [Gurobi 9.5](https://www.gurobi.com/) (not required for aGS) For troubleshooting see [here](https://support.gurobi.com/hc/en-us/articles/360039093112-How-do-I-resolve-undefined-reference-errors-while-linking-Gurobi-in-C-)

`export GRB_LICENSE_FILE="/Users/yourname/Dev/gurobi.lic"`

For Mac,

`export GUROBI_HOME="/Library/gurobi950/macos_universal2"`

For Linux,

`export GUROBI_HOME="$HOME/Dev/gurobi950/linux64"`

`export PATH=$PATH:"$HOME/Dev/gurobi950/linux64/bin"`   

`export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$HOME/Dev/gurobi950/linux64/lib"`


Data sets used in the paper may be found [here](data_sets).

<H3>Arguments</H3>

The first argument is the .laz file name.

The second argument is the algorithm: `aGS`(default) for aGS algorithm, `nsBB`, for nsBB, `gurobi` to solve the boresight MIQCQP with Gurobi.

The third argument is the time limit for `aGS`.


<H2> FUNDING </H2>

Funding was provided by Los Alamos National Laboratory's Directed Research (project number XX9H). Funding for LiDAR data collection was provided by the Next‐Generation Ecosystem Experiments (NGEE Arctic) project, supported by the Office of Biological and Environmental Research in the U.S. DOE Office of Science as well as the Source Physics Experiment (SPE), Defense Nuclear Nonproliferation Research and Development (DNN R\&D), National Nuclear Security Administration.


NNSA approval number C20126
