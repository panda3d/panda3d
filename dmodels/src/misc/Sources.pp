#define DIR_TYPE models
#define INSTALL_TO models/misc

#begin flt_egg
  #define SOURCES $[wildcard *.flt]
#end flt_egg


#begin install_egg
  #define UNPAL_SOURCES \
    camera.egg rgbCube.egg xyzAxis.egg
#end install_egg

#begin install_egg
  #define SOURCES \
    gridBack.egg objectHandles.egg sphere.egg smiley.egg lilsmiley.egg
#end install_egg

#begin install_egg
  #define SOURCES \
    fade_sphere.egg fade.egg iris.egg
#end install_egg
