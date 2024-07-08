# heesch-sat

A C++ program to compute Heesch numbers of unmarked polyforms, using a SAT Solver.  The software is built around a templated system that can enumerate and analyze polyforms in a number of common and unusual grids.  It can also check if a polyform tiles the plane isohedrally (but it will produce inconclusive results for anisohedral or aperiodic polyforms).

**The code is experimental: use or study at your own risk.  I will be adding limited user documentation soon.  I continue to debug, improve, and optimize the code from time to time.  In the meantime I'm making it publicly available if others want to play with it.**

# Installation

First, you'll need to download and build [cryptominisat](https://github.com/msoos/cryptominisat). If you want to build the visualization tool (`viz`), you'll also need the [Cairo](https://www.cairographics.org/) library.  And you'll need a C++ compiler that supports at least C++17.  I've compiled the software using both `g++` and `clang++`.

There's no fancy build system.  Edit the file `src/Makefile`, particularly the lines up to `LIBS`, to settings appropriate for your system (the provided file works for MacOS with the libraries installed via [Macports](https://www.macports.org/)).  Then run `make` in the `src/` directory.  You can also build the individual executables, which are `gen`, `sat`, `viz`, `surrounds`, and `report`. The build process is pretty robust—each executable consists of a single source file, with all the other logic contained in templated header files.

# Running the software 

## Generating polyforms

The `gen` tool uses variants of Redelmeier's algorithm to enumerate all fixed or free polyforms of a given size belonging to a given base grid.  It accepts the following command-line parameters:

 * `-omino`: Generate polyominoes
 * `-hex`: Generate polyhexes
 * `-iamond`: Generate polyiamonds
 * `-octasquare`: Generate poly-(4.8.8)-tiles (i.e., unions of octagons and squares)
 * `-trihex`: Generate poly-(3.6.3.6)-tiles (i.e., unions of hexagons and triangles)
 * `-kite`: Generate polykites (i.e., poly-[3.4.6.4]-tiles)
 * `-drafter`: Generate polydrafters (i.e., poly-[4.6.12]-tiles)
 * `-abolo`: Generate polyaboloes (i.e., poly-[4.8.8]-tiles)
 * `-halfcairo`: Generate poly-halfcairos (cells from subdivided tiles of the Cairo tiling (3.3.4.3.4))
 * `-bevelhex`: Generate poly-(4.6.12)-tiles (triangles, hexagons, and dodecagons)
    <br/><br/>
 * `-size <n>`: Choose the size (i.e., number of cells) of the shapes to be generated
 * `-sizes <n1,n2,...,nk>`: For a grid type with multiple tile shapes, specify the counts of different shape classes individually. For example, `-octasquare -sizes 5,3` will generate poly-(4.8.8)-tiles containing five squares and three octagons
 * `-free`: Generate free polyforms (i.e., consider all symmetric copies of a polyform to be redundant), as opposed to fixed polyforms.  This is almost always what you want
 * `-holes`: Keep shapes with holes in the output (normally skipped)
 * `-units`: Define "unit compounds" to be used as the base of polyform generation.  Accepts a sequence of unit compounds from standard input, one per line.  For example, the `-abolo` switch will generate polyabolos aligned to a single fixed [4.8.8] grid, which probably isn't what you want. It makes more sense to generate based on a single unit compound made from an aligned 2-abolo, resulting in shapes roughly equivalent to the standard definition of polyaboloes (which allows multiple overlapping choices for a single triangle.  *This feature should be considered experimental and potentially flaky*
 * `-o <fname.txt>`: Write output to the specified text file.  If no file name is given, output is written to standard out.

As a typical example, `./gen -hex -size 6 -free -o 6hex.txt` will generate the 81 holeless free 6-hexes, writing the result to `6hex.txt`.  The file format is somewhat arcane, but is deliberately left as a plain text file that can be understood by eye with a bit of practice.

## Analyzing polyforms

The `sat` tool reads in a sequence of polyforms and classifies them as non-tilers (in which case the Heesch number is reported) or isohedral tilers.  If a shape tiles the plane anisohedrally or aperiodically, the tool will classify it as "inconclusive".  The software can optionally output a "witness patch" for each shape that demonstrates its classification.  If the shape has a finite Heesch number, the patch will exhibit the largest possible number of coronas.  If it tiles isohedrally, no patch will be produced.  If it is inconclusive, the patch will contain a predetermined maximum number of coronas.

The `sat` tool accepts the following command-line arguments.  Any other argument is assumed to be the name of a text file meant to be processed as input.  If no such argument is provided, text is processed from standard input.

 * `-show`: Include witness patches in the output. By default, no patches are included
 * `-maxlevel`: The maximum number of coronas to generate before giving up and labelling a shape as inconclusive.  (Default: 7)
 * `-translations`: Attempt to build patches using only translated copies of a tile
 * `-rotations`: Attempt to build patches using translated and rotated (but not reflected) copies of a tile
 * `-isohedral`: Include a check for isohedral tiling (this test is not run automatically by default)
 * `-noisohedral`: Explicitly disable isohedral checking (currently redundant)
 * `-update`: Perform the classification only on shapes in the input stream that are either unclassified or inconclusive; everything else is copied over unchanged
 * `-hh`: Include the computation of Heesch numbers where the outermost corona is permitted to have holes.  Disabled by default
 * `-o <fname.txt>`: Write output to the specified text file.  If no file name is given, output is written to standard out

Continuing the example above, `./sat -isohedral -show 6hex.txt -o 6hex_out.txt` will process the free 6-hexes in `6hex.txt`, writing information about the classified shapes (including witness patches) into `6hex_out.txt`.

## Generating a summary

The `report` tool consumes a text file of classified polyforms and generate a summary text report in a simple, human-readable format.  It reads from standard input, or from an input file name if one is provided. It writes to standard output, or to an output file if the `-o` parameter is used as in the programs above.  Here's what the output looks like on the file `6hex_out.txt` generated above:

```
$ ./report 6hex_out.txt
Total: 81 shapes
  0 unprocessed
  0 with holes
  1 inconclusive
  4 non-tilers
    3 with Hc = 1
    1 with Hc = 2
    3 with Hh = 1
    1 with Hh = 2
  76 tile isohedrally
  0 tile anisohedrally
  0 tile aperiodically
```

Here, the inconclusive shape is the single 2-anisohedral 6-hex (the file format can store information about anisohedral and aperiodic polyforms, but the software doesn't know how to detect them, so the counts in the last two lines will always be zero).  The `Hc` and `Hh` counts correspond to Heesch numbers without and with holes in the outer corona, respectively.  Because the `-hh` switch was not enabled when `sat` was run, the counts will always be same.  In general, a given shape's `Hh` value might be equal to or one higher than its `Hc` value, meaning that `Hh` counts can be shifted somewhat towards higher Heesch numbers.

## Visualizing results

The `viz` tool produces a PDF showing information about each polyform in a text file, one per page. It accepts the following command-line parameters.  Any other argument is assumed to be the name of a text file meant to be processed as input.  If no such argument is provided, text is processed from standard input.

 * `-orientation`: In witness patches, colour tiles by orientation (instead of by corona number)
 * `-shapes`: Just draw small copies of all the polyforms in the file, 48 per page
 * `-o <fname.txt>`: Write output to the given text file.  If no file is specified, write output to `out.pdf`
 * `-nodraw`: Don't actually produce PDF output. Useful with the next switch
 * `-extract <fname.txt>`: If specified, write any drawn polyforms to the given text file. This can be useful if other filters are applied on the command-line, turning `viz` into a tool to find shapes with desired properties in a longer input stream
 * `-unknown`: Include tiles that haven't been classified in output
 * `-holes`: Include tiles with holes in output
 * `-inconclusive`: Include inconclusive tiles in output (note that witness patches of inconclusive tiles are always coloured by orientation)
 * `-nontiler`: Include non-tilers in output
 * `-isohedral`: Include isohedral tilers in output
 * `-hcs <n1,n2,...,nk>`: In conjunction with `-nontiler`, restrict non-tilers to shapes with the given hole-free Heesch numbers
 * `-hhs <n1,n2,...,nk>`: In conjunction with `-nontiler`, restrict non-tilers to shapes with the given hole Heesch numbers

If none of the filtering switches (`-unknown`, `-holes`, `-inconclusive`, `-nontiler`, `-isohedral`) is used, all shapes are included.

To close out the running example, executing `./viz 6hex_out.txt` will produce an 81-page PDF `out.pdf` containing drawings of the hole-free 6-hexes.  Those that don't tile will have (possibly trivial) patches exhibiting their Heesch numbers.  The isohedral tilers will show just a single copy of the shape.  The inconclusive (anisohedral) tile will show a number of coronas.

# The grids

At present, I am not providing complete documentation for the text file format used by the programs above.  If you want to understand the format, a good starting point would be to look at the function `TileInfo<grid>::write( std::ostream& os )` in `tileio.h`.  That being said, there is some value in describing the encoding of the cells of the different polyform grids.

Every polyform is described using a sequence of (x,y) coordinate pairs, given on a line with no other punctuation. Each coordinate pair specifies one cell occupied by the polyform. For example, the coordinates `0 0 1 0 2 0 0 1 2 1` might describe the U-pentomino. Some coordinate grids are specified relative to non-standard coordinate axes, and some use a sparse set of coordinate pairs, meaning that not all pairs of integers necessarily correspond to legal cells.  The benefit is that all computations can be carried using only integer operations, with no risk of inaccuracies.

### The polyomino grid

The polyomino has the simplest and most natural structure, which probably doesn't need any explanation.  The diagram gives coordinates for a few sample cells near the origin.

<p align="center"><img src="grids/omino.svg" style="width: 400px;"/></p>

### The polyhex grid

### The polyiamond grid

### The octasquare grid

### The trihex grid

### The kite grid

### The polydrafter grid

### The polyabolo grid

### The halfcairo grid

### The bevelhex grid


# References

