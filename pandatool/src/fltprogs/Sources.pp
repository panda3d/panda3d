#begin bin_target
  #define TARGET fltcopy
  #define LOCAL_LIBS cvscopy flt

  #define OTHER_LIBS \
    express:c pandaexpress:m \
    dtoolutil:c dconfig:c dtool:m pystub

  #define SOURCES \
    fltCopy.cxx fltCopy.h

#end bin_target
