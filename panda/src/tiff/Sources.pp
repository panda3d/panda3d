#define OTHER_LIBS dtool

#begin lib_target
  #define TARGET tiff

  #define SOURCES \
    t4.h tif_aux.c tif_ccittrle.c \
    tif_close.c tif_compress.c tif_dir.c tif_dirinfo.c tif_dirread.c \
    tif_dirwrite.c tif_dumpmode.c tif_error.c tif_fax3.c tif_fax3.h \
    tif_fax4.c tif_flush.c tif_getimage.c tif_jpeg.c tif_lzw.c \
    tif_next.c tif_open.c tif_packbits.c \
    tif_print.c tif_read.c tif_strip.c tif_swab.c tif_thunder.c \
    tif_tile.c tif_unix.c tif_version.c tif_warning.c \
    tif_write.c tiff.h tiffcomp.h tiffconf.h tiffio.h tiffiop.h \
    g3states.h

// Somehow, g3states.h should be generated at compile time by running
// mkg3states.  Deal with this soon.

  #define CFLAGS -DBSDTYPES

#end lib_target

#begin test_bin_target
  #define TARGET mkg3states

  #define SOURCES mkg3states.c
#end test_bin_target

#define EXTRA_DIST \
    mkspans.c tif_apple.c tif_msdos.c tif_machdep.c tif_vms.c 
