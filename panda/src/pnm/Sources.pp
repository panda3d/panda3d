#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define LOCAL_LIBS pandabase

#define CFLAGS /D__STDC__
#begin lib_target
  #define TARGET pnm

  #define SOURCES \
    bitio.c bitio.h compile.h libpbm.h libpbm1.c libpbm2.c libpbm3.c \
    libpbm4.c libpbm5.c libpgm.h libpgm1.c libpgm2.c libpnm1.c \
    libpnm2.c libpnm3.c libpnm4.c libppm.h libppm1.c libppm2.c \
    libppm3.c libppm4.c libppm5.c pbm.h pbmfont.h pbmplus.h pgm.h \
    pnm.h ppm.h ppmcmap.h ppmdraw.h rast.h version.h

  #define INSTALL_HEADERS \
    pbm.h pbmplus.h pgm.h pnm.h ppm.h

#end lib_target

