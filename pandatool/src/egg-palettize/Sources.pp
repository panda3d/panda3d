#begin bin_target
  #define TARGET egg-palettize
  #define LOCAL_LIBS \
    eggbase progbase
  #define OTHER_LIBS \
    egg:c linmath:c putil:c express:c pnmimage:c pnmimagetypes:c \
    pandaegg:m panda:m pandaexpress:m \
    dtoolutil:c dconfig:c dtool:m pystub

  #define SOURCES \
    attribFile.cxx attribFile.h config_egg_palettize.cxx \
    eggPalettize.cxx eggPalettize.h \
    imageFile.cxx imageFile.h palette.cxx palette.h sourceEgg.cxx \
    sourceEgg.h string_utils.cxx string_utils.h texture.cxx texture.h \
    userAttribLine.cxx userAttribLine.h

  #define INSTALL_HEADERS \

#end bin_target

