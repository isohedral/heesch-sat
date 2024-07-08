# heesch-sat

A C++ program to compute Heesch numbers of unmarked polyforms, using a SAT Solver.  The software is built around a templated system that can enumerate and analyze polyforms in a number of common and unusual grids.  It can also check if a polyform tiles the plane isohedrally (but it will produce inconclusive results for anisohedral or aperiodic polyforms).

**The code is experimental and currently undocumented: use or study at your own risk.  I will be adding limited user documentation soon.  I continue to debug, improve, and optimize the code from time to time.  In the meantime I'm making it publicly available if others want to play with it.**

# Installation

First, you'll need to download and build [cryptominisat](https://github.com/msoos/cryptominisat). If you want to build the visualization tool (`viz`), you'll also need the [Cairo](https://www.cairographics.org/) library.  And you'll need a C++ compiler that supports at least C++17.  I've compiled the software using both `g++` and `clang++`.

There's no fancy build system.  Edit the file `src/Makefile`, particularly the lines up to `LIBS`, to settings appropriate for your system (the provided file works for MacOS with the libraries installed via [Macports](https://www.macports.org/)).  Then run `make` in the `src/` directory.  You can also build the individual executables, which are `gen`, `sat`, `viz`, `surrounds`, and `report`. The build process is pretty robust—each executable consists of a single source file, with all the other logic contained in templated header files.
