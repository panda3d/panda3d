#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

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
    boundingPlane.I boundingPlane.h \
    boundingSphere.I boundingSphere.h  \
    boundingVolume.I boundingVolume.h config_mathutil.h  \
    fftCompressor.h finiteBoundingVolume.h frustum.h  \
    frustum_src.I frustum_src.h geometricBoundingVolume.I  \
    geometricBoundingVolume.h \
    linmath_events.h \
    look_at.h look_at_src.I  \
    look_at_src.cxx look_at_src.h omniBoundingVolume.I  \
    omniBoundingVolume.h plane.h plane_src.I plane_src.cxx  \
    plane_src.h rotate_to.h rotate_to_src.cxx
     
  #define INCLUDED_SOURCES \
    boundingHexahedron.cxx boundingLine.cxx \
    boundingPlane.cxx \
    boundingSphere.cxx  \
    boundingVolume.cxx config_mathutil.cxx fftCompressor.cxx  \
    finiteBoundingVolume.cxx geometricBoundingVolume.cxx  \
    look_at.cxx \
    linmath_events.cxx \
    omniBoundingVolume.cxx plane.cxx rotate_to.cxx

  #define INSTALL_HEADERS \
    boundingHexahedron.I boundingHexahedron.h boundingLine.I \
    boundingLine.h \
    boundingPlane.I boundingPlane.h \
    boundingSphere.I boundingSphere.h boundingVolume.I \
    boundingVolume.h config_mathutil.h fftCompressor.h \
    finiteBoundingVolume.h frustum.h frustum_src.I frustum_src.h \
    geometricBoundingVolume.I geometricBoundingVolume.h look_at.h \
    look_at_src.I look_at_src.h \
    linmath_events.h \
    mathHelpers.I mathHelpers.h \
    omniBoundingVolume.I omniBoundingVolume.h plane.h plane_src.I \
    plane_src.h rotate_to.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_mathutil
  #define LOCAL_LIBS \
    mathutil
  #define OTHER_LIBS $[OTHER_LIBS] pystub

  #define SOURCES \
    test_mathutil.cxx

#end test_bin_target

