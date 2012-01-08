#define DIR_TYPE models
#define INSTALL_TO icons

#define fltfiles $[wildcard *.flt]
#begin flt_egg
  #define SOURCES $[fltfiles]
#end flt_egg

#define mayafiles $[wildcard *.mb]
#begin maya_egg
  #define SOURCES $[mayafiles]
#end maya_egg

#define eggfiles $[wildcard *.egg]
#begin egg
  #define SOURCES $[eggfiles]
#end egg

#begin install_icons
  #define SOURCES \
      folder.gif minusnode.gif openfolder.gif plusnode.gif python.gif \
      sphere2.gif tk.gif dot_black.gif dot_blue.gif dot_green.gif \
      dot_red.gif dot_white.gif
#end install_icons

#begin install_egg
  #define SOURCES \ 
    $[fltfiles:%.flt=%.egg] $[mayafiles:%.mb=%.egg] \
    $[eggfiles] 
#end install_egg
