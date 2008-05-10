/*

 * Z buffer: 16 bits Z / 32 bits color
 * 
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "zbuffer.h"

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
    xsize = xsize & ~3;

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
    xsize = xsize & ~3;

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

PIXEL lookup_texture_bilinear(ZTextureLevel *base_level, int s, int t)
{
  PIXEL p1, p2, p3, p4;
  int sf, tf;
  int r, g, b, a;

  p1 = ZB_LOOKUP_TEXTURE_NEAREST(base_level, s, t);
  p2 = ZB_LOOKUP_TEXTURE_NEAREST(base_level, s + ZB_ST_FRAC_HIGH, t);
  sf = s & ZB_ST_FRAC_MASK;

  p3 = ZB_LOOKUP_TEXTURE_NEAREST(base_level, s, t + ZB_ST_FRAC_HIGH);
  p4 = ZB_LOOKUP_TEXTURE_NEAREST(base_level, s + ZB_ST_FRAC_HIGH, t + ZB_ST_FRAC_HIGH);
  tf = t & ZB_ST_FRAC_MASK;
  
  r = (((PIXEL_R(p4) * sf + PIXEL_R(p3) * (ZB_ST_FRAC_HIGH - sf)) >> ZB_POINT_ST_FRAC_BITS) * tf +
       ((PIXEL_R(p2) * sf + PIXEL_R(p1) * (ZB_ST_FRAC_HIGH - sf)) >> ZB_POINT_ST_FRAC_BITS) * (ZB_ST_FRAC_HIGH - tf)) >> ZB_POINT_ST_FRAC_BITS;

  g = (((PIXEL_G(p4) * sf + PIXEL_G(p3) * (ZB_ST_FRAC_HIGH - sf)) >> ZB_POINT_ST_FRAC_BITS) * tf +
       ((PIXEL_G(p2) * sf + PIXEL_G(p1) * (ZB_ST_FRAC_HIGH - sf)) >> ZB_POINT_ST_FRAC_BITS) * (ZB_ST_FRAC_HIGH - tf)) >> ZB_POINT_ST_FRAC_BITS;
  
  b = (((PIXEL_B(p4) * sf + PIXEL_B(p3) * (ZB_ST_FRAC_HIGH - sf)) >> ZB_POINT_ST_FRAC_BITS) * tf +
       ((PIXEL_B(p2) * sf + PIXEL_B(p1) * (ZB_ST_FRAC_HIGH - sf)) >> ZB_POINT_ST_FRAC_BITS) * (ZB_ST_FRAC_HIGH - tf)) >> ZB_POINT_ST_FRAC_BITS;
  
  a = (((PIXEL_A(p4) * sf + PIXEL_A(p3) * (ZB_ST_FRAC_HIGH - sf)) >> ZB_POINT_ST_FRAC_BITS) * tf +
       ((PIXEL_A(p2) * sf + PIXEL_A(p1) * (ZB_ST_FRAC_HIGH - sf)) >> ZB_POINT_ST_FRAC_BITS) * (ZB_ST_FRAC_HIGH - tf)) >> ZB_POINT_ST_FRAC_BITS;

  return RGBA_TO_PIXEL(r, g, b, a);
}
