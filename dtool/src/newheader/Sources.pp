// The program built in this directory is just a utility tool intended
// for use within the VR Studio.  Don't build it otherwise.
#define BUILD_DIRECTORY $[CTPROJS]
#if $[CTPROJS]

  #define LOCAL_LIBS p3dtoolbase p3pystub

  #begin bin_target
    #define TARGET newheader
    #define SOURCES \
      newheader.cxx
  #end bin_target

#endif
