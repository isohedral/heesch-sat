# heesch-sat

A C++ program to compute Heesch numbers of unmarked polyforms, using a SAT Solver.  The software is built around a templated system that can enumerate and analyze polyforms in a number of common and unusual grids.  It can also check if a polyform tiles the plane isohedrally (but it will produce inconclusive results for anisohedral or aperiodic polyforms).

**The code is experimental and currently undocumented: use or study at your own risk.  I will be adding limited user documentation soon.  I continue to debug, improve, and optimize the code from time to time.  In the meantime I'm making it publicly available if others want to play with it.**

# Installation

First, you'll need to download and build [cryptominisat](https://github.com/msoos/cryptominisat). If you want to build the visualization tool (`viz`), you'll also need the [Cairo](https://www.cairographics.org/) library.  And you'll need a C++ compiler that supports at least C++17.  I've compiled the software using both `g++` and `clang++`.

There's no fancy build system.  Edit the file `src/Makefile`, particularly the lines up to `LIBS`, to settings appropriate for your system (the provided file works for MacOS with the libraries installed via [Macports](https://www.macports.org/)).  Then run `make` in the `src/` directory.  You can also build the individual executables, which are `gen`, `sat`, `viz`, `surrounds`, and `report`. The build process is pretty robust—each executable consists of a single source file, with all the other logic contained in templated header files.

# Generating polyforms

The `gen` tool uses variants of Redelmeier's algorithm to enumerate all fixed or free polyforms of a given size belonging to a given base grid.  It accepts the following command-line parameters:

 * `-omino`: Generate polyominoes
 * `-hex`: Generate polyhexes
 * `-iamond`: Generate polyiamonds
 * `-octasquare`: Generate poly-(4.8.8)-tiles (i.e., unions of octagons and squares)
 * `-trihex`: Generate poly-(3.6.3.6)-tiles (i.e., unions of hexagons and triangles)
 * `-kite`: Generate polykites
 * `-drafter`: Generate polydrafters
 * `-abolo`: Generate polyaboloes
 * `-halfcairo`: Generate poly-halfcairos (cells from the superposition of the Cairo tiling and its reflection)
 * `-bevelhex`: Generate poly-(4.6.12)-tiles (triangles, hexagons, and dodecagons)
    <br/><br/>
 * `-size <n>`: Choose the size (i.e., number of cells) of the shapes to be generated
 * `-sizes <n1,n2,...,nk>`: For a grid type with multiple tile shapes, specify the counts of different shape classes individually. For example, `-octasquare -sizes 5,3` will generate poly-(4.8.8)-tiles containing five squares and three octagons
 * `-free`: Generate free polyforms (i.e., consider all symmetric copies of a polyform to be redundant), as opposed to fixed polyforms.  This is almost always what you want
 * `-holes`: Keep shapes with holes in the output (normally skipped)
 * `-units`: Define "unit compounds" to be used as the base of polyform generation.  Accepts a sequence of unit compounds from standard input, one per line.  For example, the `-abolo` switch will generate polyabolos aligned to a single fixed [4.8.8] grid, which probably isn't what you want. It makes more sense to generate based on a single unit compound made from an aligned 2-abolo, resulting in shapes roughly equivalent to the standard definition of polyaboloes (which allows multiple overlapping choices for a single triangle.  *This feature should be considered experimental and potentially flaky*
 * `-o <fname.txt>`: Write output to the specified text file.  If no file name is given, output is written to standard out.

As a typical example, `./gen -hex -size 6 -free -o 6hex.txt` will generate the 81 holeless free 6-hexes, writing the result to `6hex.txt`.  The file format is somewhat arcane, but is deliberately left as a plain text file that can be understood by eye with a bit of practice.
