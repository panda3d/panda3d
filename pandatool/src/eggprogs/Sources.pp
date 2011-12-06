#define LOCAL_LIBS \
  p3converter p3eggbase p3progbase
#define OTHER_LIBS \
  p3egg:c pandaegg:m \
  p3event:c p3pipeline:c p3pstatclient:c p3downloader:c p3net:c p3nativenet:c \
  p3pnmimagetypes:c p3pnmimage:c p3mathutil:c p3linmath:c p3putil:c \
  panda:m \
  p3pandabase:c p3express:c pandaexpress:m \
  p3interrogatedb:c p3dtoolutil:c p3dtoolbase:c p3prc:c p3dconfig:c p3dtoolconfig:m p3dtool:m p3pystub
#define UNIX_SYS_LIBS m

#begin bin_target
  #define TARGET egg-rename

  #define SOURCES \
    eggRename.cxx eggRename.h

#end bin_target

#begin bin_target
  #define TARGET egg-trans

  #define SOURCES \
    eggTrans.cxx eggTrans.h

#end bin_target

#begin bin_target
  #define TARGET egg-crop

  #define SOURCES \
    eggCrop.cxx eggCrop.h

#end bin_target

#begin bin_target
  #define TARGET egg-texture-cards

  #define SOURCES \
    eggTextureCards.cxx eggTextureCards.h

#end bin_target

#begin bin_target
  #define TARGET egg-make-tube

  #define SOURCES \
    eggMakeTube.cxx eggMakeTube.h

#end bin_target

#begin bin_target
  #define LOCAL_LIBS p3eggcharbase $[LOCAL_LIBS]
  #define TARGET egg-topstrip

  #define SOURCES \
    eggTopstrip.cxx eggTopstrip.h

#end bin_target

#begin bin_target
  #define LOCAL_LIBS p3eggcharbase $[LOCAL_LIBS]
  #define TARGET egg-retarget-anim

  #define SOURCES \
    eggRetargetAnim.cxx eggRetargetAnim.h

#end bin_target

#begin bin_target
  #define TARGET egg2c

  #define SOURCES \
    eggToC.cxx eggToC.h

#end bin_target

#begin bin_target
  #define TARGET egg-list-textures

  #define SOURCES \
    eggListTextures.cxx eggListTextures.h

#end bin_target
