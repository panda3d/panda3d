/*

 * Z buffer: 16 bits Z / 32 bits color
 * 
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "zbuffer.h"
#include "pnotify.h"

#ifdef DO_PSTATS
int pixel_count_white_untextured;
int pixel_count_flat_untextured;
int pixel_count_smooth_untextured;
int pixel_count_white_textured;
int pixel_count_flat_textured;
int pixel_count_smooth_textured;
int pixel_count_white_perspective;
int pixel_count_flat_perspective;
int pixel_count_smooth_perspective;
#endif  // DO_PSTATS

ZBuffer *ZB_open(int xsize, int ysize, int mode,
		 int nb_colors,
		 unsigned char *color_indexes,
		 unsigned int *color_table,
		 void *frame_buffer)
{
    ZBuffer *zb;
    int size;

    zb = (ZBuffer *)gl_malloc(sizeof(ZBuffer));
    if (zb == NULL)
	return NULL;

    /* xsize must be a multiple of 4 */
    xsize = (xsize + 3) & ~3;

    zb->xsize = xsize;
    zb->ysize = ysize;
    zb->mode = mode;
    zb->linesize = (xsize * PSZB + 3) & ~3;

    switch (mode) {
#ifdef TGL_FEATURE_8_BITS
    case ZB_MODE_INDEX:
	ZB_initDither(zb, nb_colors, color_indexes, color_table);
	break;
#endif
#ifdef TGL_FEATURE_32_BITS
    case ZB_MODE_RGBA:
#endif
#ifdef TGL_FEATURE_24_BITS
    case ZB_MODE_RGB24:
#endif
    case ZB_MODE_5R6G5B:
	zb->nb_colors = 0;
	break;
    default:
	goto error;
    }

    size = zb->xsize * zb->ysize * sizeof(ZPOINT);

    zb->zbuf = (ZPOINT *)gl_malloc(size);
    if (zb->zbuf == NULL)
	goto error;

    if (frame_buffer == NULL) {
	zb->pbuf = (PIXEL *)gl_malloc(zb->ysize * zb->linesize);
	if (zb->pbuf == NULL) {
	    gl_free(zb->zbuf);
	    goto error;
	}
	zb->frame_buffer_allocated = 1;
    } else {
	zb->frame_buffer_allocated = 0;
	zb->pbuf = (PIXEL *)frame_buffer;
    }

    zb->current_texture = NULL;

    return zb;
  error:
    gl_free(zb);
    return NULL;
}

void ZB_close(ZBuffer * zb)
{
#ifdef TGL_FEATURE_8_BITS
    if (zb->mode == ZB_MODE_INDEX)
	ZB_closeDither(zb);
#endif

    if (zb->frame_buffer_allocated)
	gl_free(zb->pbuf);

    gl_free(zb->zbuf);
    gl_free(zb);
}

void ZB_resize(ZBuffer * zb, void *frame_buffer, int xsize, int ysize)
{
    int size;

    /* xsize must be a multiple of 4 */
    xsize = (xsize + 3) & ~3;

    zb->xsize = xsize;
    zb->ysize = ysize;
    zb->linesize = (xsize * PSZB + 3) & ~3;

    size = zb->xsize * zb->ysize * sizeof(ZPOINT);
    gl_free(zb->zbuf);
    zb->zbuf = (ZPOINT *)gl_malloc(size);

    if (zb->frame_buffer_allocated)
	gl_free(zb->pbuf);

    if (frame_buffer == NULL) {
	zb->pbuf = (PIXEL *)gl_malloc(zb->ysize * zb->linesize);
	zb->frame_buffer_allocated = 1;
    } else {
	zb->pbuf = (PIXEL *)frame_buffer;
	zb->frame_buffer_allocated = 0;
    }
}

static void ZB_copyBuffer(ZBuffer * zb,
                          void *buf,
                          int linesize)
{
    unsigned char *p1;
    PIXEL *q;
    int y, n;

    q = zb->pbuf;
    p1 = (unsigned char *)buf;
    n = zb->xsize * PSZB;
    for (y = 0; y < zb->ysize; y++) {
	memcpy(p1, q, n);
	p1 += linesize;
	q = (PIXEL *) ((char *) q + zb->linesize);
    }
}

#define RGB32_TO_RGB16(v) \
  (((v >> 8) & 0xf800) | (((v) >> 5) & 0x07e0) | (((v) & 0xff) >> 3))

/* XXX: not optimized */
static void ZB_copyFrameBuffer5R6G5B(ZBuffer * zb, 
                                     void *buf, int linesize) 
{
    PIXEL *q;
    unsigned short *p, *p1;
    int y, n;

    q = zb->pbuf;
    p1 = (unsigned short *) buf;

    for (y = 0; y < zb->ysize; y++) {
	p = p1;
	n = zb->xsize >> 2;
	do {
            p[0] = RGB32_TO_RGB16(q[0]);
            p[1] = RGB32_TO_RGB16(q[1]);
            p[2] = RGB32_TO_RGB16(q[2]);
            p[3] = RGB32_TO_RGB16(q[3]);
	    q += 4;
	    p += 4;
	} while (--n > 0);
	p1 = (unsigned short *)((char *)p1 + linesize);
    }
}

/* XXX: not optimized */
static void ZB_copyFrameBufferRGB24(ZBuffer * zb, 
                                    void *buf, int linesize) 
{
    PIXEL *q;
    unsigned char *p, *p1;
    int y, n;

    fprintf(stderr, "copyFrameBufferRGB24\n");
    
    q = zb->pbuf;
    p1 = (unsigned char *) buf;

    for (y = 0; y < zb->ysize; y++) {
	p = p1;
	n = zb->xsize;
	do {
          p[0] = q[0];
          p[1] = q[1];
          p[2] = q[2];
          q += 4;
          p += 3;
	} while (--n > 0);
	p1 += linesize;
    }
}

void ZB_copyFrameBuffer(ZBuffer * zb, void *buf,
			int linesize)
{
    switch (zb->mode) {
#ifdef TGL_FEATURE_16_BITS
    case ZB_MODE_5R6G5B:
	ZB_copyFrameBuffer5R6G5B(zb, buf, linesize);
	break;
#endif
#ifdef TGL_FEATURE_24_BITS
    case ZB_MODE_RGB24:
	ZB_copyFrameBufferRGB24(zb, buf, linesize);
	break;
#endif
#ifdef TGL_FEATURE_32_BITS
    case ZB_MODE_RGBA:
	ZB_copyBuffer(zb, buf, linesize);
	break;
#endif
    default:
	assert(0);
    }
}


/*
 * adr must be aligned on an 'int'
 */
void memset_s(void *adr, int val, int count)
{
    int i, n, v;
    unsigned int *p;
    unsigned short *q;

    p = (unsigned int *)adr;
    v = val | (val << 16);

    n = count >> 3;
    for (i = 0; i < n; i++) {
	p[0] = v;
	p[1] = v;
	p[2] = v;
	p[3] = v;
	p += 4;
    }

    q = (unsigned short *) p;
    n = count & 7;
    for (i = 0; i < n; i++)
	*q++ = val;
}

void memset_l(void *adr, int val, int count)
{
    int i, n, v;
    unsigned int *p;

    p = (unsigned int *)adr;
    v = val;
    n = count >> 2;
    for (i = 0; i < n; i++) {
	p[0] = v;
	p[1] = v;
	p[2] = v;
	p[3] = v;
	p += 4;
    }

    n = count & 3;
    for (i = 0; i < n; i++)
	*p++ = val;
}

/* count must be a multiple of 4 and >= 4 */
void memset_RGB24(void *adr,int r, int v, int b,long count)
{
    long i, n;
    register long v1,v2,v3,*pt=(long *)(adr);
    unsigned char *p,R=(unsigned char)r,V=(unsigned char)v,B=(unsigned char)b;

    p=(unsigned char *)adr;
    *p++=R;
    *p++=V;
    *p++=B;
    *p++=R;
    *p++=V;
    *p++=B;
    *p++=R;
    *p++=V;
    *p++=B;
    *p++=R;
    *p++=V;
    *p++=B;
    v1=*pt++;
    v2=*pt++;
    v3=*pt++;
    n = count >> 2;
    for(i=1;i<n;i++) {
        *pt++=v1;
        *pt++=v2;
        *pt++=v3;
    }
}

void ZB_clear(ZBuffer * zb, int clear_z, ZPOINT z,
	      int clear_color, unsigned int r, unsigned int g, unsigned int b, unsigned int a)
{
  unsigned int color;
  int y;
  PIXEL *pp;
  
  if (clear_z) {
    memset(zb->zbuf, 0, zb->xsize * zb->ysize * sizeof(ZPOINT));
  }
  if (clear_color) {
    color = RGBA_TO_PIXEL(r, g, b, a);
    pp = zb->pbuf;
    for (y = 0; y < zb->ysize; y++) {
      memset_l(pp, color, zb->xsize);
      pp = (PIXEL *) ((char *) pp + zb->linesize);
    }
  }
}

void ZB_clear_viewport(ZBuffer * zb, int clear_z, ZPOINT z,
                       int clear_color, unsigned int r, unsigned int g, unsigned int b, unsigned int a,
                       int xmin, int ymin, int xsize, int ysize)
{
  unsigned int color;
  int y;
  PIXEL *pp;
  ZPOINT *zz;

  nassertv(xmin >= 0 && xmin < zb->xsize && 
           ymin >= 0 && ymin < zb->ysize &&
           xmin + xsize >= 0 && xmin + xsize <= zb->xsize &&
           ymin + ysize >= 0 && ymin + ysize <= zb->ysize);

  if (clear_z) {
    zz = zb->zbuf + xmin + ymin * zb->xsize;
    for (y = 0; y < ysize; ++y) {
      memset(zz, 0, xsize * sizeof(ZPOINT));
      zz += zb->xsize;
    }
  }
  if (clear_color) {
    color = RGBA_TO_PIXEL(r, g, b, a);
    pp = zb->pbuf + xmin + ymin * (zb->linesize / PSZB);
    for (y = 0; y < ysize; ++y) {
      memset_l(pp, color, xsize);
      pp += zb->xsize;
    }
  }
}

#define ZB_ST_FRAC_HIGH (1 << ZB_POINT_ST_FRAC_BITS)
#define ZB_ST_FRAC_MASK (ZB_ST_FRAC_HIGH - 1)

#define LINEAR_FILTER_BITSIZE(c1, c2, f, bitsize) \
  ((((c2) * (f)) >> bitsize) + (((c1) * ((1 << bitsize) - (f))) >> bitsize))

#define LINEAR_FILTER(c1, c2, f) \
  LINEAR_FILTER_BITSIZE(c1, c2, f, ZB_POINT_ST_FRAC_BITS)

#define BILINEAR_FILTER(c1, c2, c3, c4, sf, tf) \
  (LINEAR_FILTER(LINEAR_FILTER(c1, c2, sf), LINEAR_FILTER(c3, c4, sf), tf))

// Grab the nearest texel from the base level.  This is also
// implemented inline as ZB_LOOKUP_TEXTURE_NEAREST.
PIXEL lookup_texture_nearest(ZTextureLevel *texture_levels, int s, int t, unsigned int level, unsigned int level_dx)
{
  return ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t);
}

// Bilinear filter four texels in the base level.
PIXEL lookup_texture_bilinear(ZTextureLevel *texture_levels, int s, int t, unsigned int level, unsigned int level_dx)
{
  ZTextureLevel *base_level = texture_levels;
  PIXEL p1, p2, p3, p4;
  int sf, tf;
  int r, g, b, a;

  p1 = ZB_LOOKUP_TEXTURE_NEAREST(base_level, s - ZB_ST_FRAC_HIGH, t - ZB_ST_FRAC_HIGH);
  p2 = ZB_LOOKUP_TEXTURE_NEAREST(base_level, s, t - ZB_ST_FRAC_HIGH);
  sf = s & ZB_ST_FRAC_MASK;

  p3 = ZB_LOOKUP_TEXTURE_NEAREST(base_level, s - ZB_ST_FRAC_HIGH, t);
  p4 = ZB_LOOKUP_TEXTURE_NEAREST(base_level, s, t);
  tf = t & ZB_ST_FRAC_MASK;

  r = BILINEAR_FILTER(PIXEL_R(p1), PIXEL_R(p2), PIXEL_R(p3), PIXEL_R(p4), sf, tf);
  g = BILINEAR_FILTER(PIXEL_G(p1), PIXEL_G(p2), PIXEL_G(p3), PIXEL_G(p4), sf, tf);
  b = BILINEAR_FILTER(PIXEL_B(p1), PIXEL_B(p2), PIXEL_B(p3), PIXEL_B(p4), sf, tf);
  a = BILINEAR_FILTER(PIXEL_A(p1), PIXEL_A(p2), PIXEL_A(p3), PIXEL_A(p4), sf, tf);

  return RGBA_TO_PIXEL(r, g, b, a);
}

// Grab the nearest texel from the nearest mipmap level.  This is also
// implemented inline as ZB_LOOKUP_TEXTURE_MIPMAP_NEAREST.
PIXEL lookup_texture_mipmap_nearest(ZTextureLevel *texture_levels, int s, int t, unsigned int level, unsigned int level_dx)
{
  return ZB_LOOKUP_TEXTURE_MIPMAP_NEAREST(texture_levels, s, t, level);
}

// Linear filter the two texels from the two nearest mipmap levels.
PIXEL lookup_texture_mipmap_linear(ZTextureLevel *texture_levels, int s, int t, unsigned int level, unsigned int level_dx)
{
  PIXEL p1, p2;
  int r, g, b, a;

  p1 = ZB_LOOKUP_TEXTURE_NEAREST(texture_levels + level - 1, s >> (level - 1), t >> (level - 1));
  p2 = ZB_LOOKUP_TEXTURE_NEAREST(texture_levels + level, s >> level, t >> level);

  unsigned int bitsize = (level - 1) + ZB_POINT_ST_FRAC_BITS;
  r = LINEAR_FILTER_BITSIZE(PIXEL_R(p1), PIXEL_R(p2), level_dx, bitsize);
  g = LINEAR_FILTER_BITSIZE(PIXEL_G(p1), PIXEL_G(p2), level_dx, bitsize);
  b = LINEAR_FILTER_BITSIZE(PIXEL_B(p1), PIXEL_B(p2), level_dx, bitsize);
  a = LINEAR_FILTER_BITSIZE(PIXEL_A(p1), PIXEL_A(p2), level_dx, bitsize); 

  return RGBA_TO_PIXEL(r, g, b, a);
}

// Bilinear filter four texels in the nearest mipmap level.
PIXEL lookup_texture_mipmap_bilinear(ZTextureLevel *texture_levels, int s, int t, unsigned int level, unsigned int level_dx)
{
  ZTextureLevel *base_level = texture_levels + level;
  s >>= level;
  t >>= level;

  PIXEL p1, p2, p3, p4;
  int sf, tf;
  int r, g, b, a;

  p1 = ZB_LOOKUP_TEXTURE_NEAREST(base_level, s - ZB_ST_FRAC_HIGH, t - ZB_ST_FRAC_HIGH);
  p2 = ZB_LOOKUP_TEXTURE_NEAREST(base_level, s, t - ZB_ST_FRAC_HIGH);
  sf = s & ZB_ST_FRAC_MASK;

  p3 = ZB_LOOKUP_TEXTURE_NEAREST(base_level, s - ZB_ST_FRAC_HIGH, t);
  p4 = ZB_LOOKUP_TEXTURE_NEAREST(base_level, s, t);
  tf = t & ZB_ST_FRAC_MASK;

  r = BILINEAR_FILTER(PIXEL_R(p1), PIXEL_R(p2), PIXEL_R(p3), PIXEL_R(p4), sf, tf);
  g = BILINEAR_FILTER(PIXEL_G(p1), PIXEL_G(p2), PIXEL_G(p3), PIXEL_G(p4), sf, tf);
  b = BILINEAR_FILTER(PIXEL_B(p1), PIXEL_B(p2), PIXEL_B(p3), PIXEL_B(p4), sf, tf);
  a = BILINEAR_FILTER(PIXEL_A(p1), PIXEL_A(p2), PIXEL_A(p3), PIXEL_A(p4), sf, tf);

  return RGBA_TO_PIXEL(r, g, b, a);
}

// Bilinear filter four texels in each of the nearest two mipmap
// levels, then linear filter them together.
PIXEL lookup_texture_mipmap_trilinear(ZTextureLevel *texture_levels, int s, int t, unsigned int level, unsigned int level_dx)
{
  PIXEL p1a, p2a;

  {
    ZTextureLevel *base_level = texture_levels + level - 1;
    int sl = s >> (level - 1);
    int tl = t >> (level - 1);
    
    PIXEL p1, p2, p3, p4;
    int sf, tf;
    int r, g, b, a;

    p1 = ZB_LOOKUP_TEXTURE_NEAREST(base_level, sl - ZB_ST_FRAC_HIGH, tl - ZB_ST_FRAC_HIGH);
    p2 = ZB_LOOKUP_TEXTURE_NEAREST(base_level, sl, tl - ZB_ST_FRAC_HIGH);
    sf = sl & ZB_ST_FRAC_MASK;
    
    p3 = ZB_LOOKUP_TEXTURE_NEAREST(base_level, sl - ZB_ST_FRAC_HIGH, tl);
    p4 = ZB_LOOKUP_TEXTURE_NEAREST(base_level, sl, tl);
    tf = tl & ZB_ST_FRAC_MASK;
    
    r = BILINEAR_FILTER(PIXEL_R(p1), PIXEL_R(p2), PIXEL_R(p3), PIXEL_R(p4), sf, tf);
    g = BILINEAR_FILTER(PIXEL_G(p1), PIXEL_G(p2), PIXEL_G(p3), PIXEL_G(p4), sf, tf);
    b = BILINEAR_FILTER(PIXEL_B(p1), PIXEL_B(p2), PIXEL_B(p3), PIXEL_B(p4), sf, tf);
    a = BILINEAR_FILTER(PIXEL_A(p1), PIXEL_A(p2), PIXEL_A(p3), PIXEL_A(p4), sf, tf);
    p1a = RGBA_TO_PIXEL(r, g, b, a);
  }

  {
    ZTextureLevel *base_level = texture_levels + level;
    int sl = s >> level;
    int tl = t >> level;
    
    PIXEL p1, p2, p3, p4;
    int sf, tf;
    int r, g, b, a;

    p1 = ZB_LOOKUP_TEXTURE_NEAREST(base_level, sl - ZB_ST_FRAC_HIGH, tl - ZB_ST_FRAC_HIGH);
    p2 = ZB_LOOKUP_TEXTURE_NEAREST(base_level, sl, tl - ZB_ST_FRAC_HIGH);
    sf = sl & ZB_ST_FRAC_MASK;
    
    p3 = ZB_LOOKUP_TEXTURE_NEAREST(base_level, sl - ZB_ST_FRAC_HIGH, tl);
    p4 = ZB_LOOKUP_TEXTURE_NEAREST(base_level, sl, tl);
    tf = tl & ZB_ST_FRAC_MASK;
    
    r = BILINEAR_FILTER(PIXEL_R(p1), PIXEL_R(p2), PIXEL_R(p3), PIXEL_R(p4), sf, tf);
    g = BILINEAR_FILTER(PIXEL_G(p1), PIXEL_G(p2), PIXEL_G(p3), PIXEL_G(p4), sf, tf);
    b = BILINEAR_FILTER(PIXEL_B(p1), PIXEL_B(p2), PIXEL_B(p3), PIXEL_B(p4), sf, tf);
    a = BILINEAR_FILTER(PIXEL_A(p1), PIXEL_A(p2), PIXEL_A(p3), PIXEL_A(p4), sf, tf);
    p2a = RGBA_TO_PIXEL(r, g, b, a);
  }

  int r, g, b, a;
  unsigned int bitsize = (level - 1) + ZB_POINT_ST_FRAC_BITS;
  r = LINEAR_FILTER_BITSIZE(PIXEL_R(p1a), PIXEL_R(p2a), level_dx, bitsize);
  g = LINEAR_FILTER_BITSIZE(PIXEL_G(p1a), PIXEL_G(p2a), level_dx, bitsize);
  b = LINEAR_FILTER_BITSIZE(PIXEL_B(p1a), PIXEL_B(p2a), level_dx, bitsize);
  a = LINEAR_FILTER_BITSIZE(PIXEL_A(p1a), PIXEL_A(p2a), level_dx, bitsize); 

  return RGBA_TO_PIXEL(r, g, b, a);
}
