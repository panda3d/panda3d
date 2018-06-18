#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif

static void
FNAME(white_untextured) (ZBuffer *zb,
                         ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
#define INTERP_Z

#define EARLY_OUT()                             \
  {                                             \
  }

#define DRAW_INIT()                             \
  {                                             \
  }
 
#define PUT_PIXEL(_a)                                                   \
  {                                                                     \
    zz=z >> ZB_POINT_Z_FRAC_BITS;                                       \
    if (ZCMP(pz[_a], zz)) {                                             \
      STORE_PIX(pp[_a], 0xffffffffUL, 0xffffUL, 0xffffUL, 0xffffUL, 0xffffUL); \
      STORE_Z(pz[_a], zz);                                              \
    }                                                                   \
    z+=dzdx;                                                            \
  }

#define PIXEL_COUNT pixel_count_white_untextured

#include "ztriangle.h"
}

static void
FNAME(flat_untextured) (ZBuffer *zb,
                        ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
  UNUSED int color;
  UNUSED int or0, og0, ob0, oa0;

#define INTERP_Z

#define EARLY_OUT()                             \
  {                                             \
  }

#define DRAW_INIT()                             \
  {                                             \
    if (!ACMP(zb, p2->a)) {                     \
      return;                                   \
    }                                           \
    or0 = p2->r;                                \
    og0 = p2->g;                                \
    ob0 = p2->b;                                \
    oa0 = p2->a;                                \
    color=RGBA_TO_PIXEL(or0, og0, ob0, oa0);    \
  }
 
#define PUT_PIXEL(_a)                                   \
  {                                                     \
    zz=z >> ZB_POINT_Z_FRAC_BITS;                       \
    if (ZCMP(pz[_a], zz)) {                             \
      STORE_PIX(pp[_a], color, or0, og0, ob0, oa0);     \
      STORE_Z(pz[_a], zz);                              \
    }                                                   \
    z+=dzdx;                                            \
  }

#define PIXEL_COUNT pixel_count_flat_untextured

#include "ztriangle.h"
}

/*
 * Smooth filled triangle.
 * The code below is very tricky :)
 */

static void
FNAME(smooth_untextured) (ZBuffer *zb,
                          ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
#define INTERP_Z
#define INTERP_RGB

#define EARLY_OUT()                                     \
  {                                                     \
    unsigned int c0, c1, c2;                            \
    c0 = RGBA_TO_PIXEL(p0->r, p0->g, p0->b, p0->a);     \
    c1 = RGBA_TO_PIXEL(p1->r, p1->g, p1->b, p1->a);     \
    c2 = RGBA_TO_PIXEL(p2->r, p2->g, p2->b, p2->a);     \
    if (c0 == c1 && c0 == c2) {                         \
      /* It's really a flat-shaded triangle. */         \
      FNAME(flat_untextured)(zb, p0, p1, p2);           \
      return;                                           \
    }                                                   \
  }
  
#define DRAW_INIT()                             \
  {                                             \
  }

#define PUT_PIXEL(_a)                                                   \
  {                                                                     \
    zz=z >> ZB_POINT_Z_FRAC_BITS;                                       \
    if (ZCMP(pz[_a], zz)) {                                             \
      if (ACMP(zb, oa1)) {                                              \
        STORE_PIX(pp[_a], RGBA_TO_PIXEL(or1, og1, ob1, oa1), or1, og1, ob1, oa1); \
        STORE_Z(pz[_a], zz);                                            \
      }                                                                 \
    }                                                                   \
    z+=dzdx;                                                            \
    og1+=dgdx;                                                          \
    or1+=drdx;                                                          \
    ob1+=dbdx;                                                          \
    oa1+=dadx;                                                          \
  }

#define PIXEL_COUNT pixel_count_smooth_untextured

#include "ztriangle.h"
}

static void
FNAME(white_textured) (ZBuffer *zb,
                       ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
  ZTextureDef *texture_def;

#define INTERP_Z
#define INTERP_ST

#define EARLY_OUT()                             \
  {                                             \
  }

#define DRAW_INIT()                             \
  {                                             \
    texture_def = &zb->current_textures[0];     \
  }

#define PUT_PIXEL(_a)                                                   \
  {                                                                     \
    zz=z >> ZB_POINT_Z_FRAC_BITS;                                       \
    if (ZCMP(pz[_a], zz)) {                                             \
      tmp = ZB_LOOKUP_TEXTURE(texture_def, s, t, mipmap_level, mipmap_dx); \
      if (ACMP(zb, PIXEL_A(tmp))) {                                     \
        STORE_PIX(pp[_a], tmp, PIXEL_R(tmp), PIXEL_G(tmp), PIXEL_B(tmp), PIXEL_A(tmp)); \
        STORE_Z(pz[_a], zz);                                            \
      }                                                                 \
    }                                                                   \
    z+=dzdx;                                                            \
    s+=dsdx;                                                            \
    t+=dtdx;                                                            \
  }

#define PIXEL_COUNT pixel_count_white_textured

#include "ztriangle.h"
}

static void
FNAME(flat_textured) (ZBuffer *zb,
                      ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
  ZTextureDef *texture_def;
  UNUSED int or0, og0, ob0, oa0;

#define INTERP_Z
#define INTERP_ST

#define EARLY_OUT()                             \
  {                                             \
  }

#define DRAW_INIT()                                                     \
  {                                                                     \
    if (p2->a == 0 && !ACMP(zb, p2->a)) {                               \
      /* This alpha is zero, and we'll never get other than 0. */       \
      return;                                                           \
    }                                                                   \
    texture_def = &zb->current_textures[0];                             \
    or0 = p2->r;                                                        \
    og0 = p2->g;                                                        \
    ob0 = p2->b;                                                        \
    oa0 = p2->a;                                                        \
  }

#define PUT_PIXEL(_a)                                                   \
  {                                                                     \
    zz=z >> ZB_POINT_Z_FRAC_BITS;                                       \
    if (ZCMP(pz[_a], zz)) {                                             \
      tmp = ZB_LOOKUP_TEXTURE(texture_def, s, t, mipmap_level, mipmap_dx); \
      UNUSED int a = PALPHA_MULT(oa0, PIXEL_A(tmp));                    \
      if (ACMP(zb, a)) {                                                \
        STORE_PIX(pp[_a],                                               \
                  RGBA_TO_PIXEL(PCOMPONENT_MULT(or0, PIXEL_R(tmp)),     \
                                PCOMPONENT_MULT(og0, PIXEL_G(tmp)),     \
                                PCOMPONENT_MULT(ob0, PIXEL_B(tmp)),     \
                                a),                                     \
                  PCOMPONENT_MULT(or0, PIXEL_R(tmp)),                   \
                  PCOMPONENT_MULT(og0, PIXEL_G(tmp)),                   \
                  PCOMPONENT_MULT(ob0, PIXEL_B(tmp)),                   \
                  a);                                                   \
        STORE_Z(pz[_a], zz);                                            \
      }                                                                 \
    }                                                                   \
    z+=dzdx;                                                            \
    s+=dsdx;                                                            \
    t+=dtdx;                                                            \
  }

#define PIXEL_COUNT pixel_count_flat_textured

#include "ztriangle.h"
}

static void
FNAME(smooth_textured) (ZBuffer *zb,
                        ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
  ZTextureDef *texture_def;

#define INTERP_Z
#define INTERP_ST
#define INTERP_RGB

#define EARLY_OUT()                                     \
  {                                                     \
    unsigned int c0, c1, c2;                            \
    c0 = RGBA_TO_PIXEL(p0->r, p0->g, p0->b, p0->a);     \
    c1 = RGBA_TO_PIXEL(p1->r, p1->g, p1->b, p1->a);     \
    c2 = RGBA_TO_PIXEL(p2->r, p2->g, p2->b, p2->a);     \
    if (c0 == c1 && c0 == c2) {                         \
      /* It's really a flat-shaded triangle. */         \
      if (c0 == 0xffffffffu) {                          \
        /* Actually, it's a white triangle. */          \
        FNAME(white_textured)(zb, p0, p1, p2);          \
        return;                                         \
      }                                                 \
      FNAME(flat_textured)(zb, p0, p1, p2);             \
      return;                                           \
    }                                                   \
  }

#define DRAW_INIT()                             \
  {                                             \
    texture_def = &zb->current_textures[0];     \
  }

#define PUT_PIXEL(_a)                                                   \
  {                                                                     \
    zz=z >> ZB_POINT_Z_FRAC_BITS;                                       \
    if (ZCMP(pz[_a], zz)) {                                             \
      tmp = ZB_LOOKUP_TEXTURE(texture_def, s, t, mipmap_level, mipmap_dx); \
      UNUSED int a = PALPHA_MULT(oa1, PIXEL_A(tmp));                    \
      if (ACMP(zb, a)) {                                                \
        STORE_PIX(pp[_a],                                               \
                  RGBA_TO_PIXEL(PCOMPONENT_MULT(or1, PIXEL_R(tmp)),     \
                                PCOMPONENT_MULT(og1, PIXEL_G(tmp)),     \
                                PCOMPONENT_MULT(ob1, PIXEL_B(tmp)),     \
                                a),                                     \
                  PCOMPONENT_MULT(or1, PIXEL_R(tmp)),                   \
                  PCOMPONENT_MULT(og1, PIXEL_G(tmp)),                   \
                  PCOMPONENT_MULT(ob1, PIXEL_B(tmp)),                   \
                  a);                                                   \
        STORE_Z(pz[_a], zz);                                            \
      }                                                                 \
    }                                                                   \
    z+=dzdx;                                                            \
    og1+=dgdx;                                                          \
    or1+=drdx;                                                          \
    ob1+=dbdx;                                                          \
    oa1+=dadx;                                                          \
    s+=dsdx;                                                            \
    t+=dtdx;                                                            \
  }

#define PIXEL_COUNT pixel_count_smooth_textured

#include "ztriangle.h"
}

/*
 * Texture mapping with perspective correction.
 * We use the gradient method to make less divisions.
 * TODO: pipeline the division
 */

static void
FNAME(white_perspective) (ZBuffer *zb,
                          ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
  ZTextureDef *texture_def;
  PN_stdfloat fdzdx,fndzdx,ndszdx,ndtzdx;

#define INTERP_Z
#define INTERP_STZ

#define NB_INTERP 8

#define EARLY_OUT()                             \
  {                                             \
  }

#define DRAW_INIT()                             \
  {                                             \
    texture_def = &zb->current_textures[0];     \
    fdzdx=(PN_stdfloat)dzdx;                          \
    fndzdx=NB_INTERP * fdzdx;                   \
    ndszdx=NB_INTERP * dszdx;                   \
    ndtzdx=NB_INTERP * dtzdx;                   \
  }


#define PUT_PIXEL(_a)                                                   \
  {                                                                     \
    zz=z >> ZB_POINT_Z_FRAC_BITS;                                       \
    if (ZCMP(pz[_a], zz)) {                                             \
      tmp = ZB_LOOKUP_TEXTURE(texture_def, s, t, mipmap_level, mipmap_dx); \
      if (ACMP(zb, PIXEL_A(tmp))) {                                     \
        STORE_PIX(pp[_a], tmp, PIXEL_R(tmp), PIXEL_G(tmp), PIXEL_B(tmp), PIXEL_A(tmp)); \
        STORE_Z(pz[_a], zz);                                            \
      }                                                                 \
    }                                                                   \
    z+=dzdx;                                                            \
    s+=dsdx;                                                            \
    t+=dtdx;                                                            \
  }

#define DRAW_LINE()                                             \
  {                                                             \
    ZPOINT *pz;                                                 \
    PIXEL *pp;                                                  \
    int s,t,z,zz;                                               \
    int n,dsdx,dtdx;                                            \
    PN_stdfloat sz,tz,fz,zinv;                                  \
    n=(x2>>16)-x1;                                              \
    fz=(PN_stdfloat)z1;                                         \
    zinv=1.0f / fz;                                             \
    pp=(PIXEL *)((char *)pp1 + x1 * PSZB);                      \
    pz=pz1+x1;                                                  \
    z=z1;                                                       \
    sz=sz1;                                                     \
    tz=tz1;                                                     \
    while (n>=(NB_INTERP-1)) {                                  \
      {                                                         \
        PN_stdfloat ss,tt;                                      \
        ss=(sz * zinv);                                         \
        tt=(tz * zinv);                                         \
        s=(int) ss;                                             \
        t=(int) tt;                                             \
        dsdx= (int)( (dszdx - ss*fdzdx)*zinv );                 \
        dtdx= (int)( (dtzdx - tt*fdzdx)*zinv );                 \
        CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx); \
        fz+=fndzdx;                                             \
        zinv=1.0f / fz;                                         \
      }                                                         \
      PUT_PIXEL(0);                                             \
      PUT_PIXEL(1);                                             \
      PUT_PIXEL(2);                                             \
      PUT_PIXEL(3);                                             \
      PUT_PIXEL(4);                                             \
      PUT_PIXEL(5);                                             \
      PUT_PIXEL(6);                                             \
      PUT_PIXEL(7);                                             \
      pz+=NB_INTERP;                                            \
      pp=(PIXEL *)((char *)pp + NB_INTERP * PSZB);              \
      n-=NB_INTERP;                                             \
      sz+=ndszdx;                                               \
      tz+=ndtzdx;                                               \
    }                                                           \
    {                                                           \
      PN_stdfloat ss,tt;                                              \
      ss=(sz * zinv);                                           \
      tt=(tz * zinv);                                           \
      s=(int) ss;                                               \
      t=(int) tt;                                               \
      dsdx= (int)( (dszdx - ss*fdzdx)*zinv );                   \
      dtdx= (int)( (dtzdx - tt*fdzdx)*zinv );                   \
      CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx);   \
    }                                                           \
    while (n>=0) {                                              \
      PUT_PIXEL(0);                                             \
      pz+=1;                                                    \
      pp=(PIXEL *)((char *)pp + PSZB);                          \
      n-=1;                                                     \
    }                                                           \
  }
  
#define PIXEL_COUNT pixel_count_white_perspective

#include "ztriangle.h"
}

/*
 * Flat shaded triangle, with perspective-correct mapping.
 */

static void
FNAME(flat_perspective) (ZBuffer *zb,
                         ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
  ZTextureDef *texture_def;
  PN_stdfloat fdzdx,fndzdx,ndszdx,ndtzdx;
  UNUSED int or0, og0, ob0, oa0;

#define INTERP_Z
#define INTERP_STZ
#define INTERP_RGB

#define EARLY_OUT()                             \
  {                                             \
  }

#define DRAW_INIT()                                                     \
  {                                                                     \
    if (p2->a == 0 && !ACMP(zb, p2->a)) {                               \
      /* This alpha is zero, and we'll never get other than 0. */       \
      return;                                                           \
    }                                                                   \
    texture_def = &zb->current_textures[0];                             \
    fdzdx=(PN_stdfloat)dzdx;                                                  \
    fndzdx=NB_INTERP * fdzdx;                                           \
    ndszdx=NB_INTERP * dszdx;                                           \
    ndtzdx=NB_INTERP * dtzdx;                                           \
    or0 = p2->r;                                                        \
    og0 = p2->g;                                                        \
    ob0 = p2->b;                                                        \
    oa0 = p2->a;                                                        \
  }

#define PUT_PIXEL(_a)                                                   \
  {                                                                     \
    zz=z >> ZB_POINT_Z_FRAC_BITS;                                       \
    if (ZCMP(pz[_a], zz)) {                                             \
      tmp = ZB_LOOKUP_TEXTURE(texture_def, s, t, mipmap_level, mipmap_dx); \
      UNUSED int a = PALPHA_MULT(oa0, PIXEL_A(tmp));                    \
      if (ACMP(zb, a)) {                                                \
        STORE_PIX(pp[_a],                                               \
                  RGBA_TO_PIXEL(PCOMPONENT_MULT(or0, PIXEL_R(tmp)),     \
                                PCOMPONENT_MULT(og0, PIXEL_G(tmp)),     \
                                PCOMPONENT_MULT(ob0, PIXEL_B(tmp)),     \
                                a),                                     \
                  PCOMPONENT_MULT(or0, PIXEL_R(tmp)),                   \
                  PCOMPONENT_MULT(og0, PIXEL_G(tmp)),                   \
                  PCOMPONENT_MULT(ob0, PIXEL_B(tmp)),                   \
                  a);                                                   \
        STORE_Z(pz[_a], zz);                                            \
      }                                                                 \
    }                                                                   \
    z+=dzdx;                                                            \
    s+=dsdx;                                                            \
    t+=dtdx;                                                            \
  }

#define DRAW_LINE()                                             \
  {                                                             \
    ZPOINT *pz;                                                 \
    PIXEL *pp;                                                  \
    int s,t,z,zz;                                               \
    int n,dsdx,dtdx;                                            \
    UNUSED int or1,og1,ob1,oa1;                                 \
    PN_stdfloat sz,tz,fz,zinv;                                  \
    n=(x2>>16)-x1;                                              \
    fz=(PN_stdfloat)z1;                                         \
    zinv=1.0f / fz;                                             \
    pp=(PIXEL *)((char *)pp1 + x1 * PSZB);                      \
    pz=pz1+x1;                                                  \
    z=z1;                                                       \
    sz=sz1;                                                     \
    tz=tz1;                                                     \
    or1 = r1;                                                   \
    og1 = g1;                                                   \
    ob1 = b1;                                                   \
    oa1 = a1;                                                   \
    while (n>=(NB_INTERP-1)) {                                  \
      {                                                         \
        PN_stdfloat ss,tt;                                      \
        ss=(sz * zinv);                                         \
        tt=(tz * zinv);                                         \
        s=(int) ss;                                             \
        t=(int) tt;                                             \
        dsdx= (int)( (dszdx - ss*fdzdx)*zinv );                 \
        dtdx= (int)( (dtzdx - tt*fdzdx)*zinv );                 \
        CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx); \
        fz+=fndzdx;                                             \
        zinv=1.0f / fz;                                         \
      }                                                         \
      PUT_PIXEL(0);                                             \
      PUT_PIXEL(1);                                             \
      PUT_PIXEL(2);                                             \
      PUT_PIXEL(3);                                             \
      PUT_PIXEL(4);                                             \
      PUT_PIXEL(5);                                             \
      PUT_PIXEL(6);                                             \
      PUT_PIXEL(7);                                             \
      pz+=NB_INTERP;                                            \
      pp=(PIXEL *)((char *)pp + NB_INTERP * PSZB);              \
      n-=NB_INTERP;                                             \
      sz+=ndszdx;                                               \
      tz+=ndtzdx;                                               \
    }                                                           \
    {                                                           \
      PN_stdfloat ss,tt;                                              \
      ss=(sz * zinv);                                           \
      tt=(tz * zinv);                                           \
      s=(int) ss;                                               \
      t=(int) tt;                                               \
      dsdx= (int)( (dszdx - ss*fdzdx)*zinv );                   \
      dtdx= (int)( (dtzdx - tt*fdzdx)*zinv );                   \
      CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx);   \
    }                                                           \
    while (n>=0) {                                              \
      PUT_PIXEL(0);                                             \
      pz+=1;                                                    \
      pp=(PIXEL *)((char *)pp + PSZB);                          \
      n-=1;                                                     \
    }                                                           \
  }

#define PIXEL_COUNT pixel_count_flat_perspective

#include "ztriangle.h"
}

/*
 * Smooth filled triangle, with perspective-correct mapping.
 */

static void
FNAME(smooth_perspective) (ZBuffer *zb,
                           ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
  ZTextureDef *texture_def;
  PN_stdfloat fdzdx,fndzdx,ndszdx,ndtzdx;

#define INTERP_Z
#define INTERP_STZ
#define INTERP_RGB

#define EARLY_OUT()                                     \
  {                                                     \
    unsigned int c0, c1, c2;                            \
    c0 = RGBA_TO_PIXEL(p0->r, p0->g, p0->b, p0->a);     \
    c1 = RGBA_TO_PIXEL(p1->r, p1->g, p1->b, p1->a);     \
    c2 = RGBA_TO_PIXEL(p2->r, p2->g, p2->b, p2->a);     \
    if (c0 == c1 && c0 == c2) {                         \
      /* It's really a flat-shaded triangle. */         \
      if (c0 == 0xffffffffu) {                          \
        /* Actually, it's a white triangle. */          \
        FNAME(white_perspective)(zb, p0, p1, p2);       \
        return;                                         \
      }                                                 \
      FNAME(flat_perspective)(zb, p0, p1, p2);          \
      return;                                           \
    }                                                   \
  }

#define DRAW_INIT()                             \
  {                                             \
    texture_def = &zb->current_textures[0];     \
    fdzdx=(PN_stdfloat)dzdx;                          \
    fndzdx=NB_INTERP * fdzdx;                   \
    ndszdx=NB_INTERP * dszdx;                   \
    ndtzdx=NB_INTERP * dtzdx;                   \
  }

#define PUT_PIXEL(_a)                                                   \
  {                                                                     \
    zz=z >> ZB_POINT_Z_FRAC_BITS;                                       \
    if (ZCMP(pz[_a], zz)) {                                             \
      tmp = ZB_LOOKUP_TEXTURE(texture_def, s, t, mipmap_level, mipmap_dx); \
      UNUSED int a = PALPHA_MULT(oa1, PIXEL_A(tmp));                    \
      if (ACMP(zb, a)) {                                                \
        STORE_PIX(pp[_a],                                               \
                  RGBA_TO_PIXEL(PCOMPONENT_MULT(or1, PIXEL_R(tmp)),     \
                                PCOMPONENT_MULT(og1, PIXEL_G(tmp)),     \
                                PCOMPONENT_MULT(ob1, PIXEL_B(tmp)),     \
                                a),                                     \
                  PCOMPONENT_MULT(or1, PIXEL_R(tmp)),                   \
                  PCOMPONENT_MULT(og1, PIXEL_G(tmp)),                   \
                  PCOMPONENT_MULT(ob1, PIXEL_B(tmp)),                   \
                  a);                                                   \
        STORE_Z(pz[_a], zz);                                            \
      }                                                                 \
    }                                                                   \
    z+=dzdx;                                                            \
    og1+=dgdx;                                                          \
    or1+=drdx;                                                          \
    ob1+=dbdx;                                                          \
    oa1+=dadx;                                                          \
    s+=dsdx;                                                            \
    t+=dtdx;                                                            \
  }

#define DRAW_LINE()                                             \
  {                                                             \
    ZPOINT *pz;                                                 \
    PIXEL *pp;                                                  \
    int s,t,z,zz;                                               \
    int n,dsdx,dtdx;                                            \
    UNUSED int or1,og1,ob1,oa1;                                 \
    PN_stdfloat sz,tz,fz,zinv;                                  \
    n=(x2>>16)-x1;                                              \
    fz=(PN_stdfloat)z1;                                         \
    zinv=1.0f / fz;                                             \
    pp=(PIXEL *)((char *)pp1 + x1 * PSZB);                      \
    pz=pz1+x1;                                                  \
    z=z1;                                                       \
    sz=sz1;                                                     \
    tz=tz1;                                                     \
    or1 = r1;                                                   \
    og1 = g1;                                                   \
    ob1 = b1;                                                   \
    oa1 = a1;                                                   \
    while (n>=(NB_INTERP-1)) {                                  \
      {                                                         \
        PN_stdfloat ss,tt;                                      \
        ss=(sz * zinv);                                         \
        tt=(tz * zinv);                                         \
        s=(int) ss;                                             \
        t=(int) tt;                                             \
        dsdx= (int)( (dszdx - ss*fdzdx)*zinv );                 \
        dtdx= (int)( (dtzdx - tt*fdzdx)*zinv );                 \
        CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx); \
        fz+=fndzdx;                                             \
        zinv=1.0f / fz;                                         \
      }                                                         \
      PUT_PIXEL(0);                                             \
      PUT_PIXEL(1);                                             \
      PUT_PIXEL(2);                                             \
      PUT_PIXEL(3);                                             \
      PUT_PIXEL(4);                                             \
      PUT_PIXEL(5);                                             \
      PUT_PIXEL(6);                                             \
      PUT_PIXEL(7);                                             \
      pz+=NB_INTERP;                                            \
      pp=(PIXEL *)((char *)pp + NB_INTERP * PSZB);              \
      n-=NB_INTERP;                                             \
      sz+=ndszdx;                                               \
      tz+=ndtzdx;                                               \
    }                                                           \
    {                                                           \
      PN_stdfloat ss,tt;                                              \
      ss=(sz * zinv);                                           \
      tt=(tz * zinv);                                           \
      s=(int) ss;                                               \
      t=(int) tt;                                               \
      dsdx= (int)( (dszdx - ss*fdzdx)*zinv );                   \
      dtdx= (int)( (dtzdx - tt*fdzdx)*zinv );                   \
      CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx);   \
    }                                                           \
    while (n>=0) {                                              \
      PUT_PIXEL(0);                                             \
      pz+=1;                                                    \
      pp=(PIXEL *)((char *)pp + PSZB);                          \
      n-=1;                                                     \
    }                                                           \
  }

#define PIXEL_COUNT pixel_count_smooth_perspective

#include "ztriangle.h"
}

/*
 * Smooth filled triangle, with perspective-correct mapping, on two
 * stages of multitexture.
 */

static void
FNAME(smooth_multitex2) (ZBuffer *zb,
                         ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
  PN_stdfloat fdzdx,fndzdx,ndszdx,ndtzdx,ndszadx,ndtzadx;

#define INTERP_Z
#define INTERP_STZ
#define INTERP_STZA
#define INTERP_RGB

#define EARLY_OUT()                             \
  {                                             \
  }

#define DRAW_INIT()                             \
  {                                             \
    fdzdx=(PN_stdfloat)dzdx;                          \
    fndzdx=NB_INTERP * fdzdx;                   \
    ndszdx=NB_INTERP * dszdx;                   \
    ndtzdx=NB_INTERP * dtzdx;                   \
    ndszadx=NB_INTERP * dszadx;                 \
    ndtzadx=NB_INTERP * dtzadx;                 \
  }

#define PUT_PIXEL(_a)                                                   \
  {                                                                     \
    zz=z >> ZB_POINT_Z_FRAC_BITS;                                       \
    if (ZCMP(pz[_a], zz)) {                                             \
      tmp = ZB_LOOKUP_TEXTURE(&zb->current_textures[0], s, t, mipmap_level, mipmap_dx); \
      UNUSED int a = PALPHA_MULT(oa1, PIXEL_A(tmp));                    \
      if (ACMP(zb, a)) {                                                \
        UNUSED int tmpa = ZB_LOOKUP_TEXTURE(&zb->current_textures[1], sa, ta, mipmap_levela, mipmap_dxa); \
        STORE_PIX(pp[_a],                                               \
                  RGBA_TO_PIXEL(PCOMPONENT_MULT3(or1, PIXEL_R(tmp), PIXEL_R(tmpa)), \
                                PCOMPONENT_MULT3(og1, PIXEL_G(tmp), PIXEL_G(tmpa)), \
                                PCOMPONENT_MULT3(ob1, PIXEL_B(tmp), PIXEL_B(tmpa)), \
                                a),                                     \
                  PCOMPONENT_MULT3(or1, PIXEL_R(tmp), PIXEL_R(tmpa)),   \
                  PCOMPONENT_MULT3(og1, PIXEL_G(tmp), PIXEL_G(tmpa)),   \
                  PCOMPONENT_MULT3(ob1, PIXEL_B(tmp), PIXEL_B(tmpa)),   \
                  a);                                                   \
        STORE_Z(pz[_a], zz);                                            \
      }                                                                 \
    }                                                                   \
    z+=dzdx;                                                            \
    og1+=dgdx;                                                          \
    or1+=drdx;                                                          \
    ob1+=dbdx;                                                          \
    oa1+=dadx;                                                          \
    s+=dsdx;                                                            \
    t+=dtdx;                                                            \
    sa+=dsadx;                                                          \
    ta+=dtadx;                                                          \
  }

#define DRAW_LINE()                                                     \
  {                                                                     \
    ZPOINT *pz;                                                         \
    PIXEL *pp;                                                          \
    int s,t,sa,ta,z,zz;                                                 \
    int n,dsdx,dtdx,dsadx,dtadx;                                        \
    UNUSED int or1,og1,ob1,oa1;                                         \
    PN_stdfloat sz,tz,sza,tza,fz,zinv;                                  \
    n=(x2>>16)-x1;                                                      \
    fz=(PN_stdfloat)z1;                                                 \
    zinv=1.0f / fz;                                                     \
    pp=(PIXEL *)((char *)pp1 + x1 * PSZB);                              \
    pz=pz1+x1;                                                          \
    z=z1;                                                               \
    sz=sz1;                                                             \
    tz=tz1;                                                             \
    sza=sza1;                                                           \
    tza=tza1;                                                           \
    or1 = r1;                                                           \
    og1 = g1;                                                           \
    ob1 = b1;                                                           \
    oa1 = a1;                                                           \
    while (n>=(NB_INTERP-1)) {                                          \
      {                                                                 \
        PN_stdfloat ss,tt;                                              \
        ss=(sz * zinv);                                                 \
        tt=(tz * zinv);                                                 \
        s=(int) ss;                                                     \
        t=(int) tt;                                                     \
        dsdx= (int)( (dszdx - ss*fdzdx)*zinv );                         \
        dtdx= (int)( (dtzdx - tt*fdzdx)*zinv );                         \
        CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx);         \
      }                                                                 \
      {                                                                 \
        PN_stdfloat ssa,tta;                                            \
        ssa=(sza * zinv);                                               \
        tta=(tza * zinv);                                               \
        sa=(int) ssa;                                                   \
        ta=(int) tta;                                                   \
        dsadx= (int)( (dszadx - ssa*fdzdx)*zinv );                      \
        dtadx= (int)( (dtzadx - tta*fdzdx)*zinv );                      \
        CALC_MIPMAP_LEVEL(mipmap_levela, mipmap_dxa, dsadx, dtadx);     \
      }                                                                 \
      fz+=fndzdx;                                                       \
      zinv=1.0f / fz;                                                   \
      PUT_PIXEL(0);                                                     \
      PUT_PIXEL(1);                                                     \
      PUT_PIXEL(2);                                                     \
      PUT_PIXEL(3);                                                     \
      PUT_PIXEL(4);                                                     \
      PUT_PIXEL(5);                                                     \
      PUT_PIXEL(6);                                                     \
      PUT_PIXEL(7);                                                     \
      pz+=NB_INTERP;                                                    \
      pp=(PIXEL *)((char *)pp + NB_INTERP * PSZB);                      \
      n-=NB_INTERP;                                                     \
      sz+=ndszdx;                                                       \
      tz+=ndtzdx;                                                       \
      sza+=ndszadx;                                                     \
      tza+=ndtzadx;                                                     \
    }                                                                   \
    {                                                                   \
      PN_stdfloat ss,tt;                                                \
      ss=(sz * zinv);                                                   \
      tt=(tz * zinv);                                                   \
      s=(int) ss;                                                       \
      t=(int) tt;                                                       \
      dsdx= (int)( (dszdx - ss*fdzdx)*zinv );                           \
      dtdx= (int)( (dtzdx - tt*fdzdx)*zinv );                           \
      CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx);           \
    }                                                                   \
    {                                                                   \
      PN_stdfloat ssa,tta;                                              \
      ssa=(sza * zinv);                                                 \
      tta=(tza * zinv);                                                 \
      sa=(int) ssa;                                                     \
      ta=(int) tta;                                                     \
      dsadx= (int)( (dszadx - ssa*fdzdx)*zinv );                        \
      dtadx= (int)( (dtzadx - tta*fdzdx)*zinv );                        \
      CALC_MIPMAP_LEVEL(mipmap_levela, mipmap_dxa, dsadx, dtadx);       \
    }                                                                   \
    while (n>=0) {                                                      \
      PUT_PIXEL(0);                                                     \
      pz+=1;                                                            \
      pp=(PIXEL *)((char *)pp + PSZB);                                  \
      n-=1;                                                             \
    }                                                                   \
  }

#define PIXEL_COUNT pixel_count_smooth_multitex2

#include "ztriangle.h"
}

/*
 * Smooth filled triangle, with perspective-correct mapping, on three
 * stages of multitexture.
 */

static void
FNAME(smooth_multitex3) (ZBuffer *zb,
                         ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
  PN_stdfloat fdzdx,fndzdx,ndszdx,ndtzdx,ndszadx,ndtzadx,ndszbdx,ndtzbdx;

#define INTERP_Z
#define INTERP_STZ
#define INTERP_STZA
#define INTERP_STZB
#define INTERP_RGB

#define EARLY_OUT()                             \
  {                                             \
  }

#define DRAW_INIT()                             \
  {                                             \
    fdzdx=(PN_stdfloat)dzdx;                          \
    fndzdx=NB_INTERP * fdzdx;                   \
    ndszdx=NB_INTERP * dszdx;                   \
    ndtzdx=NB_INTERP * dtzdx;                   \
    ndszadx=NB_INTERP * dszadx;                 \
    ndtzadx=NB_INTERP * dtzadx;                 \
    ndszbdx=NB_INTERP * dszbdx;                 \
    ndtzbdx=NB_INTERP * dtzbdx;                 \
  }

#define PUT_PIXEL(_a)                                                   \
  {                                                                     \
    zz=z >> ZB_POINT_Z_FRAC_BITS;                                       \
    if (ZCMP(pz[_a], zz)) {                                             \
      tmp = ZB_LOOKUP_TEXTURE(&zb->current_textures[0], s, t, mipmap_level, mipmap_dx); \
      UNUSED int a = PALPHA_MULT(oa1, PIXEL_A(tmp));                    \
      if (ACMP(zb, a)) {                                                \
        UNUSED int tmpa = ZB_LOOKUP_TEXTURE(&zb->current_textures[1], sa, ta, mipmap_levela, mipmap_dxa); \
        UNUSED int tmpb = ZB_LOOKUP_TEXTURE(&zb->current_textures[2], sb, tb, mipmap_levelb, mipmap_dxb); \
        STORE_PIX(pp[_a],                                               \
                  RGBA_TO_PIXEL(PCOMPONENT_MULT4(or1, PIXEL_R(tmp), PIXEL_R(tmpa), PIXEL_R(tmpb)), \
                                PCOMPONENT_MULT4(og1, PIXEL_G(tmp), PIXEL_G(tmpa), PIXEL_G(tmpb)), \
                                PCOMPONENT_MULT4(ob1, PIXEL_B(tmp), PIXEL_B(tmpa), PIXEL_B(tmpb)), \
                                a),                                     \
                  PCOMPONENT_MULT4(or1, PIXEL_R(tmp), PIXEL_R(tmpa), PIXEL_R(tmpb)), \
                  PCOMPONENT_MULT4(og1, PIXEL_G(tmp), PIXEL_G(tmpa), PIXEL_G(tmpb)), \
                  PCOMPONENT_MULT4(ob1, PIXEL_B(tmp), PIXEL_B(tmpa), PIXEL_B(tmpb)), \
                  a);                                                   \
        STORE_Z(pz[_a], zz);                                            \
      }                                                                 \
    }                                                                   \
    z+=dzdx;                                                            \
    og1+=dgdx;                                                          \
    or1+=drdx;                                                          \
    ob1+=dbdx;                                                          \
    oa1+=dadx;                                                          \
    s+=dsdx;                                                            \
    t+=dtdx;                                                            \
    sa+=dsadx;                                                          \
    ta+=dtadx;                                                          \
    sb+=dsbdx;                                                          \
    tb+=dtbdx;                                                          \
  }

#define DRAW_LINE()                                                     \
  {                                                                     \
    ZPOINT *pz;                                                         \
    PIXEL *pp;                                                          \
    int s,t,sa,ta,sb,tb,z,zz;                                           \
    int n,dsdx,dtdx,dsadx,dtadx,dsbdx,dtbdx;                            \
    UNUSED int or1,og1,ob1,oa1;                                         \
    PN_stdfloat sz,tz,sza,tza,szb,tzb,fz,zinv;                          \
    n=(x2>>16)-x1;                                                      \
    fz=(PN_stdfloat)z1;                                                 \
    zinv=1.0f / fz;                                                     \
    pp=(PIXEL *)((char *)pp1 + x1 * PSZB);                              \
    pz=pz1+x1;                                                          \
    z=z1;                                                               \
    sz=sz1;                                                             \
    tz=tz1;                                                             \
    sza=sza1;                                                           \
    tza=tza1;                                                           \
    szb=szb1;                                                           \
    tzb=tzb1;                                                           \
    or1 = r1;                                                           \
    og1 = g1;                                                           \
    ob1 = b1;                                                           \
    oa1 = a1;                                                           \
    while (n>=(NB_INTERP-1)) {                                          \
      {                                                                 \
        PN_stdfloat ss,tt;                                              \
        ss=(sz * zinv);                                                 \
        tt=(tz * zinv);                                                 \
        s=(int) ss;                                                     \
        t=(int) tt;                                                     \
        dsdx= (int)( (dszdx - ss*fdzdx)*zinv );                         \
        dtdx= (int)( (dtzdx - tt*fdzdx)*zinv );                         \
        CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx);         \
      }                                                                 \
      {                                                                 \
        PN_stdfloat ssa,tta;                                            \
        ssa=(sza * zinv);                                               \
        tta=(tza * zinv);                                               \
        sa=(int) ssa;                                                   \
        ta=(int) tta;                                                   \
        dsadx= (int)( (dszadx - ssa*fdzdx)*zinv );                      \
        dtadx= (int)( (dtzadx - tta*fdzdx)*zinv );                      \
        CALC_MIPMAP_LEVEL(mipmap_levela, mipmap_dxa, dsadx, dtadx);     \
      }                                                                 \
      {                                                                 \
        PN_stdfloat ssb,ttb;                                            \
        ssb=(szb * zinv);                                               \
        ttb=(tzb * zinv);                                               \
        sb=(int) ssb;                                                   \
        tb=(int) ttb;                                                   \
        dsbdx= (int)( (dszbdx - ssb*fdzdx)*zinv );                      \
        dtbdx= (int)( (dtzbdx - ttb*fdzdx)*zinv );                      \
        CALC_MIPMAP_LEVEL(mipmap_levelb, mipmap_dxb, dsbdx, dtbdx);     \
      }                                                                 \
      fz+=fndzdx;                                                       \
      zinv=1.0f / fz;                                                   \
      PUT_PIXEL(0);                                                     \
      PUT_PIXEL(1);                                                     \
      PUT_PIXEL(2);                                                     \
      PUT_PIXEL(3);                                                     \
      PUT_PIXEL(4);                                                     \
      PUT_PIXEL(5);                                                     \
      PUT_PIXEL(6);                                                     \
      PUT_PIXEL(7);                                                     \
      pz+=NB_INTERP;                                                    \
      pp=(PIXEL *)((char *)pp + NB_INTERP * PSZB);                      \
      n-=NB_INTERP;                                                     \
      sz+=ndszdx;                                                       \
      tz+=ndtzdx;                                                       \
      sza+=ndszadx;                                                     \
      tza+=ndtzadx;                                                     \
      szb+=ndszbdx;                                                     \
      tzb+=ndtzbdx;                                                     \
    }                                                                   \
    {                                                                   \
      PN_stdfloat ss,tt;                                                \
      ss=(sz * zinv);                                                   \
      tt=(tz * zinv);                                                   \
      s=(int) ss;                                                       \
      t=(int) tt;                                                       \
      dsdx= (int)( (dszdx - ss*fdzdx)*zinv );                           \
      dtdx= (int)( (dtzdx - tt*fdzdx)*zinv );                           \
      CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx);           \
    }                                                                   \
    {                                                                   \
      PN_stdfloat ssa,tta;                                              \
      ssa=(sza * zinv);                                                 \
      tta=(tza * zinv);                                                 \
      sa=(int) ssa;                                                     \
      ta=(int) tta;                                                     \
      dsadx= (int)( (dszadx - ssa*fdzdx)*zinv );                        \
      dtadx= (int)( (dtzadx - tta*fdzdx)*zinv );                        \
      CALC_MIPMAP_LEVEL(mipmap_levela, mipmap_dxa, dsadx, dtadx);       \
    }                                                                   \
    {                                                                   \
      PN_stdfloat ssb,ttb;                                              \
      ssb=(szb * zinv);                                                 \
      ttb=(tzb * zinv);                                                 \
      sb=(int) ssb;                                                     \
      tb=(int) ttb;                                                     \
      dsbdx= (int)( (dszbdx - ssb*fdzdx)*zinv );                        \
      dtbdx= (int)( (dtzbdx - ttb*fdzdx)*zinv );                        \
      CALC_MIPMAP_LEVEL(mipmap_levelb, mipmap_dxb, dsbdx, dtbdx);       \
    }                                                                   \
    while (n>=0) {                                                      \
      PUT_PIXEL(0);                                                     \
      pz+=1;                                                            \
      pp=(PIXEL *)((char *)pp + PSZB);                                  \
      n-=1;                                                             \
    }                                                                   \
  }

#define PIXEL_COUNT pixel_count_smooth_multitex3

#include "ztriangle.h"
}

#undef ACMP
#undef ZCMP
#undef STORE_PIX
#undef STORE_Z
#undef FNAME
#undef INTERP_MIPMAP
#undef CALC_MIPMAP_LEVEL
#undef ZB_LOOKUP_TEXTURE

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
