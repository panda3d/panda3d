#begin bin_target
  #define TARGET stitch-command
  #define LOCAL_LIBS \
    stitchbase progbase

  #define SOURCES \
    stitchCommandProgram.cxx stitchCommandProgram.h

  #define INSTALL_HEADERS \

#end bin_target

#begin bin_target
  #define TARGET stitch-image
  #define LOCAL_LIBS \
    stitchbase progbase

  #define SOURCES \
    stitchImageProgram.cxx stitchImageProgram.h

#end bin_target

#begin bin_target
  #define TARGET stitch-viewer
  #define LOCAL_LIBS \
    stitchviewer stitchbase progbase
  #define OTHER_LIBS \
    device:c tform:c graph:c dgraph:c sgraph:c gobj:c sgattrib:c \
    event:c chancfg:c display:c sgraphutil:c light:c putil:c express:c \
    panda:m \
    dtoolutil:c dconfig:c dtool:m

  #define SOURCES \
    stitchViewerProgram.cxx stitchViewerProgram.h

#end bin_target

