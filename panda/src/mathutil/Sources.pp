#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET mathutil
  #define LOCAL_LIBS \
    linmath putil
  #define USE_FFTW yes
  #define UNIX_SYS_LIBS m

  #define SOURCES \
    boundingHexahedron.I boundingHexahedron.cxx boundingHexahedron.h  \
    boundingLine.I boundingLine.cxx boundingLine.h boundingSphere.I \
    boundingSphere.cxx boundingSphere.h boundingVolume.I \
    boundingVolume.cxx boundingVolume.h config_mathutil.cxx \
    config_mathutil.h fftCompressor.cxx fftCompressor.h \
    finiteBoundingVolume.cxx finiteBoundingVolume.h frustum.h \
    frustum_src.I frustum_src.h geometricBoundingVolume.I \
    geometricBoundingVolume.cxx geometricBoundingVolume.h look_at.cxx \
    look_at.h look_at_src.I look_at_src.h omniBoundingVolume.I \
    omniBoundingVolume.cxx omniBoundingVolume.h plane.cxx plane.h \
    plane_src.I plane_src.h rotate_to.cxx rotate_to.h

  #define INSTALL_HEADERS \
    boundingHexahedron.I boundingHexahedron.h boundingLine.I \
    boundingLine.h boundingSphere.I boundingSphere.h boundingVolume.I \
    boundingVolume.h config_mathutil.h fftCompressor.h \
    finiteBoundingVolume.h frustum.h frustum_src.I frustum_src.h \
    geometricBoundingVolume.I geometricBoundingVolume.h look_at.h \
    look_at_src.I look_at_src.h mathHelpers.I mathHelpers.h \
    omniBoundingVolume.I omniBoundingVolume.h plane.h plane_src.I \
    plane_src.h rotate_to.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_mathutil
  #define LOCAL_LIBS \
    mathutil

  #define SOURCES \
    test_mathutil.cxx

#end test_bin_target

