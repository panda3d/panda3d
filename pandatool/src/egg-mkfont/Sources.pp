#define LOCAL_LIBS \
  eggbase progbase
#define OTHER_LIBS \
  pnmimagetypes:c pnmimage:c \
  egg:c linmath:c putil:c express:c pandaegg:m panda:m pandaexpress:m \
  dtoolutil:c dtoolbase:c dconfig:c dtoolconfig:m dtool:m pystub

#define UNIX_SYS_LIBS m

#begin bin_target
  #define TARGET egg-mkfont
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx   
  
  #define SOURCES \
    charBitmap.I charBitmap.h \
    charLayout.h eggMakeFont.h \
    fontFile.I fontFile.h pkFontFile.h  

  #define INCLUDED_SOURCES \
    charBitmap.cxx charLayout.cxx \
    charPlacement.cxx eggMakeFont.cxx \
    fontFile.cxx pkFontFile.cxx \

#end bin_target

#define INSTALL_SCRIPTS ttf2egg pfb2egg
#define INSTALL_DATA T1-WGL4.enc
