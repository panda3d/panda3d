// Now that we have removed GeomIssuer, there's nothing left in this
// directory.
#define BUILD_DIRECTORY

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET gsgmisc
  #define LOCAL_LIBS \
    putil gobj gsgbase mathutil

#end lib_target

