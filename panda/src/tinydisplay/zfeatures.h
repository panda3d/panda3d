#ifndef _tgl_features_h_
#define _tgl_features_h_

/* Define BYTE_ORDER correctly where missing */
#ifndef BYTE_ORDER
#define LITTLE_ENDIAN  1234
#define BIG_ENDIAN     4321

#if (defined(__i386__) || defined(__i386)) || \
     defined(__ia64__) || defined(WIN32) || \
    (defined(__alpha__) || defined(__alpha)) || \
     defined(__arm__) || \
    (defined(__mips__) && defined(__MIPSEL__)) || \
     defined(__SYMBIAN32__) || \
     defined(__x86_64__) || \
     defined(__LITTLE_ENDIAN__)
#define BYTE_ORDER  LITTLE_ENDIAN
#else
#define BYTE_ORDER  BIG_ENDIAN
#endif

#endif


/* It is possible to enable/disable (compile time) features in this
   header file. */

#define TGL_FEATURE_ARRAYS         1
#define TGL_FEATURE_DISPLAYLISTS   1
#define TGL_FEATURE_POLYGON_OFFSET 1

/* enable various conversion code from internal pixel format (32 bits
   per pixel) to any external format */
#define TGL_FEATURE_16_BITS        1
//#define TGL_FEATURE_8_BITS         1
#define TGL_FEATURE_24_BITS        1
#define TGL_FEATURE_32_BITS        1

/* Number of simultaneous texture stages supported (multitexture). */
#define MAX_TEXTURE_STAGES 3

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#endif /* _tgl_features_h_ */
