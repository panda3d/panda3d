#define OTHER_LIBS dtoolconfig dtool
#define LOCAL_LIBS pandabase linmath

#begin lib_target
  #define TARGET tiff
  
   // Note: tif_aux.c switched to tif_aux.cxx, so cmath.h can be included for our fixed floor()

   #define SHARED_SRCS \
     tif_aux.cxx tif_close.c tif_codec.c tif_compress.c tif_dir.c  \
     tif_dirinfo.c tif_dirread.c tif_dirwrite.c tif_dumpmode.c  \
     tif_error.c tif_fax3.c tif_flush.c tif_jpeg.c tif_luv.c  \
     tif_lzw.c tif_next.c tif_open.c tif_packbits.c  \
     tif_pixarlog.c tif_predict.c tif_print.c tif_read.c  \
     tif_strip.c tif_swab.c tif_thunder.c tif_tile.c  \
     tif_version.c tif_warning.c tif_write.c tif_zip.c tif_getimage.c \
     g3states.h tif_dir.h tiff.h tiffio.h uvcode.h port.h tif_fax3.h \
     tiffcomp.h tiffiop.h t4.h tif_predict.h tiffconf.h tiffvers.h 
     
   #define WIN32_SRCS fax3sm_winnt.c tif_win32.c 
   #define UNIX_SRCS tif_fax3sm.c tif_unix.c
     
    #if $[eq $[PLATFORM],Win32]
      #define SOURCES $[SHARED_SRCS] $[WIN32_SRCS]
      #define CFLAGS /DTIF_PLATFORM_CONSOLE   
    #else
      #define SOURCES $[SHARED_SRCS] $[UNIX_SRCS]   
    #endif

// Somehow, g3states.h should be generated at compile time by running
// mkg3states.  Deal with this soon.

//  #define CFLAGS -DBSDTYPES $[CFLAGS]

#end lib_target

#begin test_bin_target
  #define TARGET mkg3states

  #define SOURCES mkg3states.c
#end test_bin_target

#define EXTRA_DIST \
    mkspans.c tif_apple.c tif_msdos.c tif_vms.c 
