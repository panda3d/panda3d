#define DIRECTORY_IF_PS2 yes

#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase

#begin lib_target
  #define TARGET ps2display

  #define SOURCES \
    config_ps2display.cxx ps2GraphicsPipe.cxx ps2GraphicsWindow.cxx

  #define INSTALL_HEADERS \

#end lib_target

