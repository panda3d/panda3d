/* sgi.h - header file for sgitopnm.c and pnmtosgi.c */

typedef struct {
    short           magic;
    char            storage;
    char            bpc;            /* pixel size: 1 = bytes, 2 = shorts */
    unsigned short  dimension;      /* 1 = single row, 2 = B/W, 3 = RGB */
    unsigned short  xsize,          /* width in pixels */
                    ysize,          /* height in pixels */
                    zsize;          /* # of channels; B/W=1, RGB=3, RGBA=4 */
    long            pixmin, pixmax; /* min/max pixel values */
    char            dummy1[4];
    char            name[80];
    long            colormap;
    char            dummy2[404];
} Header;
#define HeaderSize  512

#define SGI_MAGIC           (short)474

#define STORAGE_VERBATIM    0
#define STORAGE_RLE         1

#define CMAP_NORMAL         0
#define CMAP_DITHERED       1   /* not supported */
#define CMAP_SCREEN         2   /* not supported */
#define CMAP_COLORMAP       3   /* not supported */


