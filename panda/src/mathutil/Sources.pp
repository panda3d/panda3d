#define OTHER_LIBS interrogatedb:c dconfig:c dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET mathutil
  #define LOCAL_LIBS \
    linmath putil
  #define USE_FFTW yes
  #define UNIX_SYS_LIBS m

  #define SOURCES \
    boundingHexahedron.I boundingHexahedron.cxx boundingHexahedron.h \
    boundingLine.I boundingLine.cxx boundingLine.h boundingSphere.I \
    boundingSphere.cxx boundingSphere.h boundingVolume.I \
    boundingVolume.cxx boundingVolume.h config_mathutil.cxx \
    config_mathutil.h \
    fftCompressor.cxx fftCompressor.h \
    finiteBoundingVolume.cxx finiteBoundingVolume.h \
    geometricBoundingVolume.I geometricBoundingVolume.cxx \
    geometricBoundingVolume.h look_at.I look_at.cxx look_at.h \
    omniBoundingVolume.I omniBoundingVolume.cxx omniBoundingVolume.h \
    plane.I plane.N plane.cxx plane.h rotate_to.cxx rotate_to.h

  #define INSTALL_HEADERS \
    boundingHexahedron.I boundingHexahedron.h boundingLine.I \
    boundingLine.h boundingSphere.I boundingSphere.h boundingVolume.I \
    boundingVolume.h config_mathutil.h \
    fftCompressor.h \
    finiteBoundingVolume.h frustum.I \
    frustum.h geometricBoundingVolume.I geometricBoundingVolume.h \
    look_at.I look_at.h mathHelpers.I mathHelpers.h mathutil.h \
    omniBoundingVolume.I omniBoundingVolume.h plane.I plane.h \
    rotate_to.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_mathutil
  #define LOCAL_LIBS \
    mathutil

  #define SOURCES \
    test_mathutil.cxx

#end test_bin_target

