/* rast.h - header file for Sun raster files
**
** The format of a Sun raster file is as follows.  First, a struct
** rasterfile.  Note the 32-bit magic number at the beginning; this
** identifies the file type and lets you figure out whether you need
** to do little-endian / big-endian byte-swapping or not.  (The PBMPLUS
** implementation does not do byte-swapping; instead, it reads all
** multi-byte values a byte at a time.)
**
** After the struct is an optional colormap.  If ras_maptype is RMT_NONE,
** no map is present; if it's RMT_EQUAL_RGB then the map consists of
** three unsigned-char arrays ras_maplength long, one each for r g and b.
** I don't know what RMT_RAW means.  Black and white bitmaps are stored
** as ras_maptype == RMT_NONE and ras_depth == 1, with the bits stored
** eight to a byte MSB first.
**
** Finally comes the image data.  If ras_type is RT_OLD or RT_STANDARD,
** the data is just plain old uncompressed bytes, padded out to a multiple
** of 16 bits in each row.  If ras_type is RT_BYTE_ENCODED, a run-length
** compression scheme is used: an escape-byte of 128 indicates a run;
** the next byte is a count, and the one after that is the byte to be
** replicated.  The one exception to this is if the count is 1; then
** there is no third byte in the packet, it means to put a single 128
** in the data stream.
*/

#ifndef _RAST_H_
#define _RAST_H_

#define PIX_ERR		-1

struct rasterfile {
    long ras_magic;
#define	RAS_MAGIC	0x59a66a95
    long ras_width;
    long ras_height;
    long ras_depth;
    long ras_length;
    long ras_type;
#define RT_OLD		0	/* Raw pixrect image in 68000 byte order */
#define RT_STANDARD	1	/* Raw pixrect image in 68000 byte order */
#define RT_BYTE_ENCODED	2	/* Run-length compression of bytes */
#define RT_FORMAT_RGB	3	/* XRGB or RGB instead of XBGR or BGR */
#define RT_FORMAT_TIFF	4	/* tiff <-> standard rasterfile */
#define RT_FORMAT_IFF	5	/* iff (TAAC format) <-> standard rasterfile */
#define RT_EXPERIMENTAL 0xffff	/* Reserved for testing */
    long ras_maptype;
#define RMT_NONE	0
#define RMT_EQUAL_RGB	1
#define RMT_RAW		2
    long ras_maplength;
    };

struct pixrectops {
    int	(*pro_rop)();
    int	(*pro_stencil)();
    int	(*pro_batchrop)();
    int	(*pro_nop)();
    int	(*pro_destroy)();
    int	(*pro_get)();
    int	(*pro_put)();
    int	(*pro_vector)();
    struct pixrect* (*pro_region)();
    int	(*pro_putcolormap)();
    int	(*pro_getcolormap)();
    int	(*pro_putattributes)();
    int	(*pro_getattributes)();
    };

struct pr_size {
    int x, y;
    };
struct pr_pos {
    int x, y;
    };

struct pixrect {
    struct pixrectops* pr_ops;
    struct pr_size pr_size;
    int pr_depth;
    struct mpr_data* pr_data;	/* work-alike only handles memory pixrects */
    };

struct mpr_data {
    int md_linebytes;
    unsigned char* md_image;	/* note, byte not short -- avoid pr_flip() */
    struct pr_pos md_offset;
    short md_primary;
    short md_flags;
    };

typedef struct {
    int type;
    int length;
    unsigned char* map[3];
    } colormap_t;

/* And the routine definitions. */

struct pixrect* mem_create ARGS(( int w, int h, int depth ));
void mem_free ARGS(( struct pixrect* p ));

int pr_dump ARGS(( struct pixrect* p, FILE* out, colormap_t* colormap, int type, int copy_flag ));

int pr_load_header ARGS(( FILE* in, struct rasterfile* hP ));

int pr_load_colormap ARGS(( FILE* in, struct rasterfile* hP, colormap_t* colormap ));

struct pixrect* pr_load_image ARGS(( FILE* in, struct rasterfile* hP, colormap_t* colormap ));

#endif /*_RAST_H_*/
