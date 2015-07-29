#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin lib_target
  #define TARGET p3mathutil
  #define LOCAL_LIBS \
    p3linmath p3putil p3event p3express
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
    geometricBoundingVolume.I geometricBoundingVolume.h \
    intersectionBoundingVolume.h intersectionBoundingVolume.I \
    linmath_events.h \
    look_at.h \
    linmath_events.h \
    mersenne.h \
    omniBoundingVolume.I  \
    omniBoundingVolume.h \
    parabola.h \
    perlinNoise.h perlinNoise.I \
    perlinNoise2.h perlinNoise2.I \
    perlinNoise3.h perlinNoise3.I \
    plane.h \
    pta_LMatrix4.h pta_LMatrix3.h pta_LVecBase3.h \
    pta_LVecBase4.h pta_LVecBase2.h \
    randomizer.h randomizer.I \
    rotate_to.h \
    stackedPerlinNoise2.h stackedPerlinNoise2.I \
    stackedPerlinNoise3.h stackedPerlinNoise3.I \
    triangulator.h triangulator.I \
    triangulator3.h triangulator3.I \
    unionBoundingVolume.h unionBoundingVolume.I

  #define INCLUDED_SOURCES \
    boundingHexahedron.cxx boundingLine.cxx \
    boundingBox.cxx \
    boundingPlane.cxx \
    boundingSphere.cxx  \
    boundingVolume.cxx config_mathutil.cxx fftCompressor.cxx  \
    finiteBoundingVolume.cxx geometricBoundingVolume.cxx  \
    intersectionBoundingVolume.cxx \
    look_at.cxx \
    linmath_events.cxx \
    mersenne.cxx \
    omniBoundingVolume.cxx \
    parabola.cxx \
    perlinNoise.cxx \
    perlinNoise2.cxx \
    perlinNoise3.cxx \
    plane.cxx \
    pta_LMatrix4.cxx pta_LMatrix3.cxx pta_LVecBase3.cxx \
    pta_LVecBase4.cxx pta_LVecBase2.cxx \
    randomizer.cxx \
    rotate_to.cxx \
    stackedPerlinNoise2.cxx \
    stackedPerlinNoise3.cxx \
    triangulator.cxx \
    triangulator3.cxx \
    unionBoundingVolume.cxx

  #define INSTALL_HEADERS \
    boundingHexahedron.I boundingHexahedron.h boundingLine.I \
    boundingLine.h \
    boundingBox.I boundingBox.h \
    boundingPlane.I boundingPlane.h \
    boundingSphere.I boundingSphere.h boundingVolume.I \
    boundingVolume.h config_mathutil.h fftCompressor.h \
    pta_LMatrix4.h pta_LMatrix3.h pta_LVecBase3.h \
    pta_LVecBase4.h pta_LVecBase2.h \
    finiteBoundingVolume.h frustum.h frustum_src.I frustum_src.h \
    geometricBoundingVolume.I geometricBoundingVolume.h look_at.h \
    intersectionBoundingVolume.h intersectionBoundingVolume.I \
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
    triangulator.h triangulator.I \
    triangulator3.h triangulator3.I \
    unionBoundingVolume.h unionBoundingVolume.I



  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_mathutil
  #define LOCAL_LIBS \
    p3mathutil p3pipeline
  #define OTHER_LIBS $[OTHER_LIBS] p3pystub

  #define SOURCES \
    test_mathutil.cxx

#end test_bin_target


#begin test_bin_target
  #define TARGET test_tri
  #define LOCAL_LIBS \
    p3mathutil p3pipeline
  #define OTHER_LIBS $[OTHER_LIBS] p3pystub

  #define SOURCES \
    test_tri.cxx

#end test_bin_target

