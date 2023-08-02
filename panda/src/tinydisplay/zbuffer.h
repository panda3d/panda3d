#ifndef _tgl_zbuffer_h_
#define _tgl_zbuffer_h_

/*
 * Z buffer
 */

#include "zfeatures.h"
#include "pbitops.h"
#include "srgb_tables.h"

typedef unsigned int ZPOINT;
#define ZB_Z_BITS 20
#define ZB_POINT_Z_FRAC_BITS 10  // These must add to < 32.

/* The number of fractional bits below the S and T texture coords.
   The more we have, the more precise the texel calculation will be
   when we zoom into small details of a texture; but the greater
   chance we might overflow our 32-bit integer when texcoords get
   large.  This also limits our greatest texture size (its T dimension
   cannot exceed this number of bits).*/
#define ZB_POINT_ST_FRAC_BITS 12

/* This is the theoretical max number of bits we have available to
   shift down to achieve each next mipmap level, based on the size of
   a 32-bit int.  We need to preallocate mipmap arrays of this size. */
#define MAX_MIPMAP_LEVELS (32 - ZB_POINT_ST_FRAC_BITS + 1)

/* Returns the index within a texture level for the given (s, t) texel. */
#define ZB_TEXEL(texture_level, s, t)                                   \
  ((((t) & (texture_level).t_mask) >> (texture_level).t_shift) |      \
   (((s) & (texture_level).s_mask) >> (texture_level).s_shift))

#define ZB_LOOKUP_TEXTURE_NEAREST(texture_def, s, t) \
  (texture_def)->levels[0].pixmap[ZB_TEXEL((texture_def)->levels[0], s, t)]

#define ZB_LOOKUP_TEXTURE_MIPMAP_NEAREST(texture_def, s, t, level) \
  (texture_def)->levels[(level)].pixmap[ZB_TEXEL((texture_def)->levels[(level)], s, t)]

/* A special abs() function which doesn't require any branching
   instructions.  Might not work on some exotic hardware. */

/* Also doesn't appear to be any faster in practice.  Guess gcc is
   already doing the right thing.  Is msvc? */
//#define FAST_ABS(v) (((v) ^ ((v) >> (sizeof(v) * 8 - 1))) - ((v) >> (sizeof(v) * 8 - 1)))

#define DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) \
  { \
    (mipmap_dx) = ((unsigned int)abs(dsdx) + (unsigned int)abs(dtdx)); \
    (mipmap_level) = get_next_higher_bit((mipmap_dx) >> ZB_POINT_ST_FRAC_BITS); \
    (mipmap_dx) &= ((1 << (((mipmap_level) - 1) + ZB_POINT_ST_FRAC_BITS)) - 1); \
  }

#define ZB_POINT_RED_MIN   0x0000
#define ZB_POINT_RED_MAX   0xffff
#define ZB_POINT_GREEN_MIN 0x0000
#define ZB_POINT_GREEN_MAX 0xffff
#define ZB_POINT_BLUE_MIN  0x0000
#define ZB_POINT_BLUE_MAX  0xffff
#define ZB_POINT_ALPHA_MIN 0x0000
#define ZB_POINT_ALPHA_MAX 0xffff

/* display modes */
#define ZB_MODE_5R6G5B  1  /* true color 16 bits */
#define ZB_MODE_INDEX   2  /* color index 8 bits */
#define ZB_MODE_RGBA    3  /* 32 bit rgba mode */
#define ZB_MODE_RGB24   4  /* 24 bit rgb mode */
#define ZB_NB_COLORS    225 /* number of colors for 8 bit display */

#define RGB_TO_PIXEL(r,g,b) \
  ((((unsigned int)(r) << 8) & 0xff0000) | ((unsigned int)(g) & 0xff00) | ((unsigned int)(b) >> 8))
#define RGBA_TO_PIXEL(r,g,b,a)                                   \
  ((((unsigned int)(a) << 16) & 0xff000000) | (((unsigned int)(r) << 8) & 0xff0000) | ((unsigned int)(g) & 0xff00) | ((unsigned int)(b) >> 8))
#define RGBA8_TO_PIXEL(r,g,b,a)                                   \
  ((((unsigned int)(a) << 24) & 0xff000000) | (((unsigned int)(r) << 16) & 0xff0000) | (((unsigned int)(g) << 8) & 0xff00) | (unsigned int)(b))

#define SRGB_TO_PIXEL(r,g,b) \
  ((encode_sRGB[(unsigned int)(r) >> 4] << 16) | (encode_sRGB[(unsigned int)(g) >> 4] << 8) | (encode_sRGB[(unsigned int)(b) >> 4]))
#define SRGBA_TO_PIXEL(r,g,b,a) \
  ((((unsigned int)(a) << 16) & 0xff000000) | (encode_sRGB[(unsigned int)(r) >> 4] << 16) | (encode_sRGB[(unsigned int)(g) >> 4] << 8) | (encode_sRGB[(unsigned int)(b) >> 4]))

#define PIXEL_R(p) (((unsigned int)(p) & 0xff0000) >> 8)
#define PIXEL_G(p) ((unsigned int)(p) & 0xff00)
#define PIXEL_B(p) (((unsigned int)(p) & 0x00ff) << 8)
#define PIXEL_A(p) (((unsigned int)(p) & 0xff000000) >> 16)
#define PIXEL_SR(p) (decode_sRGB[((unsigned int)(p) & 0xff0000) >> 16])
#define PIXEL_SG(p) (decode_sRGB[((unsigned int)(p) & 0xff00) >> 8])
#define PIXEL_SB(p) (decode_sRGB[((unsigned int)(p) & 0x00ff)])
typedef unsigned int PIXEL;
#define PSZB 4
#define PSZSH 5

// Returns an unsigned product of c1 * c2
#define PCOMPONENT_MULT(c1, c2) \
  ((((unsigned int)(c1) * (unsigned int)(c2))) >> 16)
#define PCOMPONENT_MULT3(c1, c2, c3) \
  PCOMPONENT_MULT(c1, PCOMPONENT_MULT(c2, c3))
#define PCOMPONENT_MULT4(c1, c2, c3, c4) \
  PCOMPONENT_MULT(PCOMPONENT_MULT(c1, c2), PCOMPONENT_MULT(c3, c4))

// Returns a signed product of c1 * c2, where c1 is initially signed.
// We leave 2 bits on the top to differentiate between c1 < 0 and c1 >
// 0xffff; the result has the same sign.
#define PALPHA_MULT(c1, c2) \
  (((int)(((int)(c1) >> 2) * (unsigned int)(c2))) >> 14)

#define PCOMPONENT_BLEND(c1, c2, a2) \
  ((((unsigned int)(c1) * ((unsigned int)0xffff - (unsigned int)(a2)) + (unsigned int)(c2) * (unsigned int)(a2))) >> 16)

#define PALPHA_BLEND(a1, a2) \
  ((((unsigned int)(a1) * ((unsigned int)0xffff - (unsigned int)(a2))) >> 16) + (unsigned int)(a2))

#define _BLEND_RGB(r1, g1, b1, a1, r2, g2, b2, a2) \
  RGBA_TO_PIXEL(PCOMPONENT_BLEND(r1, r2, a2),   \
                PCOMPONENT_BLEND(g1, g2, a2),   \
                PCOMPONENT_BLEND(b1, b2, a2),   \
                PALPHA_BLEND(a1, a2))

#define _BLEND_SRGB(r1, g1, b1, a1, r2, g2, b2, a2) \
  SRGBA_TO_PIXEL(PCOMPONENT_BLEND(r1, r2, a2),   \
                 PCOMPONENT_BLEND(g1, g2, a2),   \
                 PCOMPONENT_BLEND(b1, b2, a2),   \
                 PALPHA_BLEND(a1, a2))

#define PIXEL_BLEND_RGB(rgb, r, g, b, a) \
  _BLEND_RGB(PIXEL_R(rgb), PIXEL_G(rgb), PIXEL_B(rgb), PIXEL_A(rgb), r, g, b, a)

#define PIXEL_BLEND_SRGB(rgb, r, g, b, a) \
  _BLEND_SRGB(PIXEL_SR(rgb), PIXEL_SG(rgb), PIXEL_SB(rgb), PIXEL_A(rgb), r, g, b, a)


typedef struct {
  PIXEL *pixmap;
  unsigned int s_mask, s_shift, t_mask, t_shift;
} ZTextureLevel;

typedef struct ZBuffer ZBuffer;
typedef struct ZBufferPoint ZBufferPoint;
typedef struct ZTextureDef ZTextureDef;

typedef void (*ZB_fillTriangleFunc)(ZBuffer *, ZBufferPoint *, ZBufferPoint *, ZBufferPoint *);

typedef void (*ZB_storePixelFunc)(ZBuffer *zb, PIXEL &result, int r, int g, int b, int a);

typedef PIXEL (*ZB_lookupTextureFunc)(ZTextureDef *texture_def, int s, int t, unsigned int level, unsigned int level_dx);

typedef int (*ZB_texWrapFunc)(int coord, int max_coord);

struct ZTextureDef {
  ZTextureLevel *levels;
  ZB_lookupTextureFunc tex_minfilter_func;
  ZB_lookupTextureFunc tex_magfilter_func;
  ZB_lookupTextureFunc tex_minfilter_func_impl;
  ZB_lookupTextureFunc tex_magfilter_func_impl;
  ZB_texWrapFunc tex_wrap_u_func;
  ZB_texWrapFunc tex_wrap_v_func;
  int s_max, t_max;
  PIXEL border_color;
};

struct ZBuffer {
  int xsize,ysize;
  int linesize; /* line size, in bytes */
  int mode;

  ZPOINT *zbuf;
  PIXEL *pbuf;
  int frame_buffer_allocated;

  int nb_colors;
  unsigned char *dctable;
  int *ctable;
  ZTextureDef current_textures[MAX_TEXTURE_STAGES];
  int reference_alpha;
  int blend_r, blend_g, blend_b, blend_a;
  ZB_storePixelFunc store_pix_func;
};

struct ZBufferPoint {
  int x,y,z;     /* integer coordinates in the zbuffer */
  int s,t;       /* coordinates for the mapping */
  int r,g,b,a;     /* color indexes */

  PN_stdfloat sz,tz;   /* temporary coordinates for mapping */

  int sa, ta;   /* mapping coordinates for optional second texture stage */
  PN_stdfloat sza,tza;

  int sb, tb;   /* mapping coordinates for optional third texture stage */
  PN_stdfloat szb,tzb;
};

/* zbuffer.c */

#ifdef DO_PSTATS
extern int pixel_count_white_untextured;
extern int pixel_count_flat_untextured;
extern int pixel_count_smooth_untextured;
extern int pixel_count_white_textured;
extern int pixel_count_flat_textured;
extern int pixel_count_smooth_textured;
extern int pixel_count_white_perspective;
extern int pixel_count_flat_perspective;
extern int pixel_count_smooth_perspective;
extern int pixel_count_smooth_multitex2;
extern int pixel_count_smooth_multitex3;

#define COUNT_PIXELS(pixel_count, p0, p1, p2) \
  (pixel_count) += abs((p0)->x * ((p1)->y - (p2)->y) + (p1)->x * ((p2)->y - (p0)->y) + (p2)->x * ((p0)->y - (p1)->y)) / 2

#else

#define COUNT_PIXELS(pixel_count, p0, p1, p2)

#endif  // DO_PSTATS

ZBuffer *ZB_open(int xsize,int ysize,int mode,
                 int nb_colors,
                 unsigned char *color_indexes,
                 unsigned int *color_table,
                 void *frame_buffer);


void ZB_close(ZBuffer *zb);

void ZB_resize(ZBuffer *zb,void *frame_buffer,int xsize,int ysize);
void ZB_clear(ZBuffer *zb, int clear_z, ZPOINT z, int clear_color, PIXEL color);
void ZB_clear_viewport(ZBuffer * zb, int clear_z, ZPOINT z, int clear_color, PIXEL color,
                       int xmin, int ymin, int xsize, int ysize);

PIXEL lookup_texture_nearest(ZTextureDef *texture_def, int s, int t, unsigned int level, unsigned int level_dx);
PIXEL lookup_texture_bilinear(ZTextureDef *texture_def, int s, int t, unsigned int level, unsigned int level_dx);
PIXEL lookup_texture_mipmap_nearest(ZTextureDef *texture_def, int s, int t, unsigned int level, unsigned int level_dx);
PIXEL lookup_texture_mipmap_linear(ZTextureDef *texture_def, int s, int t, unsigned int level, unsigned int level_dx);
PIXEL lookup_texture_mipmap_bilinear(ZTextureDef *texture_def, int s, int t, unsigned int level, unsigned int level_dx);
PIXEL lookup_texture_mipmap_trilinear(ZTextureDef *texture_def, int s, int t, unsigned int level, unsigned int level_dx);

PIXEL apply_wrap_general_minfilter(ZTextureDef *texture_def, int s, int t, unsigned int level, unsigned int level_dx);
PIXEL apply_wrap_general_magfilter(ZTextureDef *texture_def, int s, int t, unsigned int level, unsigned int level_dx);

PIXEL apply_wrap_border_color_minfilter(ZTextureDef *texture_def, int s, int t, unsigned int level, unsigned int level_dx);
PIXEL apply_wrap_border_color_magfilter(ZTextureDef *texture_def, int s, int t, unsigned int level, unsigned int level_dx);

PIXEL apply_wrap_clamp_minfilter(ZTextureDef *texture_def, int s, int t, unsigned int level, unsigned int level_dx);
PIXEL apply_wrap_clamp_magfilter(ZTextureDef *texture_def, int s, int t, unsigned int level, unsigned int level_dx);

int texcoord_clamp(int coord, int max_coord);
int texcoord_repeat(int coord, int max_coord);
int texcoord_mirror(int coord, int max_coord);
int texcoord_mirror_once(int coord, int max_coord);

/* linesize is in BYTES */
void ZB_copyFrameBuffer(const ZBuffer *zb,void *buf,int linesize);
void ZB_copyFrameBufferNoAlpha(const ZBuffer *zb,void *buf,int linesize);
void ZB_zoomFrameBuffer(ZBuffer *dest, int dest_xmin, int dest_ymin,
                        int dest_xsize, int dest_ysize,
                        const ZBuffer *source, int source_xmin, int source_ymin,
                        int source_xsize, int source_ysize);

/* zdither.c */

void ZB_initDither(ZBuffer *zb,int nb_colors,
                   unsigned char *color_indexes,int *color_table);
void ZB_closeDither(ZBuffer *zb);
void ZB_ditherFrameBuffer(ZBuffer *zb,unsigned char *dest,
                          int linesize);

/* zline.c */

void ZB_plot(ZBuffer *zb,ZBufferPoint *p);
void ZB_line(ZBuffer *zb,ZBufferPoint *p1,ZBufferPoint *p2);
void ZB_line_z(ZBuffer * zb, ZBufferPoint * p1, ZBufferPoint * p2);


/* memory.c */
void gl_free(void *p);
void *gl_malloc(int size);
void *gl_zalloc(int size);

#endif /* _tgl_zbuffer_h_ */
