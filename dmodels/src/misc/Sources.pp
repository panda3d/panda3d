#define DIR_TYPE models
#define INSTALL_TO models/p3misc

#begin flt_egg
  #define SOURCES $[wildcard *.flt]
#end flt_egg


#begin install_egg
  #define UNPAL_SOURCES \
    camera.p3egg rgbCube.p3egg xyzAxis.p3egg
#end install_egg

#begin install_egg
  #define SOURCES \
    gridBack.p3egg objectHandles.p3egg sphere.p3egg smiley.p3egg lilsmiley.p3egg
#end install_egg

#begin install_egg
  #define SOURCES \
    fade_sphere.p3egg fade.p3egg iris.p3egg
#end install_egg
