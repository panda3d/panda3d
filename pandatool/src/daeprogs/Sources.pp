#define BUILD_DIRECTORY $[HAVE_FCOLLADA]

#define OTHER_LIBS \
  p3egg:c pandaegg:m \
  p3pipeline:c p3pnmimage:c p3putil:c p3event:c p3mathutil:c p3linmath:c panda:m \
  p3pandabase:c p3express:c pandaexpress:m \
  p3interrogatedb:c p3dtoolutil:c p3dtoolbase:c p3prc:c p3dconfig:c p3dtoolconfig:m p3dtool:m p3pystub

#begin bin_target
  #define TARGET dae2egg
  #define LOCAL_LIBS p3daeegg p3eggbase p3progbase

  #define SOURCES \
    daeToEgg.cxx daeToEgg.h

#end bin_target

//#begin bin_target
//  #define TARGET egg2dae
//  #define LOCAL_LIBS p3daeegg p3eggbase p3progbase
//
//  #define SOURCES \
//    eggToDAE.cxx eggToDAE.h
//
//#end bin_target
