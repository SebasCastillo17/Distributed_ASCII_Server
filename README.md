# Distributed ASCII Server — CV Portfolio Project

This repository contains a set of integrated coursework projects and experiments that combine networking and operating-systems concepts into practical, portable implementations. The materials were produced as part of university projects (CI-0123) and personal exercises, and are suitable for inclusion in a technical portfolio or CV to demonstrate systems-level programming and distributed service design.

Summary
 - Course: CI-0123 — Integrated Networks & Operating Systems (2025-I)
 - Project theme: Distributed service for delivering ASCII figure components

Project Overview (adapted)
 - Build a distributed service capable of supplying the building pieces for ASCII figures. Multiple redundant servers can run on different machines and respond to client HTTP requests.
 - The project progresses in stages: local client simulation (no network), basic piece-server over HTTP, intermediate routing servers that aggregate inventories, and a final distributed router/map that determines which servers collectively satisfy a requested figure.

Key Objectives
 - Implement an HTTP client that requests an object (ASCII figure) and receives the inventory of ASCII components required to build it.
 - Design a disk storage model for figure data where each block is limited to 256 bytes.
 - Implement a threaded simulator for local interaction between components (no network IO for the first stage).
 - Implement a piece-server that serves available inventory via HTTP and logs server events online.
 - Implement intermediate routers that maintain a map of piece-servers, propagate updates, and locate pieces across the distributed system.
 - Consider encryption for inter-node communication and provide clear logging and storage practices.

Technologies and Tools
 - Languages: C, C++
 - Build: Make, CMake
 - Simulation: NachOS (OS teaching tools), custom threaded simulators
 - Containerization (optional): Docker

Highlights for CV
 - Implemented HTTP-based client/server interactions for a custom resource-discovery protocol.
 - Designed a block-based on-disk storage model (256 B blocks) and inventory management.
 - Developed multi-threaded simulators and distributed routing logic.
 - Produced documentation, tests, and Docker-enabled examples for reproducible evaluation.

File System Module
 - This repository includes a small on-disk FileSystem implementation used by the servers and simulations. The FileSystem components are written in C and can be found under `Server/FileSystem/` (e.g. `FileSystem.c`, `FileSystem.h`, `Structures.c`, `Structures.h`) and in simulation sources (e.g. `Simulacion/src/FileSystem.c`). The storage model follows the project's constraint of 256-byte blocks and provides inventory and object-layout structures used by the piece-servers.

How to explore this repository
 1. Clone the repository.
 2. Inspect the major modules and examples grouped by topic.
 3. Compile subprojects using provided `Makefile` or `CMakeLists.txt` files.

Author
Sebastián Castillo
