
# CubeSat-Geant4

<p align="center">
  <img src="payload.jpg" width="550">
</p>

<h1 align="center">CubeSat Radiation Simulation using Geant4</h1>

<p align="center">

![Geant4](https://img.shields.io/badge/Geant4-11.x-blue)
![C++](https://img.shields.io/badge/C++-17-blue)
![CMake](https://img.shields.io/badge/CMake-Build-red)
![Platform](https://img.shields.io/badge/Platform-Linux-success)

</p>

A **Geant4-based Monte Carlo radiation simulation framework** for CubeSat missions in Low Earth Orbit (LEO). This project evaluates the interaction of space radiation with a realistic CubeSat model to analyze particle transport, energy deposition, detector response, and shielding effectiveness.

---

# Overview

This project integrates realistic CAD geometry, Geant4 physics processes, and representative space radiation environments (GCR, AP9, and AE9). It provides a platform for radiation shielding studies, detector evaluation, and CubeSat mission analysis.

## Features

- Monte Carlo particle transport
- Realistic CubeSat CAD geometry
- Custom detector implementation
- Energy deposition analysis
- Radiation shielding evaluation
- GCR / AP9 / AE9 radiation spectra
- Geant4 visualization
- STK orbit simulation
- Modular C++ architecture

---

# Simulation Workflow

```text
Space Radiation (GCR / AP9 / AE9)
            │
            ▼
 Primary Particle Generator
            │
            ▼
     Geant4 Physics List
            │
            ▼
      CubeSat Geometry
            │
            ▼
      Particle Tracking
            │
            ▼
     Sensitive Detector
            │
            ▼
    Energy Deposition
            │
            ▼
      Result Analysis
```

---

# Project Structure

```text
CubeSat-Geant4/
├── DetectorConstruction.*
├── ActionInitialization.*
├── PrimaryGeneratorAction.*
├── RunAction.*
├── EventAction.*
├── SteppingAction.*
├── exampleB1.cc
├── exampleB2a.cc
├── proton.mac
├── payload.jpg
├── gcr_proton.png
├── gcr_He.png
├── ap9flux_spec.png
├── ae9flux_spec.png
├── Geant4結果.mp4
├── STK結果.mp4
└── README.md
```

---

# CubeSat Geometry

<p align="center">
<img src="payload.jpg" width="600">
</p>

The CubeSat mechanical assembly was created in CAD software and imported into Geant4. Multiple CAD formats are included, such as STL, IGES, OBJ, SAT, and Fusion 360 files, making future modifications straightforward.

---

# Space Radiation Environment

## Galactic Cosmic Rays (GCR)

| Proton | Helium |
|:--:|:--:|
| <img src="gcr_proton.png" width="350"> | <img src="gcr_He.png" width="350"> |

## Trapped Radiation Belt

| AP9 Proton | AE9 Electron |
|:--:|:--:|
| <img src="ap9flux_spec.png" width="350"> | <img src="ae9flux_spec.png" width="350"> |

---

# Simulation Components

## DetectorConstruction
Defines the world volume, materials, CubeSat geometry, and detector placement.

## PrimaryGeneratorAction
Generates primary particles according to the selected radiation spectrum.

## EventAction
Processes event-level information and detector responses.

## RunAction
Handles initialization, statistics collection, and simulation summaries.

## SteppingAction
Tracks particle interactions and energy deposition at every simulation step.

---

# Physics Processes

The framework supports electromagnetic interactions, ionization, multiple scattering, secondary particle generation, particle tracking, and energy deposition inside spacecraft structures.

---

# Simulation Results

The repository includes:

- **Geant4結果.mp4** – particle transport visualization.
- **STK結果.mp4** – CubeSat orbit visualization.
- **Geant4結果.txt** – simulation statistics and output.

You may also create an `images/` folder to store screenshots from the videos for display on the GitHub homepage.

---

# Requirements

- Geant4 11.x
- C++17
- CMake
- OpenGL / Qt Visualization

# Build

```bash
mkdir build
cd build
cmake ..
make -j
```

# Run

```bash
./exampleB1 proton.mac
```

or

```bash
./exampleB2a proton.mac
```

---

# Applications

- CubeSat radiation analysis
- Radiation shielding optimization
- Detector development
- Space environment simulation
- Aerospace education and research

---

# Future Work

- Total Ionizing Dose (TID) analysis
- Single Event Effect (SEE) simulation
- Parallel computing
- GDML geometry support
- Automatic report generation
- GPU acceleration

---

# Technologies

| Category | Technology |
|---|---|
| Programming | C++17 |
| Simulation | Geant4 |
| Build System | CMake |
| CAD | Fusion 360 |
| Visualization | OpenGL / Qt |
| Orbit Analysis | STK |

---

# References

- Geant4 Collaboration: https://geant4.web.cern.ch/
- NASA AP9/AE9 Radiation Models
- ESA Space Environment Information System

---

# License

This project is intended for academic research and educational purposes. Please cite Geant4 in any publications that use this work.
