// This directory is temporarily commented out until it can be brought
// into the new scene graph.
#define BUILD_DIRECTORY

#begin ss_lib_target
  #define TARGET stitchviewer
  #define LOCAL_LIBS stitchbase
  #define OTHER_LIBS \
    progbase converter \
    device:c tform:c graph:c dgraph:c sgraph:c gobj:c pnmimage:c \
    sgattrib:c event:c chancfg:c display:c sgraphutil:c light:c putil:c \
    express:c panda:m

  #define SOURCES \
    stitchImageConverter.cxx stitchImageConverter.h \
    stitchImageVisualizer.cxx stitchImageVisualizer.h triangleMesh.cxx \
    triangleMesh.h

  #define INSTALL_HEADERS \
    stitchImageConverter.h stitchImageVisualizer.h triangleMesh.h

#end ss_lib_target

