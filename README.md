# View-dependent progressive meshes implementations

  This project includes implementations of View-dependent progressive meshes.
  http://research.microsoft.com/en-us/um/people/hoppe/proj/vdrpm


Libraries:
  vdpm:
  Core runtime refinement library. Features includes

  * Geomorph with custom enhancement to reduce popping effect.
  * Refinement criteria:
    - View frustum
    - Surface orientation
    - Screen-space geometric error
  * Regulation
  * Amortization
  * Runtime triangle strips generation
  * OpenGL VBO & IBO renderer
  * Custom number of vertex attributes (Normal/Color/TexCoord)

  Source codes are at share/vdpm. Configuration is at share/vdpm/include/vdpm/Config.h.

  osgVdpm:
  OpenSceneGraph wrapper. Features includes

  * Encapsulates SRMesh as osgDrawable
  * UserStats to show VDPM related parameters
  * Serializer to convert general model to VDPM format.
  * Multiple viewport with multiple SRMeshes are possible.

  Source codes are at share/osgvdpm.

  MixKit:
  Modified from QSlim to support VDPM format conversion.
  http://mgarland.org/software/qslim.html

  Features includes

  * Algorithm: Simplifying Surfaces with Color and Texture using Quadric Error Metrics
  * Refinement criteria calculation:
    - Cone-of-normals angle
    - Uniform error
    - Directional error

  Source codes are at share/mixkit.


Program:
  vdpmview:
  Simple UI to display .vdpm model. WIN32 GUI program.
  Source codes are at project/vdpmview.

  vdpmslim:
  Modified from QSlim to support VDPM format conversion. Console program.
  Source codes are at project/vdpmslim.

  osgvdpmstaticviewer/osgvdpmcompositeviewer:
  Modified from osgstaticviewer/osgcompositeviewer to display .vdpm or osgt/osgb format of vdpm files.
  Source codes are at project/openscenegraphvdpm.

  osgvdpmconv:
  Modified from osgconv to convert general models to osgt/osgb format of vdpm files.
  Source codes are at project/osgvdpmconv.


Platform:
  Built and tested on windows platform only.


How to build:
  1. Install Visual Studio
  2. Install CMake
  3. mkdir <Root directory of this project>/build
  4. In CMake GUI:
     - Where is the source code: --> [Root directory of this project]
     - Where to build the binaries: --> [Root directory of this project]/build/[program name]
  5. Press [Configure] & [Generate], the solution files will be generated at [Root directory of this project]/build/[program name]
