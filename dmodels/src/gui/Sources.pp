#define DIR_TYPE models
#define INSTALL_TO models/gui

#begin flt_egg
  #define SOURCES $[wildcard *.flt]
#end flt_egg

#begin install_egg
  #define SOURCES $[patsubst %.flt,%.egg,$[wildcard *.flt]]
#end install_egg

