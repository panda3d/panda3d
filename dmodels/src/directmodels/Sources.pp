#define DIR_TYPE models
#define INSTALL_TO models/directmodels

#begin flt_egg
  #define SOURCES $[wildcard *.flt]
#end flt_egg

#begin install_egg
  #define SOURCES \
    sphere.egg smiley.egg
#end install_egg
