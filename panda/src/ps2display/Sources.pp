#define DIRECTORY_IF_PS2 yes

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET ps2display

  #define SOURCES \
    config_ps2display.cxx ps2GraphicsPipe.cxx ps2GraphicsWindow.cxx

  #define INSTALL_HEADERS \

#end lib_target

