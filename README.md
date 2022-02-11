# STEPToMesh
The program STEPToMesh converts solids contained in STEP files into triangle meshes.

# Description
The program STEPToMesh is a command line utility to convert solids contained in STEP files into triangle meshes. The supported triangles formats are ASCII STL and binary STL. STEPToMesh is based on OpenCASCADE (https://www.opencascade.com). The program uses cxxops (https://github.com/jarro2783/cxxopts) for parsing the command line.

# Requirements
 * CMake installation (https://cmake.org)
 * Visual Studio C++ installation (https://visualstudio.microsoft.com)
 * OpenCASCADE installation (https://old.opencascade.com/content/latest-release, download needs registration)

# Usage
Listing the contents (solids) of a STEP file:
`STEPToMesh -c -i <step file>`

Converting the overall file content (solids) into a mesh:

`STEPToMesh -i <step file> -o <output file> -l <linear deflection> -a <angular deflection>`

The parameters `<linear deflection>` and `<angular deflection>` control the resolution of the triangulation as described in https://dev.opencascade.org/doc/overview/html/occt_user_guides__mesh.html#occt_modalg_11_2.

Converting selected solids of the file into a mesh:

`STEPToMesh -i <step file> -o <output file> -l <linear deflection> -a <angular deflection> -s <solid1>,<solid2>,<...>`

In order to change the default output format binary STL to ASCII STL the command line argument `-f stl_ascii`has to be specified.

Following the help text from the command line:
```
STEPToMesh -h
STEP to triangle mesh conversion
Usage:
  STEPToMesh [OPTION...]

  -i, --in arg       Input file
  -o, --out arg      Output file
  -f, --format arg   Output file format (stl_bin or stl_ascii) (default:
                     stl_bin)
  -c, --content      List content (solids)
  -s, --select arg   Select solids by name or index (comma seperated list, index starts with 1)
  -l, --linear arg   Linear deflection
  -a, --angular arg  Angular deflection (degrees)
  -h, --help         Print usage
```

# Examples
 * See `examples` directory
 
# Remarks
This code has been tested with an OpenCASCADE 7.5.0 prebuilt binary (`opencascade-7.5.0-vc14-64.exe`) on Windows, as well as OpenCASCADE system packages on openSUSE Linux. With changes in the configuration section in the `CMakeLists.txt` file the build should also work with other OpenCASCADE versions.
