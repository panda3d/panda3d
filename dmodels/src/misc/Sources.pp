#define DIR_TYPE models
#define INSTALL_TO models/misc

#begin flt_egg
  #define SOURCES $[wildcard *.flt]
#end flt_egg


#begin install_egg
  #define UNPAL_SOURCES \
    camera.egg
#end install_egg

#begin install_egg
  #define SOURCES \
    gridBack.egg objectHandles.egg sphere.egg smiley.egg
#end install_egg

#begin install_egg
  #define SOURCES \
    fade.egg iris.egg
#end install_egg
