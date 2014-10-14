/* This file is generated code--do not edit.  See ztriangle.py. */

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_NEAREST(texture_def, s, t)
#define FNAME(name) FB_triangle_zoff_coff_anone_znone_tnearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_MIPMAP_NEAREST(texture_def, s, t, level)
#define FNAME(name) FB_triangle_zoff_coff_anone_znone_tmipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ((level == 0) ? (texture_def)->tex_magfilter_func(texture_def, s, t, level, level_dx) : (texture_def)->tex_minfilter_func(texture_def, s, t, level, level_dx))
#define FNAME(name) FB_triangle_zoff_coff_anone_znone_tgeneral_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_NEAREST(texture_def, s, t)
#define FNAME(name) FB_triangle_zoff_coff_anone_zless_tnearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_MIPMAP_NEAREST(texture_def, s, t, level)
#define FNAME(name) FB_triangle_zoff_coff_anone_zless_tmipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ((level == 0) ? (texture_def)->tex_magfilter_func(texture_def, s, t, level, level_dx) : (texture_def)->tex_minfilter_func(texture_def, s, t, level, level_dx))
#define FNAME(name) FB_triangle_zoff_coff_anone_zless_tgeneral_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_NEAREST(texture_def, s, t)
#define FNAME(name) FB_triangle_zoff_coff_aless_znone_tnearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_MIPMAP_NEAREST(texture_def, s, t, level)
#define FNAME(name) FB_triangle_zoff_coff_aless_znone_tmipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ((level == 0) ? (texture_def)->tex_magfilter_func(texture_def, s, t, level, level_dx) : (texture_def)->tex_minfilter_func(texture_def, s, t, level, level_dx))
#define FNAME(name) FB_triangle_zoff_coff_aless_znone_tgeneral_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_NEAREST(texture_def, s, t)
#define FNAME(name) FB_triangle_zoff_coff_aless_zless_tnearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_MIPMAP_NEAREST(texture_def, s, t, level)
#define FNAME(name) FB_triangle_zoff_coff_aless_zless_tmipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ((level == 0) ? (texture_def)->tex_magfilter_func(texture_def, s, t, level, level_dx) : (texture_def)->tex_minfilter_func(texture_def, s, t, level, level_dx))
#define FNAME(name) FB_triangle_zoff_coff_aless_zless_tgeneral_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_NEAREST(texture_def, s, t)
#define FNAME(name) FB_triangle_zoff_coff_amore_znone_tnearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_MIPMAP_NEAREST(texture_def, s, t, level)
#define FNAME(name) FB_triangle_zoff_coff_amore_znone_tmipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ((level == 0) ? (texture_def)->tex_magfilter_func(texture_def, s, t, level, level_dx) : (texture_def)->tex_minfilter_func(texture_def, s, t, level, level_dx))
#define FNAME(name) FB_triangle_zoff_coff_amore_znone_tgeneral_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_NEAREST(texture_def, s, t)
#define FNAME(name) FB_triangle_zoff_coff_amore_zless_tnearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_MIPMAP_NEAREST(texture_def, s, t, level)
#define FNAME(name) FB_triangle_zoff_coff_amore_zless_tmipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ((level == 0) ? (texture_def)->tex_magfilter_func(texture_def, s, t, level, level_dx) : (texture_def)->tex_minfilter_func(texture_def, s, t, level, level_dx))
#define FNAME(name) FB_triangle_zoff_coff_amore_zless_tgeneral_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = SRGBA_TO_PIXEL(r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_NEAREST(texture_def, s, t)
#define FNAME(name) FB_triangle_zoff_csstore_anone_znone_tnearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = SRGBA_TO_PIXEL(r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_MIPMAP_NEAREST(texture_def, s, t, level)
#define FNAME(name) FB_triangle_zoff_csstore_anone_znone_tmipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = SRGBA_TO_PIXEL(r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ((level == 0) ? (texture_def)->tex_magfilter_func(texture_def, s, t, level, level_dx) : (texture_def)->tex_minfilter_func(texture_def, s, t, level, level_dx))
#define FNAME(name) FB_triangle_zoff_csstore_anone_znone_tgeneral_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = SRGBA_TO_PIXEL(r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_NEAREST(texture_def, s, t)
#define FNAME(name) FB_triangle_zoff_csstore_anone_zless_tnearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = SRGBA_TO_PIXEL(r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_MIPMAP_NEAREST(texture_def, s, t, level)
#define FNAME(name) FB_triangle_zoff_csstore_anone_zless_tmipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = SRGBA_TO_PIXEL(r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ((level == 0) ? (texture_def)->tex_magfilter_func(texture_def, s, t, level, level_dx) : (texture_def)->tex_minfilter_func(texture_def, s, t, level, level_dx))
#define FNAME(name) FB_triangle_zoff_csstore_anone_zless_tgeneral_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = SRGBA_TO_PIXEL(r, g, b, a)
#define ACMP(zb, a) (((int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_NEAREST(texture_def, s, t)
#define FNAME(name) FB_triangle_zoff_csstore_aless_znone_tnearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = SRGBA_TO_PIXEL(r, g, b, a)
#define ACMP(zb, a) (((int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_MIPMAP_NEAREST(texture_def, s, t, level)
#define FNAME(name) FB_triangle_zoff_csstore_aless_znone_tmipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = SRGBA_TO_PIXEL(r, g, b, a)
#define ACMP(zb, a) (((int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ((level == 0) ? (texture_def)->tex_magfilter_func(texture_def, s, t, level, level_dx) : (texture_def)->tex_minfilter_func(texture_def, s, t, level, level_dx))
#define FNAME(name) FB_triangle_zoff_csstore_aless_znone_tgeneral_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = SRGBA_TO_PIXEL(r, g, b, a)
#define ACMP(zb, a) (((int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_NEAREST(texture_def, s, t)
#define FNAME(name) FB_triangle_zoff_csstore_aless_zless_tnearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = SRGBA_TO_PIXEL(r, g, b, a)
#define ACMP(zb, a) (((int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_MIPMAP_NEAREST(texture_def, s, t, level)
#define FNAME(name) FB_triangle_zoff_csstore_aless_zless_tmipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = SRGBA_TO_PIXEL(r, g, b, a)
#define ACMP(zb, a) (((int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ((level == 0) ? (texture_def)->tex_magfilter_func(texture_def, s, t, level, level_dx) : (texture_def)->tex_minfilter_func(texture_def, s, t, level, level_dx))
#define FNAME(name) FB_triangle_zoff_csstore_aless_zless_tgeneral_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = SRGBA_TO_PIXEL(r, g, b, a)
#define ACMP(zb, a) (((int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_NEAREST(texture_def, s, t)
#define FNAME(name) FB_triangle_zoff_csstore_amore_znone_tnearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = SRGBA_TO_PIXEL(r, g, b, a)
#define ACMP(zb, a) (((int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_MIPMAP_NEAREST(texture_def, s, t, level)
#define FNAME(name) FB_triangle_zoff_csstore_amore_znone_tmipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = SRGBA_TO_PIXEL(r, g, b, a)
#define ACMP(zb, a) (((int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ((level == 0) ? (texture_def)->tex_magfilter_func(texture_def, s, t, level, level_dx) : (texture_def)->tex_minfilter_func(texture_def, s, t, level, level_dx))
#define FNAME(name) FB_triangle_zoff_csstore_amore_znone_tgeneral_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = SRGBA_TO_PIXEL(r, g, b, a)
#define ACMP(zb, a) (((int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_NEAREST(texture_def, s, t)
#define FNAME(name) FB_triangle_zoff_csstore_amore_zless_tnearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = SRGBA_TO_PIXEL(r, g, b, a)
#define ACMP(zb, a) (((int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_MIPMAP_NEAREST(texture_def, s, t, level)
#define FNAME(name) FB_triangle_zoff_csstore_amore_zless_tmipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = SRGBA_TO_PIXEL(r, g, b, a)
#define ACMP(zb, a) (((int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ((level == 0) ? (texture_def)->tex_magfilter_func(texture_def, s, t, level, level_dx) : (texture_def)->tex_minfilter_func(texture_def, s, t, level, level_dx))
#define FNAME(name) FB_triangle_zoff_csstore_amore_zless_tgeneral_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_SRGB(pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_NEAREST(texture_def, s, t)
#define FNAME(name) FB_triangle_zoff_csblend_anone_znone_tnearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_SRGB(pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_MIPMAP_NEAREST(texture_def, s, t, level)
#define FNAME(name) FB_triangle_zoff_csblend_anone_znone_tmipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_SRGB(pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ((level == 0) ? (texture_def)->tex_magfilter_func(texture_def, s, t, level, level_dx) : (texture_def)->tex_minfilter_func(texture_def, s, t, level, level_dx))
#define FNAME(name) FB_triangle_zoff_csblend_anone_znone_tgeneral_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_SRGB(pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_NEAREST(texture_def, s, t)
#define FNAME(name) FB_triangle_zoff_csblend_anone_zless_tnearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_SRGB(pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_MIPMAP_NEAREST(texture_def, s, t, level)
#define FNAME(name) FB_triangle_zoff_csblend_anone_zless_tmipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_SRGB(pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ((level == 0) ? (texture_def)->tex_magfilter_func(texture_def, s, t, level, level_dx) : (texture_def)->tex_minfilter_func(texture_def, s, t, level, level_dx))
#define FNAME(name) FB_triangle_zoff_csblend_anone_zless_tgeneral_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_SRGB(pix, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_NEAREST(texture_def, s, t)
#define FNAME(name) FB_triangle_zoff_csblend_aless_znone_tnearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_SRGB(pix, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_MIPMAP_NEAREST(texture_def, s, t, level)
#define FNAME(name) FB_triangle_zoff_csblend_aless_znone_tmipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_SRGB(pix, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ((level == 0) ? (texture_def)->tex_magfilter_func(texture_def, s, t, level, level_dx) : (texture_def)->tex_minfilter_func(texture_def, s, t, level, level_dx))
#define FNAME(name) FB_triangle_zoff_csblend_aless_znone_tgeneral_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_SRGB(pix, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_NEAREST(texture_def, s, t)
#define FNAME(name) FB_triangle_zoff_csblend_aless_zless_tnearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_SRGB(pix, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_MIPMAP_NEAREST(texture_def, s, t, level)
#define FNAME(name) FB_triangle_zoff_csblend_aless_zless_tmipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_SRGB(pix, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ((level == 0) ? (texture_def)->tex_magfilter_func(texture_def, s, t, level, level_dx) : (texture_def)->tex_minfilter_func(texture_def, s, t, level, level_dx))
#define FNAME(name) FB_triangle_zoff_csblend_aless_zless_tgeneral_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_SRGB(pix, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_NEAREST(texture_def, s, t)
#define FNAME(name) FB_triangle_zoff_csblend_amore_znone_tnearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_SRGB(pix, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_MIPMAP_NEAREST(texture_def, s, t, level)
#define FNAME(name) FB_triangle_zoff_csblend_amore_znone_tmipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_SRGB(pix, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ((level == 0) ? (texture_def)->tex_magfilter_func(texture_def, s, t, level, level_dx) : (texture_def)->tex_minfilter_func(texture_def, s, t, level, level_dx))
#define FNAME(name) FB_triangle_zoff_csblend_amore_znone_tgeneral_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_SRGB(pix, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_NEAREST(texture_def, s, t)
#define FNAME(name) FB_triangle_zoff_csblend_amore_zless_tnearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_SRGB(pix, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ZB_LOOKUP_TEXTURE_MIPMAP_NEAREST(texture_def, s, t, level)
#define FNAME(name) FB_triangle_zoff_csblend_amore_zless_tmipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_SRGB(pix, r, g, b, a)
#define ACMP(zb, a) (((int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx) DO_CALC_MIPMAP_LEVEL(mipmap_level, mipmap_dx, dsdx, dtdx)
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_def, s, t, level, level_dx) ((level == 0) ? (texture_def)->tex_magfilter_func(texture_def, s, t, level, level_dx) : (texture_def)->tex_minfilter_func(texture_def, s, t, level, level_dx))
#define FNAME(name) FB_triangle_zoff_csblend_amore_zless_tgeneral_ ## name
#include "ztriangle_two.h"


ZB_fillTriangleFunc ztriangle_code_4[810] = {
  FB_triangle_zoff_coff_anone_znone_tnearest_white_untextured,
  FB_triangle_zoff_coff_anone_znone_tnearest_white_textured,
  FB_triangle_zoff_coff_anone_znone_tnearest_white_perspective,
  FB_triangle_zoff_coff_anone_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_coff_anone_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_coff_anone_znone_tnearest_flat_untextured,
  FB_triangle_zoff_coff_anone_znone_tnearest_flat_textured,
  FB_triangle_zoff_coff_anone_znone_tnearest_flat_perspective,
  FB_triangle_zoff_coff_anone_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_coff_anone_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_coff_anone_znone_tnearest_smooth_untextured,
  FB_triangle_zoff_coff_anone_znone_tnearest_smooth_textured,
  FB_triangle_zoff_coff_anone_znone_tnearest_smooth_perspective,
  FB_triangle_zoff_coff_anone_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_coff_anone_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_coff_anone_znone_tmipmap_white_untextured,
  FB_triangle_zoff_coff_anone_znone_tmipmap_white_textured,
  FB_triangle_zoff_coff_anone_znone_tmipmap_white_perspective,
  FB_triangle_zoff_coff_anone_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_coff_anone_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_coff_anone_znone_tmipmap_flat_untextured,
  FB_triangle_zoff_coff_anone_znone_tmipmap_flat_textured,
  FB_triangle_zoff_coff_anone_znone_tmipmap_flat_perspective,
  FB_triangle_zoff_coff_anone_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_coff_anone_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_coff_anone_znone_tmipmap_smooth_untextured,
  FB_triangle_zoff_coff_anone_znone_tmipmap_smooth_textured,
  FB_triangle_zoff_coff_anone_znone_tmipmap_smooth_perspective,
  FB_triangle_zoff_coff_anone_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_coff_anone_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_coff_anone_znone_tgeneral_white_untextured,
  FB_triangle_zoff_coff_anone_znone_tgeneral_white_textured,
  FB_triangle_zoff_coff_anone_znone_tgeneral_white_perspective,
  FB_triangle_zoff_coff_anone_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_coff_anone_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_coff_anone_znone_tgeneral_flat_untextured,
  FB_triangle_zoff_coff_anone_znone_tgeneral_flat_textured,
  FB_triangle_zoff_coff_anone_znone_tgeneral_flat_perspective,
  FB_triangle_zoff_coff_anone_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_coff_anone_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_coff_anone_znone_tgeneral_smooth_untextured,
  FB_triangle_zoff_coff_anone_znone_tgeneral_smooth_textured,
  FB_triangle_zoff_coff_anone_znone_tgeneral_smooth_perspective,
  FB_triangle_zoff_coff_anone_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_coff_anone_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_coff_anone_zless_tnearest_white_untextured,
  FB_triangle_zoff_coff_anone_zless_tnearest_white_textured,
  FB_triangle_zoff_coff_anone_zless_tnearest_white_perspective,
  FB_triangle_zoff_coff_anone_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_coff_anone_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_coff_anone_zless_tnearest_flat_untextured,
  FB_triangle_zoff_coff_anone_zless_tnearest_flat_textured,
  FB_triangle_zoff_coff_anone_zless_tnearest_flat_perspective,
  FB_triangle_zoff_coff_anone_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_coff_anone_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_coff_anone_zless_tnearest_smooth_untextured,
  FB_triangle_zoff_coff_anone_zless_tnearest_smooth_textured,
  FB_triangle_zoff_coff_anone_zless_tnearest_smooth_perspective,
  FB_triangle_zoff_coff_anone_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_coff_anone_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_coff_anone_zless_tmipmap_white_untextured,
  FB_triangle_zoff_coff_anone_zless_tmipmap_white_textured,
  FB_triangle_zoff_coff_anone_zless_tmipmap_white_perspective,
  FB_triangle_zoff_coff_anone_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_coff_anone_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_coff_anone_zless_tmipmap_flat_untextured,
  FB_triangle_zoff_coff_anone_zless_tmipmap_flat_textured,
  FB_triangle_zoff_coff_anone_zless_tmipmap_flat_perspective,
  FB_triangle_zoff_coff_anone_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_coff_anone_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_coff_anone_zless_tmipmap_smooth_untextured,
  FB_triangle_zoff_coff_anone_zless_tmipmap_smooth_textured,
  FB_triangle_zoff_coff_anone_zless_tmipmap_smooth_perspective,
  FB_triangle_zoff_coff_anone_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_coff_anone_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_coff_anone_zless_tgeneral_white_untextured,
  FB_triangle_zoff_coff_anone_zless_tgeneral_white_textured,
  FB_triangle_zoff_coff_anone_zless_tgeneral_white_perspective,
  FB_triangle_zoff_coff_anone_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_coff_anone_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_coff_anone_zless_tgeneral_flat_untextured,
  FB_triangle_zoff_coff_anone_zless_tgeneral_flat_textured,
  FB_triangle_zoff_coff_anone_zless_tgeneral_flat_perspective,
  FB_triangle_zoff_coff_anone_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_coff_anone_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_coff_anone_zless_tgeneral_smooth_untextured,
  FB_triangle_zoff_coff_anone_zless_tgeneral_smooth_textured,
  FB_triangle_zoff_coff_anone_zless_tgeneral_smooth_perspective,
  FB_triangle_zoff_coff_anone_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_coff_anone_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_coff_aless_znone_tnearest_white_untextured,
  FB_triangle_zoff_coff_aless_znone_tnearest_white_textured,
  FB_triangle_zoff_coff_aless_znone_tnearest_white_perspective,
  FB_triangle_zoff_coff_aless_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_coff_aless_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_coff_aless_znone_tnearest_flat_untextured,
  FB_triangle_zoff_coff_aless_znone_tnearest_flat_textured,
  FB_triangle_zoff_coff_aless_znone_tnearest_flat_perspective,
  FB_triangle_zoff_coff_aless_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_coff_aless_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_coff_aless_znone_tnearest_smooth_untextured,
  FB_triangle_zoff_coff_aless_znone_tnearest_smooth_textured,
  FB_triangle_zoff_coff_aless_znone_tnearest_smooth_perspective,
  FB_triangle_zoff_coff_aless_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_coff_aless_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_coff_aless_znone_tmipmap_white_untextured,
  FB_triangle_zoff_coff_aless_znone_tmipmap_white_textured,
  FB_triangle_zoff_coff_aless_znone_tmipmap_white_perspective,
  FB_triangle_zoff_coff_aless_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_coff_aless_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_coff_aless_znone_tmipmap_flat_untextured,
  FB_triangle_zoff_coff_aless_znone_tmipmap_flat_textured,
  FB_triangle_zoff_coff_aless_znone_tmipmap_flat_perspective,
  FB_triangle_zoff_coff_aless_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_coff_aless_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_coff_aless_znone_tmipmap_smooth_untextured,
  FB_triangle_zoff_coff_aless_znone_tmipmap_smooth_textured,
  FB_triangle_zoff_coff_aless_znone_tmipmap_smooth_perspective,
  FB_triangle_zoff_coff_aless_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_coff_aless_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_coff_aless_znone_tgeneral_white_untextured,
  FB_triangle_zoff_coff_aless_znone_tgeneral_white_textured,
  FB_triangle_zoff_coff_aless_znone_tgeneral_white_perspective,
  FB_triangle_zoff_coff_aless_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_coff_aless_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_coff_aless_znone_tgeneral_flat_untextured,
  FB_triangle_zoff_coff_aless_znone_tgeneral_flat_textured,
  FB_triangle_zoff_coff_aless_znone_tgeneral_flat_perspective,
  FB_triangle_zoff_coff_aless_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_coff_aless_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_coff_aless_znone_tgeneral_smooth_untextured,
  FB_triangle_zoff_coff_aless_znone_tgeneral_smooth_textured,
  FB_triangle_zoff_coff_aless_znone_tgeneral_smooth_perspective,
  FB_triangle_zoff_coff_aless_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_coff_aless_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_coff_aless_zless_tnearest_white_untextured,
  FB_triangle_zoff_coff_aless_zless_tnearest_white_textured,
  FB_triangle_zoff_coff_aless_zless_tnearest_white_perspective,
  FB_triangle_zoff_coff_aless_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_coff_aless_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_coff_aless_zless_tnearest_flat_untextured,
  FB_triangle_zoff_coff_aless_zless_tnearest_flat_textured,
  FB_triangle_zoff_coff_aless_zless_tnearest_flat_perspective,
  FB_triangle_zoff_coff_aless_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_coff_aless_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_coff_aless_zless_tnearest_smooth_untextured,
  FB_triangle_zoff_coff_aless_zless_tnearest_smooth_textured,
  FB_triangle_zoff_coff_aless_zless_tnearest_smooth_perspective,
  FB_triangle_zoff_coff_aless_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_coff_aless_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_coff_aless_zless_tmipmap_white_untextured,
  FB_triangle_zoff_coff_aless_zless_tmipmap_white_textured,
  FB_triangle_zoff_coff_aless_zless_tmipmap_white_perspective,
  FB_triangle_zoff_coff_aless_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_coff_aless_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_coff_aless_zless_tmipmap_flat_untextured,
  FB_triangle_zoff_coff_aless_zless_tmipmap_flat_textured,
  FB_triangle_zoff_coff_aless_zless_tmipmap_flat_perspective,
  FB_triangle_zoff_coff_aless_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_coff_aless_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_coff_aless_zless_tmipmap_smooth_untextured,
  FB_triangle_zoff_coff_aless_zless_tmipmap_smooth_textured,
  FB_triangle_zoff_coff_aless_zless_tmipmap_smooth_perspective,
  FB_triangle_zoff_coff_aless_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_coff_aless_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_coff_aless_zless_tgeneral_white_untextured,
  FB_triangle_zoff_coff_aless_zless_tgeneral_white_textured,
  FB_triangle_zoff_coff_aless_zless_tgeneral_white_perspective,
  FB_triangle_zoff_coff_aless_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_coff_aless_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_coff_aless_zless_tgeneral_flat_untextured,
  FB_triangle_zoff_coff_aless_zless_tgeneral_flat_textured,
  FB_triangle_zoff_coff_aless_zless_tgeneral_flat_perspective,
  FB_triangle_zoff_coff_aless_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_coff_aless_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_coff_aless_zless_tgeneral_smooth_untextured,
  FB_triangle_zoff_coff_aless_zless_tgeneral_smooth_textured,
  FB_triangle_zoff_coff_aless_zless_tgeneral_smooth_perspective,
  FB_triangle_zoff_coff_aless_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_coff_aless_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_coff_amore_znone_tnearest_white_untextured,
  FB_triangle_zoff_coff_amore_znone_tnearest_white_textured,
  FB_triangle_zoff_coff_amore_znone_tnearest_white_perspective,
  FB_triangle_zoff_coff_amore_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_coff_amore_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_coff_amore_znone_tnearest_flat_untextured,
  FB_triangle_zoff_coff_amore_znone_tnearest_flat_textured,
  FB_triangle_zoff_coff_amore_znone_tnearest_flat_perspective,
  FB_triangle_zoff_coff_amore_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_coff_amore_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_coff_amore_znone_tnearest_smooth_untextured,
  FB_triangle_zoff_coff_amore_znone_tnearest_smooth_textured,
  FB_triangle_zoff_coff_amore_znone_tnearest_smooth_perspective,
  FB_triangle_zoff_coff_amore_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_coff_amore_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_coff_amore_znone_tmipmap_white_untextured,
  FB_triangle_zoff_coff_amore_znone_tmipmap_white_textured,
  FB_triangle_zoff_coff_amore_znone_tmipmap_white_perspective,
  FB_triangle_zoff_coff_amore_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_coff_amore_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_coff_amore_znone_tmipmap_flat_untextured,
  FB_triangle_zoff_coff_amore_znone_tmipmap_flat_textured,
  FB_triangle_zoff_coff_amore_znone_tmipmap_flat_perspective,
  FB_triangle_zoff_coff_amore_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_coff_amore_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_coff_amore_znone_tmipmap_smooth_untextured,
  FB_triangle_zoff_coff_amore_znone_tmipmap_smooth_textured,
  FB_triangle_zoff_coff_amore_znone_tmipmap_smooth_perspective,
  FB_triangle_zoff_coff_amore_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_coff_amore_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_coff_amore_znone_tgeneral_white_untextured,
  FB_triangle_zoff_coff_amore_znone_tgeneral_white_textured,
  FB_triangle_zoff_coff_amore_znone_tgeneral_white_perspective,
  FB_triangle_zoff_coff_amore_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_coff_amore_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_coff_amore_znone_tgeneral_flat_untextured,
  FB_triangle_zoff_coff_amore_znone_tgeneral_flat_textured,
  FB_triangle_zoff_coff_amore_znone_tgeneral_flat_perspective,
  FB_triangle_zoff_coff_amore_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_coff_amore_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_coff_amore_znone_tgeneral_smooth_untextured,
  FB_triangle_zoff_coff_amore_znone_tgeneral_smooth_textured,
  FB_triangle_zoff_coff_amore_znone_tgeneral_smooth_perspective,
  FB_triangle_zoff_coff_amore_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_coff_amore_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_coff_amore_zless_tnearest_white_untextured,
  FB_triangle_zoff_coff_amore_zless_tnearest_white_textured,
  FB_triangle_zoff_coff_amore_zless_tnearest_white_perspective,
  FB_triangle_zoff_coff_amore_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_coff_amore_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_coff_amore_zless_tnearest_flat_untextured,
  FB_triangle_zoff_coff_amore_zless_tnearest_flat_textured,
  FB_triangle_zoff_coff_amore_zless_tnearest_flat_perspective,
  FB_triangle_zoff_coff_amore_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_coff_amore_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_coff_amore_zless_tnearest_smooth_untextured,
  FB_triangle_zoff_coff_amore_zless_tnearest_smooth_textured,
  FB_triangle_zoff_coff_amore_zless_tnearest_smooth_perspective,
  FB_triangle_zoff_coff_amore_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_coff_amore_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_coff_amore_zless_tmipmap_white_untextured,
  FB_triangle_zoff_coff_amore_zless_tmipmap_white_textured,
  FB_triangle_zoff_coff_amore_zless_tmipmap_white_perspective,
  FB_triangle_zoff_coff_amore_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_coff_amore_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_coff_amore_zless_tmipmap_flat_untextured,
  FB_triangle_zoff_coff_amore_zless_tmipmap_flat_textured,
  FB_triangle_zoff_coff_amore_zless_tmipmap_flat_perspective,
  FB_triangle_zoff_coff_amore_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_coff_amore_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_coff_amore_zless_tmipmap_smooth_untextured,
  FB_triangle_zoff_coff_amore_zless_tmipmap_smooth_textured,
  FB_triangle_zoff_coff_amore_zless_tmipmap_smooth_perspective,
  FB_triangle_zoff_coff_amore_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_coff_amore_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_coff_amore_zless_tgeneral_white_untextured,
  FB_triangle_zoff_coff_amore_zless_tgeneral_white_textured,
  FB_triangle_zoff_coff_amore_zless_tgeneral_white_perspective,
  FB_triangle_zoff_coff_amore_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_coff_amore_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_coff_amore_zless_tgeneral_flat_untextured,
  FB_triangle_zoff_coff_amore_zless_tgeneral_flat_textured,
  FB_triangle_zoff_coff_amore_zless_tgeneral_flat_perspective,
  FB_triangle_zoff_coff_amore_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_coff_amore_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_coff_amore_zless_tgeneral_smooth_untextured,
  FB_triangle_zoff_coff_amore_zless_tgeneral_smooth_textured,
  FB_triangle_zoff_coff_amore_zless_tgeneral_smooth_perspective,
  FB_triangle_zoff_coff_amore_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_coff_amore_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csstore_anone_znone_tnearest_white_untextured,
  FB_triangle_zoff_csstore_anone_znone_tnearest_white_textured,
  FB_triangle_zoff_csstore_anone_znone_tnearest_white_perspective,
  FB_triangle_zoff_csstore_anone_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_csstore_anone_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_csstore_anone_znone_tnearest_flat_untextured,
  FB_triangle_zoff_csstore_anone_znone_tnearest_flat_textured,
  FB_triangle_zoff_csstore_anone_znone_tnearest_flat_perspective,
  FB_triangle_zoff_csstore_anone_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_csstore_anone_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_csstore_anone_znone_tnearest_smooth_untextured,
  FB_triangle_zoff_csstore_anone_znone_tnearest_smooth_textured,
  FB_triangle_zoff_csstore_anone_znone_tnearest_smooth_perspective,
  FB_triangle_zoff_csstore_anone_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_csstore_anone_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_csstore_anone_znone_tmipmap_white_untextured,
  FB_triangle_zoff_csstore_anone_znone_tmipmap_white_textured,
  FB_triangle_zoff_csstore_anone_znone_tmipmap_white_perspective,
  FB_triangle_zoff_csstore_anone_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csstore_anone_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csstore_anone_znone_tmipmap_flat_untextured,
  FB_triangle_zoff_csstore_anone_znone_tmipmap_flat_textured,
  FB_triangle_zoff_csstore_anone_znone_tmipmap_flat_perspective,
  FB_triangle_zoff_csstore_anone_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csstore_anone_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csstore_anone_znone_tmipmap_smooth_untextured,
  FB_triangle_zoff_csstore_anone_znone_tmipmap_smooth_textured,
  FB_triangle_zoff_csstore_anone_znone_tmipmap_smooth_perspective,
  FB_triangle_zoff_csstore_anone_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csstore_anone_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csstore_anone_znone_tgeneral_white_untextured,
  FB_triangle_zoff_csstore_anone_znone_tgeneral_white_textured,
  FB_triangle_zoff_csstore_anone_znone_tgeneral_white_perspective,
  FB_triangle_zoff_csstore_anone_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csstore_anone_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csstore_anone_znone_tgeneral_flat_untextured,
  FB_triangle_zoff_csstore_anone_znone_tgeneral_flat_textured,
  FB_triangle_zoff_csstore_anone_znone_tgeneral_flat_perspective,
  FB_triangle_zoff_csstore_anone_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csstore_anone_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csstore_anone_znone_tgeneral_smooth_untextured,
  FB_triangle_zoff_csstore_anone_znone_tgeneral_smooth_textured,
  FB_triangle_zoff_csstore_anone_znone_tgeneral_smooth_perspective,
  FB_triangle_zoff_csstore_anone_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csstore_anone_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csstore_anone_zless_tnearest_white_untextured,
  FB_triangle_zoff_csstore_anone_zless_tnearest_white_textured,
  FB_triangle_zoff_csstore_anone_zless_tnearest_white_perspective,
  FB_triangle_zoff_csstore_anone_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_csstore_anone_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_csstore_anone_zless_tnearest_flat_untextured,
  FB_triangle_zoff_csstore_anone_zless_tnearest_flat_textured,
  FB_triangle_zoff_csstore_anone_zless_tnearest_flat_perspective,
  FB_triangle_zoff_csstore_anone_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_csstore_anone_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_csstore_anone_zless_tnearest_smooth_untextured,
  FB_triangle_zoff_csstore_anone_zless_tnearest_smooth_textured,
  FB_triangle_zoff_csstore_anone_zless_tnearest_smooth_perspective,
  FB_triangle_zoff_csstore_anone_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_csstore_anone_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_csstore_anone_zless_tmipmap_white_untextured,
  FB_triangle_zoff_csstore_anone_zless_tmipmap_white_textured,
  FB_triangle_zoff_csstore_anone_zless_tmipmap_white_perspective,
  FB_triangle_zoff_csstore_anone_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csstore_anone_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csstore_anone_zless_tmipmap_flat_untextured,
  FB_triangle_zoff_csstore_anone_zless_tmipmap_flat_textured,
  FB_triangle_zoff_csstore_anone_zless_tmipmap_flat_perspective,
  FB_triangle_zoff_csstore_anone_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csstore_anone_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csstore_anone_zless_tmipmap_smooth_untextured,
  FB_triangle_zoff_csstore_anone_zless_tmipmap_smooth_textured,
  FB_triangle_zoff_csstore_anone_zless_tmipmap_smooth_perspective,
  FB_triangle_zoff_csstore_anone_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csstore_anone_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csstore_anone_zless_tgeneral_white_untextured,
  FB_triangle_zoff_csstore_anone_zless_tgeneral_white_textured,
  FB_triangle_zoff_csstore_anone_zless_tgeneral_white_perspective,
  FB_triangle_zoff_csstore_anone_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csstore_anone_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csstore_anone_zless_tgeneral_flat_untextured,
  FB_triangle_zoff_csstore_anone_zless_tgeneral_flat_textured,
  FB_triangle_zoff_csstore_anone_zless_tgeneral_flat_perspective,
  FB_triangle_zoff_csstore_anone_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csstore_anone_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csstore_anone_zless_tgeneral_smooth_untextured,
  FB_triangle_zoff_csstore_anone_zless_tgeneral_smooth_textured,
  FB_triangle_zoff_csstore_anone_zless_tgeneral_smooth_perspective,
  FB_triangle_zoff_csstore_anone_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csstore_anone_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csstore_aless_znone_tnearest_white_untextured,
  FB_triangle_zoff_csstore_aless_znone_tnearest_white_textured,
  FB_triangle_zoff_csstore_aless_znone_tnearest_white_perspective,
  FB_triangle_zoff_csstore_aless_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_csstore_aless_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_csstore_aless_znone_tnearest_flat_untextured,
  FB_triangle_zoff_csstore_aless_znone_tnearest_flat_textured,
  FB_triangle_zoff_csstore_aless_znone_tnearest_flat_perspective,
  FB_triangle_zoff_csstore_aless_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_csstore_aless_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_csstore_aless_znone_tnearest_smooth_untextured,
  FB_triangle_zoff_csstore_aless_znone_tnearest_smooth_textured,
  FB_triangle_zoff_csstore_aless_znone_tnearest_smooth_perspective,
  FB_triangle_zoff_csstore_aless_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_csstore_aless_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_csstore_aless_znone_tmipmap_white_untextured,
  FB_triangle_zoff_csstore_aless_znone_tmipmap_white_textured,
  FB_triangle_zoff_csstore_aless_znone_tmipmap_white_perspective,
  FB_triangle_zoff_csstore_aless_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csstore_aless_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csstore_aless_znone_tmipmap_flat_untextured,
  FB_triangle_zoff_csstore_aless_znone_tmipmap_flat_textured,
  FB_triangle_zoff_csstore_aless_znone_tmipmap_flat_perspective,
  FB_triangle_zoff_csstore_aless_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csstore_aless_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csstore_aless_znone_tmipmap_smooth_untextured,
  FB_triangle_zoff_csstore_aless_znone_tmipmap_smooth_textured,
  FB_triangle_zoff_csstore_aless_znone_tmipmap_smooth_perspective,
  FB_triangle_zoff_csstore_aless_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csstore_aless_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csstore_aless_znone_tgeneral_white_untextured,
  FB_triangle_zoff_csstore_aless_znone_tgeneral_white_textured,
  FB_triangle_zoff_csstore_aless_znone_tgeneral_white_perspective,
  FB_triangle_zoff_csstore_aless_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csstore_aless_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csstore_aless_znone_tgeneral_flat_untextured,
  FB_triangle_zoff_csstore_aless_znone_tgeneral_flat_textured,
  FB_triangle_zoff_csstore_aless_znone_tgeneral_flat_perspective,
  FB_triangle_zoff_csstore_aless_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csstore_aless_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csstore_aless_znone_tgeneral_smooth_untextured,
  FB_triangle_zoff_csstore_aless_znone_tgeneral_smooth_textured,
  FB_triangle_zoff_csstore_aless_znone_tgeneral_smooth_perspective,
  FB_triangle_zoff_csstore_aless_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csstore_aless_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csstore_aless_zless_tnearest_white_untextured,
  FB_triangle_zoff_csstore_aless_zless_tnearest_white_textured,
  FB_triangle_zoff_csstore_aless_zless_tnearest_white_perspective,
  FB_triangle_zoff_csstore_aless_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_csstore_aless_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_csstore_aless_zless_tnearest_flat_untextured,
  FB_triangle_zoff_csstore_aless_zless_tnearest_flat_textured,
  FB_triangle_zoff_csstore_aless_zless_tnearest_flat_perspective,
  FB_triangle_zoff_csstore_aless_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_csstore_aless_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_csstore_aless_zless_tnearest_smooth_untextured,
  FB_triangle_zoff_csstore_aless_zless_tnearest_smooth_textured,
  FB_triangle_zoff_csstore_aless_zless_tnearest_smooth_perspective,
  FB_triangle_zoff_csstore_aless_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_csstore_aless_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_csstore_aless_zless_tmipmap_white_untextured,
  FB_triangle_zoff_csstore_aless_zless_tmipmap_white_textured,
  FB_triangle_zoff_csstore_aless_zless_tmipmap_white_perspective,
  FB_triangle_zoff_csstore_aless_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csstore_aless_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csstore_aless_zless_tmipmap_flat_untextured,
  FB_triangle_zoff_csstore_aless_zless_tmipmap_flat_textured,
  FB_triangle_zoff_csstore_aless_zless_tmipmap_flat_perspective,
  FB_triangle_zoff_csstore_aless_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csstore_aless_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csstore_aless_zless_tmipmap_smooth_untextured,
  FB_triangle_zoff_csstore_aless_zless_tmipmap_smooth_textured,
  FB_triangle_zoff_csstore_aless_zless_tmipmap_smooth_perspective,
  FB_triangle_zoff_csstore_aless_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csstore_aless_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csstore_aless_zless_tgeneral_white_untextured,
  FB_triangle_zoff_csstore_aless_zless_tgeneral_white_textured,
  FB_triangle_zoff_csstore_aless_zless_tgeneral_white_perspective,
  FB_triangle_zoff_csstore_aless_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csstore_aless_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csstore_aless_zless_tgeneral_flat_untextured,
  FB_triangle_zoff_csstore_aless_zless_tgeneral_flat_textured,
  FB_triangle_zoff_csstore_aless_zless_tgeneral_flat_perspective,
  FB_triangle_zoff_csstore_aless_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csstore_aless_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csstore_aless_zless_tgeneral_smooth_untextured,
  FB_triangle_zoff_csstore_aless_zless_tgeneral_smooth_textured,
  FB_triangle_zoff_csstore_aless_zless_tgeneral_smooth_perspective,
  FB_triangle_zoff_csstore_aless_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csstore_aless_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csstore_amore_znone_tnearest_white_untextured,
  FB_triangle_zoff_csstore_amore_znone_tnearest_white_textured,
  FB_triangle_zoff_csstore_amore_znone_tnearest_white_perspective,
  FB_triangle_zoff_csstore_amore_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_csstore_amore_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_csstore_amore_znone_tnearest_flat_untextured,
  FB_triangle_zoff_csstore_amore_znone_tnearest_flat_textured,
  FB_triangle_zoff_csstore_amore_znone_tnearest_flat_perspective,
  FB_triangle_zoff_csstore_amore_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_csstore_amore_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_csstore_amore_znone_tnearest_smooth_untextured,
  FB_triangle_zoff_csstore_amore_znone_tnearest_smooth_textured,
  FB_triangle_zoff_csstore_amore_znone_tnearest_smooth_perspective,
  FB_triangle_zoff_csstore_amore_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_csstore_amore_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_csstore_amore_znone_tmipmap_white_untextured,
  FB_triangle_zoff_csstore_amore_znone_tmipmap_white_textured,
  FB_triangle_zoff_csstore_amore_znone_tmipmap_white_perspective,
  FB_triangle_zoff_csstore_amore_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csstore_amore_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csstore_amore_znone_tmipmap_flat_untextured,
  FB_triangle_zoff_csstore_amore_znone_tmipmap_flat_textured,
  FB_triangle_zoff_csstore_amore_znone_tmipmap_flat_perspective,
  FB_triangle_zoff_csstore_amore_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csstore_amore_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csstore_amore_znone_tmipmap_smooth_untextured,
  FB_triangle_zoff_csstore_amore_znone_tmipmap_smooth_textured,
  FB_triangle_zoff_csstore_amore_znone_tmipmap_smooth_perspective,
  FB_triangle_zoff_csstore_amore_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csstore_amore_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csstore_amore_znone_tgeneral_white_untextured,
  FB_triangle_zoff_csstore_amore_znone_tgeneral_white_textured,
  FB_triangle_zoff_csstore_amore_znone_tgeneral_white_perspective,
  FB_triangle_zoff_csstore_amore_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csstore_amore_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csstore_amore_znone_tgeneral_flat_untextured,
  FB_triangle_zoff_csstore_amore_znone_tgeneral_flat_textured,
  FB_triangle_zoff_csstore_amore_znone_tgeneral_flat_perspective,
  FB_triangle_zoff_csstore_amore_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csstore_amore_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csstore_amore_znone_tgeneral_smooth_untextured,
  FB_triangle_zoff_csstore_amore_znone_tgeneral_smooth_textured,
  FB_triangle_zoff_csstore_amore_znone_tgeneral_smooth_perspective,
  FB_triangle_zoff_csstore_amore_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csstore_amore_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csstore_amore_zless_tnearest_white_untextured,
  FB_triangle_zoff_csstore_amore_zless_tnearest_white_textured,
  FB_triangle_zoff_csstore_amore_zless_tnearest_white_perspective,
  FB_triangle_zoff_csstore_amore_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_csstore_amore_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_csstore_amore_zless_tnearest_flat_untextured,
  FB_triangle_zoff_csstore_amore_zless_tnearest_flat_textured,
  FB_triangle_zoff_csstore_amore_zless_tnearest_flat_perspective,
  FB_triangle_zoff_csstore_amore_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_csstore_amore_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_csstore_amore_zless_tnearest_smooth_untextured,
  FB_triangle_zoff_csstore_amore_zless_tnearest_smooth_textured,
  FB_triangle_zoff_csstore_amore_zless_tnearest_smooth_perspective,
  FB_triangle_zoff_csstore_amore_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_csstore_amore_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_csstore_amore_zless_tmipmap_white_untextured,
  FB_triangle_zoff_csstore_amore_zless_tmipmap_white_textured,
  FB_triangle_zoff_csstore_amore_zless_tmipmap_white_perspective,
  FB_triangle_zoff_csstore_amore_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csstore_amore_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csstore_amore_zless_tmipmap_flat_untextured,
  FB_triangle_zoff_csstore_amore_zless_tmipmap_flat_textured,
  FB_triangle_zoff_csstore_amore_zless_tmipmap_flat_perspective,
  FB_triangle_zoff_csstore_amore_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csstore_amore_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csstore_amore_zless_tmipmap_smooth_untextured,
  FB_triangle_zoff_csstore_amore_zless_tmipmap_smooth_textured,
  FB_triangle_zoff_csstore_amore_zless_tmipmap_smooth_perspective,
  FB_triangle_zoff_csstore_amore_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csstore_amore_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csstore_amore_zless_tgeneral_white_untextured,
  FB_triangle_zoff_csstore_amore_zless_tgeneral_white_textured,
  FB_triangle_zoff_csstore_amore_zless_tgeneral_white_perspective,
  FB_triangle_zoff_csstore_amore_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csstore_amore_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csstore_amore_zless_tgeneral_flat_untextured,
  FB_triangle_zoff_csstore_amore_zless_tgeneral_flat_textured,
  FB_triangle_zoff_csstore_amore_zless_tgeneral_flat_perspective,
  FB_triangle_zoff_csstore_amore_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csstore_amore_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csstore_amore_zless_tgeneral_smooth_untextured,
  FB_triangle_zoff_csstore_amore_zless_tgeneral_smooth_textured,
  FB_triangle_zoff_csstore_amore_zless_tgeneral_smooth_perspective,
  FB_triangle_zoff_csstore_amore_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csstore_amore_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csblend_anone_znone_tnearest_white_untextured,
  FB_triangle_zoff_csblend_anone_znone_tnearest_white_textured,
  FB_triangle_zoff_csblend_anone_znone_tnearest_white_perspective,
  FB_triangle_zoff_csblend_anone_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_csblend_anone_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_csblend_anone_znone_tnearest_flat_untextured,
  FB_triangle_zoff_csblend_anone_znone_tnearest_flat_textured,
  FB_triangle_zoff_csblend_anone_znone_tnearest_flat_perspective,
  FB_triangle_zoff_csblend_anone_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_csblend_anone_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_csblend_anone_znone_tnearest_smooth_untextured,
  FB_triangle_zoff_csblend_anone_znone_tnearest_smooth_textured,
  FB_triangle_zoff_csblend_anone_znone_tnearest_smooth_perspective,
  FB_triangle_zoff_csblend_anone_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_csblend_anone_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_csblend_anone_znone_tmipmap_white_untextured,
  FB_triangle_zoff_csblend_anone_znone_tmipmap_white_textured,
  FB_triangle_zoff_csblend_anone_znone_tmipmap_white_perspective,
  FB_triangle_zoff_csblend_anone_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csblend_anone_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csblend_anone_znone_tmipmap_flat_untextured,
  FB_triangle_zoff_csblend_anone_znone_tmipmap_flat_textured,
  FB_triangle_zoff_csblend_anone_znone_tmipmap_flat_perspective,
  FB_triangle_zoff_csblend_anone_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csblend_anone_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csblend_anone_znone_tmipmap_smooth_untextured,
  FB_triangle_zoff_csblend_anone_znone_tmipmap_smooth_textured,
  FB_triangle_zoff_csblend_anone_znone_tmipmap_smooth_perspective,
  FB_triangle_zoff_csblend_anone_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csblend_anone_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csblend_anone_znone_tgeneral_white_untextured,
  FB_triangle_zoff_csblend_anone_znone_tgeneral_white_textured,
  FB_triangle_zoff_csblend_anone_znone_tgeneral_white_perspective,
  FB_triangle_zoff_csblend_anone_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csblend_anone_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csblend_anone_znone_tgeneral_flat_untextured,
  FB_triangle_zoff_csblend_anone_znone_tgeneral_flat_textured,
  FB_triangle_zoff_csblend_anone_znone_tgeneral_flat_perspective,
  FB_triangle_zoff_csblend_anone_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csblend_anone_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csblend_anone_znone_tgeneral_smooth_untextured,
  FB_triangle_zoff_csblend_anone_znone_tgeneral_smooth_textured,
  FB_triangle_zoff_csblend_anone_znone_tgeneral_smooth_perspective,
  FB_triangle_zoff_csblend_anone_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csblend_anone_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csblend_anone_zless_tnearest_white_untextured,
  FB_triangle_zoff_csblend_anone_zless_tnearest_white_textured,
  FB_triangle_zoff_csblend_anone_zless_tnearest_white_perspective,
  FB_triangle_zoff_csblend_anone_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_csblend_anone_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_csblend_anone_zless_tnearest_flat_untextured,
  FB_triangle_zoff_csblend_anone_zless_tnearest_flat_textured,
  FB_triangle_zoff_csblend_anone_zless_tnearest_flat_perspective,
  FB_triangle_zoff_csblend_anone_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_csblend_anone_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_csblend_anone_zless_tnearest_smooth_untextured,
  FB_triangle_zoff_csblend_anone_zless_tnearest_smooth_textured,
  FB_triangle_zoff_csblend_anone_zless_tnearest_smooth_perspective,
  FB_triangle_zoff_csblend_anone_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_csblend_anone_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_csblend_anone_zless_tmipmap_white_untextured,
  FB_triangle_zoff_csblend_anone_zless_tmipmap_white_textured,
  FB_triangle_zoff_csblend_anone_zless_tmipmap_white_perspective,
  FB_triangle_zoff_csblend_anone_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csblend_anone_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csblend_anone_zless_tmipmap_flat_untextured,
  FB_triangle_zoff_csblend_anone_zless_tmipmap_flat_textured,
  FB_triangle_zoff_csblend_anone_zless_tmipmap_flat_perspective,
  FB_triangle_zoff_csblend_anone_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csblend_anone_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csblend_anone_zless_tmipmap_smooth_untextured,
  FB_triangle_zoff_csblend_anone_zless_tmipmap_smooth_textured,
  FB_triangle_zoff_csblend_anone_zless_tmipmap_smooth_perspective,
  FB_triangle_zoff_csblend_anone_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csblend_anone_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csblend_anone_zless_tgeneral_white_untextured,
  FB_triangle_zoff_csblend_anone_zless_tgeneral_white_textured,
  FB_triangle_zoff_csblend_anone_zless_tgeneral_white_perspective,
  FB_triangle_zoff_csblend_anone_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csblend_anone_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csblend_anone_zless_tgeneral_flat_untextured,
  FB_triangle_zoff_csblend_anone_zless_tgeneral_flat_textured,
  FB_triangle_zoff_csblend_anone_zless_tgeneral_flat_perspective,
  FB_triangle_zoff_csblend_anone_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csblend_anone_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csblend_anone_zless_tgeneral_smooth_untextured,
  FB_triangle_zoff_csblend_anone_zless_tgeneral_smooth_textured,
  FB_triangle_zoff_csblend_anone_zless_tgeneral_smooth_perspective,
  FB_triangle_zoff_csblend_anone_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csblend_anone_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csblend_aless_znone_tnearest_white_untextured,
  FB_triangle_zoff_csblend_aless_znone_tnearest_white_textured,
  FB_triangle_zoff_csblend_aless_znone_tnearest_white_perspective,
  FB_triangle_zoff_csblend_aless_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_csblend_aless_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_csblend_aless_znone_tnearest_flat_untextured,
  FB_triangle_zoff_csblend_aless_znone_tnearest_flat_textured,
  FB_triangle_zoff_csblend_aless_znone_tnearest_flat_perspective,
  FB_triangle_zoff_csblend_aless_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_csblend_aless_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_csblend_aless_znone_tnearest_smooth_untextured,
  FB_triangle_zoff_csblend_aless_znone_tnearest_smooth_textured,
  FB_triangle_zoff_csblend_aless_znone_tnearest_smooth_perspective,
  FB_triangle_zoff_csblend_aless_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_csblend_aless_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_csblend_aless_znone_tmipmap_white_untextured,
  FB_triangle_zoff_csblend_aless_znone_tmipmap_white_textured,
  FB_triangle_zoff_csblend_aless_znone_tmipmap_white_perspective,
  FB_triangle_zoff_csblend_aless_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csblend_aless_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csblend_aless_znone_tmipmap_flat_untextured,
  FB_triangle_zoff_csblend_aless_znone_tmipmap_flat_textured,
  FB_triangle_zoff_csblend_aless_znone_tmipmap_flat_perspective,
  FB_triangle_zoff_csblend_aless_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csblend_aless_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csblend_aless_znone_tmipmap_smooth_untextured,
  FB_triangle_zoff_csblend_aless_znone_tmipmap_smooth_textured,
  FB_triangle_zoff_csblend_aless_znone_tmipmap_smooth_perspective,
  FB_triangle_zoff_csblend_aless_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csblend_aless_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csblend_aless_znone_tgeneral_white_untextured,
  FB_triangle_zoff_csblend_aless_znone_tgeneral_white_textured,
  FB_triangle_zoff_csblend_aless_znone_tgeneral_white_perspective,
  FB_triangle_zoff_csblend_aless_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csblend_aless_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csblend_aless_znone_tgeneral_flat_untextured,
  FB_triangle_zoff_csblend_aless_znone_tgeneral_flat_textured,
  FB_triangle_zoff_csblend_aless_znone_tgeneral_flat_perspective,
  FB_triangle_zoff_csblend_aless_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csblend_aless_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csblend_aless_znone_tgeneral_smooth_untextured,
  FB_triangle_zoff_csblend_aless_znone_tgeneral_smooth_textured,
  FB_triangle_zoff_csblend_aless_znone_tgeneral_smooth_perspective,
  FB_triangle_zoff_csblend_aless_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csblend_aless_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csblend_aless_zless_tnearest_white_untextured,
  FB_triangle_zoff_csblend_aless_zless_tnearest_white_textured,
  FB_triangle_zoff_csblend_aless_zless_tnearest_white_perspective,
  FB_triangle_zoff_csblend_aless_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_csblend_aless_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_csblend_aless_zless_tnearest_flat_untextured,
  FB_triangle_zoff_csblend_aless_zless_tnearest_flat_textured,
  FB_triangle_zoff_csblend_aless_zless_tnearest_flat_perspective,
  FB_triangle_zoff_csblend_aless_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_csblend_aless_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_csblend_aless_zless_tnearest_smooth_untextured,
  FB_triangle_zoff_csblend_aless_zless_tnearest_smooth_textured,
  FB_triangle_zoff_csblend_aless_zless_tnearest_smooth_perspective,
  FB_triangle_zoff_csblend_aless_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_csblend_aless_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_csblend_aless_zless_tmipmap_white_untextured,
  FB_triangle_zoff_csblend_aless_zless_tmipmap_white_textured,
  FB_triangle_zoff_csblend_aless_zless_tmipmap_white_perspective,
  FB_triangle_zoff_csblend_aless_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csblend_aless_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csblend_aless_zless_tmipmap_flat_untextured,
  FB_triangle_zoff_csblend_aless_zless_tmipmap_flat_textured,
  FB_triangle_zoff_csblend_aless_zless_tmipmap_flat_perspective,
  FB_triangle_zoff_csblend_aless_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csblend_aless_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csblend_aless_zless_tmipmap_smooth_untextured,
  FB_triangle_zoff_csblend_aless_zless_tmipmap_smooth_textured,
  FB_triangle_zoff_csblend_aless_zless_tmipmap_smooth_perspective,
  FB_triangle_zoff_csblend_aless_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csblend_aless_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csblend_aless_zless_tgeneral_white_untextured,
  FB_triangle_zoff_csblend_aless_zless_tgeneral_white_textured,
  FB_triangle_zoff_csblend_aless_zless_tgeneral_white_perspective,
  FB_triangle_zoff_csblend_aless_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csblend_aless_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csblend_aless_zless_tgeneral_flat_untextured,
  FB_triangle_zoff_csblend_aless_zless_tgeneral_flat_textured,
  FB_triangle_zoff_csblend_aless_zless_tgeneral_flat_perspective,
  FB_triangle_zoff_csblend_aless_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csblend_aless_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csblend_aless_zless_tgeneral_smooth_untextured,
  FB_triangle_zoff_csblend_aless_zless_tgeneral_smooth_textured,
  FB_triangle_zoff_csblend_aless_zless_tgeneral_smooth_perspective,
  FB_triangle_zoff_csblend_aless_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csblend_aless_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csblend_amore_znone_tnearest_white_untextured,
  FB_triangle_zoff_csblend_amore_znone_tnearest_white_textured,
  FB_triangle_zoff_csblend_amore_znone_tnearest_white_perspective,
  FB_triangle_zoff_csblend_amore_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_csblend_amore_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_csblend_amore_znone_tnearest_flat_untextured,
  FB_triangle_zoff_csblend_amore_znone_tnearest_flat_textured,
  FB_triangle_zoff_csblend_amore_znone_tnearest_flat_perspective,
  FB_triangle_zoff_csblend_amore_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_csblend_amore_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_csblend_amore_znone_tnearest_smooth_untextured,
  FB_triangle_zoff_csblend_amore_znone_tnearest_smooth_textured,
  FB_triangle_zoff_csblend_amore_znone_tnearest_smooth_perspective,
  FB_triangle_zoff_csblend_amore_znone_tnearest_smooth_multitex2,
  FB_triangle_zoff_csblend_amore_znone_tnearest_smooth_multitex3,
  FB_triangle_zoff_csblend_amore_znone_tmipmap_white_untextured,
  FB_triangle_zoff_csblend_amore_znone_tmipmap_white_textured,
  FB_triangle_zoff_csblend_amore_znone_tmipmap_white_perspective,
  FB_triangle_zoff_csblend_amore_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csblend_amore_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csblend_amore_znone_tmipmap_flat_untextured,
  FB_triangle_zoff_csblend_amore_znone_tmipmap_flat_textured,
  FB_triangle_zoff_csblend_amore_znone_tmipmap_flat_perspective,
  FB_triangle_zoff_csblend_amore_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csblend_amore_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csblend_amore_znone_tmipmap_smooth_untextured,
  FB_triangle_zoff_csblend_amore_znone_tmipmap_smooth_textured,
  FB_triangle_zoff_csblend_amore_znone_tmipmap_smooth_perspective,
  FB_triangle_zoff_csblend_amore_znone_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csblend_amore_znone_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csblend_amore_znone_tgeneral_white_untextured,
  FB_triangle_zoff_csblend_amore_znone_tgeneral_white_textured,
  FB_triangle_zoff_csblend_amore_znone_tgeneral_white_perspective,
  FB_triangle_zoff_csblend_amore_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csblend_amore_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csblend_amore_znone_tgeneral_flat_untextured,
  FB_triangle_zoff_csblend_amore_znone_tgeneral_flat_textured,
  FB_triangle_zoff_csblend_amore_znone_tgeneral_flat_perspective,
  FB_triangle_zoff_csblend_amore_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csblend_amore_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csblend_amore_znone_tgeneral_smooth_untextured,
  FB_triangle_zoff_csblend_amore_znone_tgeneral_smooth_textured,
  FB_triangle_zoff_csblend_amore_znone_tgeneral_smooth_perspective,
  FB_triangle_zoff_csblend_amore_znone_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csblend_amore_znone_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csblend_amore_zless_tnearest_white_untextured,
  FB_triangle_zoff_csblend_amore_zless_tnearest_white_textured,
  FB_triangle_zoff_csblend_amore_zless_tnearest_white_perspective,
  FB_triangle_zoff_csblend_amore_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_csblend_amore_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_csblend_amore_zless_tnearest_flat_untextured,
  FB_triangle_zoff_csblend_amore_zless_tnearest_flat_textured,
  FB_triangle_zoff_csblend_amore_zless_tnearest_flat_perspective,
  FB_triangle_zoff_csblend_amore_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_csblend_amore_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_csblend_amore_zless_tnearest_smooth_untextured,
  FB_triangle_zoff_csblend_amore_zless_tnearest_smooth_textured,
  FB_triangle_zoff_csblend_amore_zless_tnearest_smooth_perspective,
  FB_triangle_zoff_csblend_amore_zless_tnearest_smooth_multitex2,
  FB_triangle_zoff_csblend_amore_zless_tnearest_smooth_multitex3,
  FB_triangle_zoff_csblend_amore_zless_tmipmap_white_untextured,
  FB_triangle_zoff_csblend_amore_zless_tmipmap_white_textured,
  FB_triangle_zoff_csblend_amore_zless_tmipmap_white_perspective,
  FB_triangle_zoff_csblend_amore_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csblend_amore_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csblend_amore_zless_tmipmap_flat_untextured,
  FB_triangle_zoff_csblend_amore_zless_tmipmap_flat_textured,
  FB_triangle_zoff_csblend_amore_zless_tmipmap_flat_perspective,
  FB_triangle_zoff_csblend_amore_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csblend_amore_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csblend_amore_zless_tmipmap_smooth_untextured,
  FB_triangle_zoff_csblend_amore_zless_tmipmap_smooth_textured,
  FB_triangle_zoff_csblend_amore_zless_tmipmap_smooth_perspective,
  FB_triangle_zoff_csblend_amore_zless_tmipmap_smooth_multitex2,
  FB_triangle_zoff_csblend_amore_zless_tmipmap_smooth_multitex3,
  FB_triangle_zoff_csblend_amore_zless_tgeneral_white_untextured,
  FB_triangle_zoff_csblend_amore_zless_tgeneral_white_textured,
  FB_triangle_zoff_csblend_amore_zless_tgeneral_white_perspective,
  FB_triangle_zoff_csblend_amore_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csblend_amore_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csblend_amore_zless_tgeneral_flat_untextured,
  FB_triangle_zoff_csblend_amore_zless_tgeneral_flat_textured,
  FB_triangle_zoff_csblend_amore_zless_tgeneral_flat_perspective,
  FB_triangle_zoff_csblend_amore_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csblend_amore_zless_tgeneral_smooth_multitex3,
  FB_triangle_zoff_csblend_amore_zless_tgeneral_smooth_untextured,
  FB_triangle_zoff_csblend_amore_zless_tgeneral_smooth_textured,
  FB_triangle_zoff_csblend_amore_zless_tgeneral_smooth_perspective,
  FB_triangle_zoff_csblend_amore_zless_tgeneral_smooth_multitex2,
  FB_triangle_zoff_csblend_amore_zless_tgeneral_smooth_multitex3,
};
