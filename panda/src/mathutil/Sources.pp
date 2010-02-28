#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#begin lib_target
  #define TARGET mathutil
  #define LOCAL_LIBS \
    linmath putil event express
  #define USE_PACKAGES fftw
  #define UNIX_SYS_LIBS m

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx 

  #define SOURCES  \
    boundingHexahedron.I boundingHexahedron.h boundingLine.I  \
    boundingLine.h \
    boundingBox.I boundingBox.h \
    boundingPlane.I boundingPlane.h \
    boundingSphere.I boundingSphere.h  \
    boundingVolume.I boundingVolume.h config_mathutil.h  \
    fftCompressor.h finiteBoundingVolume.h frustum.h  \
    frustum_src.I frustum_src.h geometricBoundingVolume.I  \
    geometricBoundingVolume.h \
    linmath_events.h \
    look_at.h look_at_src.I  \
    look_at_src.cxx look_at_src.h \
    linmath_events.h \
    mersenne.h \
    omniBoundingVolume.I  \
    omniBoundingVolume.h \
    parabola.h parabola_src.I parabola_src.cxx parabola_src.h \
    perlinNoise.h perlinNoise.I \
    perlinNoise2.h perlinNoise2.I \
    perlinNoise3.h perlinNoise3.I \
    plane.h plane_src.I plane_src.cxx plane_src.h \
    pta_LMatrix4f.h pta_LVecBase3f.h \
    randomizer.h randomizer.I \
    rotate_to.h rotate_to_src.cxx \
    stackedPerlinNoise2.h stackedPerlinNoise2.I \
    stackedPerlinNoise3.h stackedPerlinNoise3.I \
    triangulator.h triangulator.I

  #define INCLUDED_SOURCES \
    boundingHexahedron.cxx boundingLine.cxx \
    boundingBox.cxx \
    boundingPlane.cxx \
    boundingSphere.cxx  \
    boundingVolume.cxx config_mathutil.cxx fftCompressor.cxx  \
    finiteBoundingVolume.cxx geometricBoundingVolume.cxx  \
    look_at.cxx \
    linmath_events.cxx \
    mersenne.cxx \
    omniBoundingVolume.cxx \
    parabola.cxx \
    perlinNoise.cxx \
    perlinNoise2.cxx \
    perlinNoise3.cxx \
    plane.cxx \
    pta_LMatrix4f.cxx pta_LVecBase3f.cxx \
    randomizer.cxx \
    rotate_to.cxx \
    stackedPerlinNoise2.cxx \
    stackedPerlinNoise3.cxx \
    triangulator.cxx

  #define INSTALL_HEADERS \
    boundingHexahedron.I boundingHexahedron.h boundingLine.I \
    boundingLine.h \
    boundingBox.I boundingBox.h \
    boundingPlane.I boundingPlane.h \
    boundingSphere.I boundingSphere.h boundingVolume.I \
    boundingVolume.h config_mathutil.h fftCompressor.h \
    finiteBoundingVolume.h frustum.h frustum_src.I frustum_src.h \
    geometricBoundingVolume.I geometricBoundingVolume.h look_at.h \
    look_at_src.I look_at_src.h \
    linmath_events.h \
    mersenne.h \
    omniBoundingVolume.I omniBoundingVolume.h \
    parabola.h parabola_src.I parabola_src.cxx parabola_src.h \
    perlinNoise.h perlinNoise.I \
    perlinNoise2.h perlinNoise2.I \
    perlinNoise3.h perlinNoise3.I \
    plane.h plane_src.I plane_src.cxx plane_src.h \
    randomizer.h randomizer.I \
    rotate_to.h rotate_to_src.cxx \
    stackedPerlinNoise2.h stackedPerlinNoise2.I \
    stackedPerlinNoise3.h stackedPerlinNoise3.I \
    triangulator.h triangulator.I



  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_mathutil
  #define LOCAL_LIBS \
    mathutil pipeline
  #define OTHER_LIBS $[OTHER_LIBS] pystub

  #define SOURCES \
    test_mathutil.cxx

#end test_bin_target


#begin test_bin_target
  #define TARGET test_tri
  #define LOCAL_LIBS \
    mathutil pipeline
  #define OTHER_LIBS $[OTHER_LIBS] pystub

  #define SOURCES \
    test_tri.cxx

#end test_bin_target

