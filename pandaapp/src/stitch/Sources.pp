#begin bin_target
  #define TARGET stitch-command
  #define LOCAL_LIBS \
    stitchbase
  #define OTHER_LIBS \
    progbase pandatoolbase converter \
    pnmimage:c mathutil:c linmath:c putil:c panda:m \
    express:c pandabase:c pandaexpress:m \
    dtoolutil:c dconfig:c dtoolbase:c dtoolconfig:m dtool:m \
    pystub

  #define SOURCES \
    stitchCommandProgram.cxx stitchCommandProgram.h

  #define INSTALL_HEADERS \

#end bin_target

#begin bin_target
  #define TARGET stitch-image
  #define LOCAL_LIBS \
    stitchbase
  #define OTHER_LIBS \
    progbase pandatoolbase converter \
    pnmimagetypes:c pnmimage:c linmath:c putil:c panda:m \
    express:c pandabase:c pandaexpress:m \
    dtoolutil:c dconfig:c dtoolbase:c dtoolconfig:m dtool:m \
    pystub

  #define SOURCES \
    stitchImageProgram.cxx stitchImageProgram.h

#end bin_target

#begin bin_target
  // Temporarily commented out until we can bring this to new scene graph.
  #define BUILD_TARGET
  #define TARGET stitch-viewer
  #define LOCAL_LIBS \
    stitchviewer stitchbase
  #define OTHER_LIBS \
    progbase converter \
    device:c tform:c graph:c dgraph:c sgraph:c gobj:c sgattrib:c \
    event:c chancfg:c display:c sgraphutil:c light:c \
    pnmimagetypes:c pnmimage:c putil:c express:c pandabase:c \
    panda:m pandaexpress:m \
    dtoolutil:c dconfig:c dtoolbase:c dtoolconfig:m dtool:m \
    pystub

  #define SOURCES \
    stitchViewerProgram.cxx stitchViewerProgram.h

#end bin_target

