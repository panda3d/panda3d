/* This file is generated code--do not edit.  See ztriangle.py. */

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_cstore_anone_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_cstore_anone_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_cstore_anone_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_cstore_anone_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_cstore_aless_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_cstore_aless_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_cstore_aless_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_cstore_aless_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_cstore_amore_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_cstore_amore_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_cstore_amore_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_cstore_amore_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_cblend_anone_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_cblend_anone_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_cblend_anone_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_cblend_anone_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_cblend_aless_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_cblend_aless_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_cblend_aless_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_cblend_aless_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_cblend_amore_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_cblend_amore_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_cblend_amore_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_cblend_amore_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_cgeneral_anone_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_cgeneral_anone_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_cgeneral_anone_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_cgeneral_anone_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_cgeneral_aless_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_cgeneral_aless_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_cgeneral_aless_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_cgeneral_aless_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_cgeneral_amore_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_cgeneral_amore_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_cgeneral_amore_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_cgeneral_amore_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_coff_anone_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_coff_anone_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_coff_anone_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_coff_anone_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_coff_aless_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_coff_aless_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_coff_aless_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_coff_aless_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_coff_amore_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_coff_amore_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_coff_amore_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_coff_amore_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_cstore_anone_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_cstore_anone_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_cstore_anone_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_cstore_anone_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_cstore_aless_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_cstore_aless_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_cstore_aless_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_cstore_aless_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_cstore_amore_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_cstore_amore_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_cstore_amore_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_cstore_amore_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_cblend_anone_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_cblend_anone_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_cblend_anone_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_cblend_anone_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_cblend_aless_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_cblend_aless_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_cblend_aless_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_cblend_aless_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_cblend_amore_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_cblend_amore_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_cblend_amore_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_cblend_amore_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_cgeneral_anone_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_cgeneral_anone_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_cgeneral_anone_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_cgeneral_anone_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_cgeneral_aless_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_cgeneral_aless_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_cgeneral_aless_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_cgeneral_aless_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_cgeneral_amore_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_cgeneral_amore_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_cgeneral_amore_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) zb->store_pix_func(zb, pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_cgeneral_amore_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_coff_anone_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_coff_anone_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_coff_anone_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_coff_anone_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_coff_aless_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_coff_aless_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_coff_aless_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_coff_aless_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_coff_amore_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_coff_amore_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_coff_amore_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_coff_amore_zless_mipmap_ ## name
#include "ztriangle_two.h"

const ZB_fillTriangleFunc fill_tri_funcs[2][4][3][2][2][3][3] = {
  {
    {
      {
        {
          {
            {
              FB_triangle_zon_cstore_anone_znone_nearest_white_untextured,
              FB_triangle_zon_cstore_anone_znone_nearest_white_textured,
              FB_triangle_zon_cstore_anone_znone_nearest_white_perspective
            },
            {
              FB_triangle_zon_cstore_anone_znone_nearest_flat_untextured,
              FB_triangle_zon_cstore_anone_znone_nearest_flat_textured,
              FB_triangle_zon_cstore_anone_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zon_cstore_anone_znone_nearest_smooth_untextured,
              FB_triangle_zon_cstore_anone_znone_nearest_smooth_textured,
              FB_triangle_zon_cstore_anone_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_cstore_anone_znone_mipmap_white_untextured,
              FB_triangle_zon_cstore_anone_znone_mipmap_white_textured,
              FB_triangle_zon_cstore_anone_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zon_cstore_anone_znone_mipmap_flat_untextured,
              FB_triangle_zon_cstore_anone_znone_mipmap_flat_textured,
              FB_triangle_zon_cstore_anone_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_cstore_anone_znone_mipmap_smooth_untextured,
              FB_triangle_zon_cstore_anone_znone_mipmap_smooth_textured,
              FB_triangle_zon_cstore_anone_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zon_cstore_anone_zless_nearest_white_untextured,
              FB_triangle_zon_cstore_anone_zless_nearest_white_textured,
              FB_triangle_zon_cstore_anone_zless_nearest_white_perspective
            },
            {
              FB_triangle_zon_cstore_anone_zless_nearest_flat_untextured,
              FB_triangle_zon_cstore_anone_zless_nearest_flat_textured,
              FB_triangle_zon_cstore_anone_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zon_cstore_anone_zless_nearest_smooth_untextured,
              FB_triangle_zon_cstore_anone_zless_nearest_smooth_textured,
              FB_triangle_zon_cstore_anone_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_cstore_anone_zless_mipmap_white_untextured,
              FB_triangle_zon_cstore_anone_zless_mipmap_white_textured,
              FB_triangle_zon_cstore_anone_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zon_cstore_anone_zless_mipmap_flat_untextured,
              FB_triangle_zon_cstore_anone_zless_mipmap_flat_textured,
              FB_triangle_zon_cstore_anone_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_cstore_anone_zless_mipmap_smooth_untextured,
              FB_triangle_zon_cstore_anone_zless_mipmap_smooth_textured,
              FB_triangle_zon_cstore_anone_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zon_cstore_aless_znone_nearest_white_untextured,
              FB_triangle_zon_cstore_aless_znone_nearest_white_textured,
              FB_triangle_zon_cstore_aless_znone_nearest_white_perspective
            },
            {
              FB_triangle_zon_cstore_aless_znone_nearest_flat_untextured,
              FB_triangle_zon_cstore_aless_znone_nearest_flat_textured,
              FB_triangle_zon_cstore_aless_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zon_cstore_aless_znone_nearest_smooth_untextured,
              FB_triangle_zon_cstore_aless_znone_nearest_smooth_textured,
              FB_triangle_zon_cstore_aless_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_cstore_aless_znone_mipmap_white_untextured,
              FB_triangle_zon_cstore_aless_znone_mipmap_white_textured,
              FB_triangle_zon_cstore_aless_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zon_cstore_aless_znone_mipmap_flat_untextured,
              FB_triangle_zon_cstore_aless_znone_mipmap_flat_textured,
              FB_triangle_zon_cstore_aless_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_cstore_aless_znone_mipmap_smooth_untextured,
              FB_triangle_zon_cstore_aless_znone_mipmap_smooth_textured,
              FB_triangle_zon_cstore_aless_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zon_cstore_aless_zless_nearest_white_untextured,
              FB_triangle_zon_cstore_aless_zless_nearest_white_textured,
              FB_triangle_zon_cstore_aless_zless_nearest_white_perspective
            },
            {
              FB_triangle_zon_cstore_aless_zless_nearest_flat_untextured,
              FB_triangle_zon_cstore_aless_zless_nearest_flat_textured,
              FB_triangle_zon_cstore_aless_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zon_cstore_aless_zless_nearest_smooth_untextured,
              FB_triangle_zon_cstore_aless_zless_nearest_smooth_textured,
              FB_triangle_zon_cstore_aless_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_cstore_aless_zless_mipmap_white_untextured,
              FB_triangle_zon_cstore_aless_zless_mipmap_white_textured,
              FB_triangle_zon_cstore_aless_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zon_cstore_aless_zless_mipmap_flat_untextured,
              FB_triangle_zon_cstore_aless_zless_mipmap_flat_textured,
              FB_triangle_zon_cstore_aless_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_cstore_aless_zless_mipmap_smooth_untextured,
              FB_triangle_zon_cstore_aless_zless_mipmap_smooth_textured,
              FB_triangle_zon_cstore_aless_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zon_cstore_amore_znone_nearest_white_untextured,
              FB_triangle_zon_cstore_amore_znone_nearest_white_textured,
              FB_triangle_zon_cstore_amore_znone_nearest_white_perspective
            },
            {
              FB_triangle_zon_cstore_amore_znone_nearest_flat_untextured,
              FB_triangle_zon_cstore_amore_znone_nearest_flat_textured,
              FB_triangle_zon_cstore_amore_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zon_cstore_amore_znone_nearest_smooth_untextured,
              FB_triangle_zon_cstore_amore_znone_nearest_smooth_textured,
              FB_triangle_zon_cstore_amore_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_cstore_amore_znone_mipmap_white_untextured,
              FB_triangle_zon_cstore_amore_znone_mipmap_white_textured,
              FB_triangle_zon_cstore_amore_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zon_cstore_amore_znone_mipmap_flat_untextured,
              FB_triangle_zon_cstore_amore_znone_mipmap_flat_textured,
              FB_triangle_zon_cstore_amore_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_cstore_amore_znone_mipmap_smooth_untextured,
              FB_triangle_zon_cstore_amore_znone_mipmap_smooth_textured,
              FB_triangle_zon_cstore_amore_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zon_cstore_amore_zless_nearest_white_untextured,
              FB_triangle_zon_cstore_amore_zless_nearest_white_textured,
              FB_triangle_zon_cstore_amore_zless_nearest_white_perspective
            },
            {
              FB_triangle_zon_cstore_amore_zless_nearest_flat_untextured,
              FB_triangle_zon_cstore_amore_zless_nearest_flat_textured,
              FB_triangle_zon_cstore_amore_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zon_cstore_amore_zless_nearest_smooth_untextured,
              FB_triangle_zon_cstore_amore_zless_nearest_smooth_textured,
              FB_triangle_zon_cstore_amore_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_cstore_amore_zless_mipmap_white_untextured,
              FB_triangle_zon_cstore_amore_zless_mipmap_white_textured,
              FB_triangle_zon_cstore_amore_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zon_cstore_amore_zless_mipmap_flat_untextured,
              FB_triangle_zon_cstore_amore_zless_mipmap_flat_textured,
              FB_triangle_zon_cstore_amore_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_cstore_amore_zless_mipmap_smooth_untextured,
              FB_triangle_zon_cstore_amore_zless_mipmap_smooth_textured,
              FB_triangle_zon_cstore_amore_zless_mipmap_smooth_perspective
            }
          }
        }
      }
    },
    {
      {
        {
          {
            {
              FB_triangle_zon_cblend_anone_znone_nearest_white_untextured,
              FB_triangle_zon_cblend_anone_znone_nearest_white_textured,
              FB_triangle_zon_cblend_anone_znone_nearest_white_perspective
            },
            {
              FB_triangle_zon_cblend_anone_znone_nearest_flat_untextured,
              FB_triangle_zon_cblend_anone_znone_nearest_flat_textured,
              FB_triangle_zon_cblend_anone_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zon_cblend_anone_znone_nearest_smooth_untextured,
              FB_triangle_zon_cblend_anone_znone_nearest_smooth_textured,
              FB_triangle_zon_cblend_anone_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_cblend_anone_znone_mipmap_white_untextured,
              FB_triangle_zon_cblend_anone_znone_mipmap_white_textured,
              FB_triangle_zon_cblend_anone_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zon_cblend_anone_znone_mipmap_flat_untextured,
              FB_triangle_zon_cblend_anone_znone_mipmap_flat_textured,
              FB_triangle_zon_cblend_anone_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_cblend_anone_znone_mipmap_smooth_untextured,
              FB_triangle_zon_cblend_anone_znone_mipmap_smooth_textured,
              FB_triangle_zon_cblend_anone_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zon_cblend_anone_zless_nearest_white_untextured,
              FB_triangle_zon_cblend_anone_zless_nearest_white_textured,
              FB_triangle_zon_cblend_anone_zless_nearest_white_perspective
            },
            {
              FB_triangle_zon_cblend_anone_zless_nearest_flat_untextured,
              FB_triangle_zon_cblend_anone_zless_nearest_flat_textured,
              FB_triangle_zon_cblend_anone_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zon_cblend_anone_zless_nearest_smooth_untextured,
              FB_triangle_zon_cblend_anone_zless_nearest_smooth_textured,
              FB_triangle_zon_cblend_anone_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_cblend_anone_zless_mipmap_white_untextured,
              FB_triangle_zon_cblend_anone_zless_mipmap_white_textured,
              FB_triangle_zon_cblend_anone_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zon_cblend_anone_zless_mipmap_flat_untextured,
              FB_triangle_zon_cblend_anone_zless_mipmap_flat_textured,
              FB_triangle_zon_cblend_anone_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_cblend_anone_zless_mipmap_smooth_untextured,
              FB_triangle_zon_cblend_anone_zless_mipmap_smooth_textured,
              FB_triangle_zon_cblend_anone_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zon_cblend_aless_znone_nearest_white_untextured,
              FB_triangle_zon_cblend_aless_znone_nearest_white_textured,
              FB_triangle_zon_cblend_aless_znone_nearest_white_perspective
            },
            {
              FB_triangle_zon_cblend_aless_znone_nearest_flat_untextured,
              FB_triangle_zon_cblend_aless_znone_nearest_flat_textured,
              FB_triangle_zon_cblend_aless_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zon_cblend_aless_znone_nearest_smooth_untextured,
              FB_triangle_zon_cblend_aless_znone_nearest_smooth_textured,
              FB_triangle_zon_cblend_aless_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_cblend_aless_znone_mipmap_white_untextured,
              FB_triangle_zon_cblend_aless_znone_mipmap_white_textured,
              FB_triangle_zon_cblend_aless_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zon_cblend_aless_znone_mipmap_flat_untextured,
              FB_triangle_zon_cblend_aless_znone_mipmap_flat_textured,
              FB_triangle_zon_cblend_aless_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_cblend_aless_znone_mipmap_smooth_untextured,
              FB_triangle_zon_cblend_aless_znone_mipmap_smooth_textured,
              FB_triangle_zon_cblend_aless_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zon_cblend_aless_zless_nearest_white_untextured,
              FB_triangle_zon_cblend_aless_zless_nearest_white_textured,
              FB_triangle_zon_cblend_aless_zless_nearest_white_perspective
            },
            {
              FB_triangle_zon_cblend_aless_zless_nearest_flat_untextured,
              FB_triangle_zon_cblend_aless_zless_nearest_flat_textured,
              FB_triangle_zon_cblend_aless_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zon_cblend_aless_zless_nearest_smooth_untextured,
              FB_triangle_zon_cblend_aless_zless_nearest_smooth_textured,
              FB_triangle_zon_cblend_aless_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_cblend_aless_zless_mipmap_white_untextured,
              FB_triangle_zon_cblend_aless_zless_mipmap_white_textured,
              FB_triangle_zon_cblend_aless_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zon_cblend_aless_zless_mipmap_flat_untextured,
              FB_triangle_zon_cblend_aless_zless_mipmap_flat_textured,
              FB_triangle_zon_cblend_aless_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_cblend_aless_zless_mipmap_smooth_untextured,
              FB_triangle_zon_cblend_aless_zless_mipmap_smooth_textured,
              FB_triangle_zon_cblend_aless_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zon_cblend_amore_znone_nearest_white_untextured,
              FB_triangle_zon_cblend_amore_znone_nearest_white_textured,
              FB_triangle_zon_cblend_amore_znone_nearest_white_perspective
            },
            {
              FB_triangle_zon_cblend_amore_znone_nearest_flat_untextured,
              FB_triangle_zon_cblend_amore_znone_nearest_flat_textured,
              FB_triangle_zon_cblend_amore_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zon_cblend_amore_znone_nearest_smooth_untextured,
              FB_triangle_zon_cblend_amore_znone_nearest_smooth_textured,
              FB_triangle_zon_cblend_amore_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_cblend_amore_znone_mipmap_white_untextured,
              FB_triangle_zon_cblend_amore_znone_mipmap_white_textured,
              FB_triangle_zon_cblend_amore_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zon_cblend_amore_znone_mipmap_flat_untextured,
              FB_triangle_zon_cblend_amore_znone_mipmap_flat_textured,
              FB_triangle_zon_cblend_amore_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_cblend_amore_znone_mipmap_smooth_untextured,
              FB_triangle_zon_cblend_amore_znone_mipmap_smooth_textured,
              FB_triangle_zon_cblend_amore_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zon_cblend_amore_zless_nearest_white_untextured,
              FB_triangle_zon_cblend_amore_zless_nearest_white_textured,
              FB_triangle_zon_cblend_amore_zless_nearest_white_perspective
            },
            {
              FB_triangle_zon_cblend_amore_zless_nearest_flat_untextured,
              FB_triangle_zon_cblend_amore_zless_nearest_flat_textured,
              FB_triangle_zon_cblend_amore_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zon_cblend_amore_zless_nearest_smooth_untextured,
              FB_triangle_zon_cblend_amore_zless_nearest_smooth_textured,
              FB_triangle_zon_cblend_amore_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_cblend_amore_zless_mipmap_white_untextured,
              FB_triangle_zon_cblend_amore_zless_mipmap_white_textured,
              FB_triangle_zon_cblend_amore_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zon_cblend_amore_zless_mipmap_flat_untextured,
              FB_triangle_zon_cblend_amore_zless_mipmap_flat_textured,
              FB_triangle_zon_cblend_amore_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_cblend_amore_zless_mipmap_smooth_untextured,
              FB_triangle_zon_cblend_amore_zless_mipmap_smooth_textured,
              FB_triangle_zon_cblend_amore_zless_mipmap_smooth_perspective
            }
          }
        }
      }
    },
    {
      {
        {
          {
            {
              FB_triangle_zon_cgeneral_anone_znone_nearest_white_untextured,
              FB_triangle_zon_cgeneral_anone_znone_nearest_white_textured,
              FB_triangle_zon_cgeneral_anone_znone_nearest_white_perspective
            },
            {
              FB_triangle_zon_cgeneral_anone_znone_nearest_flat_untextured,
              FB_triangle_zon_cgeneral_anone_znone_nearest_flat_textured,
              FB_triangle_zon_cgeneral_anone_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zon_cgeneral_anone_znone_nearest_smooth_untextured,
              FB_triangle_zon_cgeneral_anone_znone_nearest_smooth_textured,
              FB_triangle_zon_cgeneral_anone_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_cgeneral_anone_znone_mipmap_white_untextured,
              FB_triangle_zon_cgeneral_anone_znone_mipmap_white_textured,
              FB_triangle_zon_cgeneral_anone_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zon_cgeneral_anone_znone_mipmap_flat_untextured,
              FB_triangle_zon_cgeneral_anone_znone_mipmap_flat_textured,
              FB_triangle_zon_cgeneral_anone_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_cgeneral_anone_znone_mipmap_smooth_untextured,
              FB_triangle_zon_cgeneral_anone_znone_mipmap_smooth_textured,
              FB_triangle_zon_cgeneral_anone_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zon_cgeneral_anone_zless_nearest_white_untextured,
              FB_triangle_zon_cgeneral_anone_zless_nearest_white_textured,
              FB_triangle_zon_cgeneral_anone_zless_nearest_white_perspective
            },
            {
              FB_triangle_zon_cgeneral_anone_zless_nearest_flat_untextured,
              FB_triangle_zon_cgeneral_anone_zless_nearest_flat_textured,
              FB_triangle_zon_cgeneral_anone_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zon_cgeneral_anone_zless_nearest_smooth_untextured,
              FB_triangle_zon_cgeneral_anone_zless_nearest_smooth_textured,
              FB_triangle_zon_cgeneral_anone_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_cgeneral_anone_zless_mipmap_white_untextured,
              FB_triangle_zon_cgeneral_anone_zless_mipmap_white_textured,
              FB_triangle_zon_cgeneral_anone_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zon_cgeneral_anone_zless_mipmap_flat_untextured,
              FB_triangle_zon_cgeneral_anone_zless_mipmap_flat_textured,
              FB_triangle_zon_cgeneral_anone_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_cgeneral_anone_zless_mipmap_smooth_untextured,
              FB_triangle_zon_cgeneral_anone_zless_mipmap_smooth_textured,
              FB_triangle_zon_cgeneral_anone_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zon_cgeneral_aless_znone_nearest_white_untextured,
              FB_triangle_zon_cgeneral_aless_znone_nearest_white_textured,
              FB_triangle_zon_cgeneral_aless_znone_nearest_white_perspective
            },
            {
              FB_triangle_zon_cgeneral_aless_znone_nearest_flat_untextured,
              FB_triangle_zon_cgeneral_aless_znone_nearest_flat_textured,
              FB_triangle_zon_cgeneral_aless_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zon_cgeneral_aless_znone_nearest_smooth_untextured,
              FB_triangle_zon_cgeneral_aless_znone_nearest_smooth_textured,
              FB_triangle_zon_cgeneral_aless_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_cgeneral_aless_znone_mipmap_white_untextured,
              FB_triangle_zon_cgeneral_aless_znone_mipmap_white_textured,
              FB_triangle_zon_cgeneral_aless_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zon_cgeneral_aless_znone_mipmap_flat_untextured,
              FB_triangle_zon_cgeneral_aless_znone_mipmap_flat_textured,
              FB_triangle_zon_cgeneral_aless_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_cgeneral_aless_znone_mipmap_smooth_untextured,
              FB_triangle_zon_cgeneral_aless_znone_mipmap_smooth_textured,
              FB_triangle_zon_cgeneral_aless_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zon_cgeneral_aless_zless_nearest_white_untextured,
              FB_triangle_zon_cgeneral_aless_zless_nearest_white_textured,
              FB_triangle_zon_cgeneral_aless_zless_nearest_white_perspective
            },
            {
              FB_triangle_zon_cgeneral_aless_zless_nearest_flat_untextured,
              FB_triangle_zon_cgeneral_aless_zless_nearest_flat_textured,
              FB_triangle_zon_cgeneral_aless_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zon_cgeneral_aless_zless_nearest_smooth_untextured,
              FB_triangle_zon_cgeneral_aless_zless_nearest_smooth_textured,
              FB_triangle_zon_cgeneral_aless_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_cgeneral_aless_zless_mipmap_white_untextured,
              FB_triangle_zon_cgeneral_aless_zless_mipmap_white_textured,
              FB_triangle_zon_cgeneral_aless_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zon_cgeneral_aless_zless_mipmap_flat_untextured,
              FB_triangle_zon_cgeneral_aless_zless_mipmap_flat_textured,
              FB_triangle_zon_cgeneral_aless_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_cgeneral_aless_zless_mipmap_smooth_untextured,
              FB_triangle_zon_cgeneral_aless_zless_mipmap_smooth_textured,
              FB_triangle_zon_cgeneral_aless_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zon_cgeneral_amore_znone_nearest_white_untextured,
              FB_triangle_zon_cgeneral_amore_znone_nearest_white_textured,
              FB_triangle_zon_cgeneral_amore_znone_nearest_white_perspective
            },
            {
              FB_triangle_zon_cgeneral_amore_znone_nearest_flat_untextured,
              FB_triangle_zon_cgeneral_amore_znone_nearest_flat_textured,
              FB_triangle_zon_cgeneral_amore_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zon_cgeneral_amore_znone_nearest_smooth_untextured,
              FB_triangle_zon_cgeneral_amore_znone_nearest_smooth_textured,
              FB_triangle_zon_cgeneral_amore_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_cgeneral_amore_znone_mipmap_white_untextured,
              FB_triangle_zon_cgeneral_amore_znone_mipmap_white_textured,
              FB_triangle_zon_cgeneral_amore_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zon_cgeneral_amore_znone_mipmap_flat_untextured,
              FB_triangle_zon_cgeneral_amore_znone_mipmap_flat_textured,
              FB_triangle_zon_cgeneral_amore_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_cgeneral_amore_znone_mipmap_smooth_untextured,
              FB_triangle_zon_cgeneral_amore_znone_mipmap_smooth_textured,
              FB_triangle_zon_cgeneral_amore_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zon_cgeneral_amore_zless_nearest_white_untextured,
              FB_triangle_zon_cgeneral_amore_zless_nearest_white_textured,
              FB_triangle_zon_cgeneral_amore_zless_nearest_white_perspective
            },
            {
              FB_triangle_zon_cgeneral_amore_zless_nearest_flat_untextured,
              FB_triangle_zon_cgeneral_amore_zless_nearest_flat_textured,
              FB_triangle_zon_cgeneral_amore_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zon_cgeneral_amore_zless_nearest_smooth_untextured,
              FB_triangle_zon_cgeneral_amore_zless_nearest_smooth_textured,
              FB_triangle_zon_cgeneral_amore_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_cgeneral_amore_zless_mipmap_white_untextured,
              FB_triangle_zon_cgeneral_amore_zless_mipmap_white_textured,
              FB_triangle_zon_cgeneral_amore_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zon_cgeneral_amore_zless_mipmap_flat_untextured,
              FB_triangle_zon_cgeneral_amore_zless_mipmap_flat_textured,
              FB_triangle_zon_cgeneral_amore_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_cgeneral_amore_zless_mipmap_smooth_untextured,
              FB_triangle_zon_cgeneral_amore_zless_mipmap_smooth_textured,
              FB_triangle_zon_cgeneral_amore_zless_mipmap_smooth_perspective
            }
          }
        }
      }
    },
    {
      {
        {
          {
            {
              FB_triangle_zon_coff_anone_znone_nearest_white_untextured,
              FB_triangle_zon_coff_anone_znone_nearest_white_textured,
              FB_triangle_zon_coff_anone_znone_nearest_white_perspective
            },
            {
              FB_triangle_zon_coff_anone_znone_nearest_flat_untextured,
              FB_triangle_zon_coff_anone_znone_nearest_flat_textured,
              FB_triangle_zon_coff_anone_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zon_coff_anone_znone_nearest_smooth_untextured,
              FB_triangle_zon_coff_anone_znone_nearest_smooth_textured,
              FB_triangle_zon_coff_anone_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_coff_anone_znone_mipmap_white_untextured,
              FB_triangle_zon_coff_anone_znone_mipmap_white_textured,
              FB_triangle_zon_coff_anone_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zon_coff_anone_znone_mipmap_flat_untextured,
              FB_triangle_zon_coff_anone_znone_mipmap_flat_textured,
              FB_triangle_zon_coff_anone_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_coff_anone_znone_mipmap_smooth_untextured,
              FB_triangle_zon_coff_anone_znone_mipmap_smooth_textured,
              FB_triangle_zon_coff_anone_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zon_coff_anone_zless_nearest_white_untextured,
              FB_triangle_zon_coff_anone_zless_nearest_white_textured,
              FB_triangle_zon_coff_anone_zless_nearest_white_perspective
            },
            {
              FB_triangle_zon_coff_anone_zless_nearest_flat_untextured,
              FB_triangle_zon_coff_anone_zless_nearest_flat_textured,
              FB_triangle_zon_coff_anone_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zon_coff_anone_zless_nearest_smooth_untextured,
              FB_triangle_zon_coff_anone_zless_nearest_smooth_textured,
              FB_triangle_zon_coff_anone_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_coff_anone_zless_mipmap_white_untextured,
              FB_triangle_zon_coff_anone_zless_mipmap_white_textured,
              FB_triangle_zon_coff_anone_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zon_coff_anone_zless_mipmap_flat_untextured,
              FB_triangle_zon_coff_anone_zless_mipmap_flat_textured,
              FB_triangle_zon_coff_anone_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_coff_anone_zless_mipmap_smooth_untextured,
              FB_triangle_zon_coff_anone_zless_mipmap_smooth_textured,
              FB_triangle_zon_coff_anone_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zon_coff_aless_znone_nearest_white_untextured,
              FB_triangle_zon_coff_aless_znone_nearest_white_textured,
              FB_triangle_zon_coff_aless_znone_nearest_white_perspective
            },
            {
              FB_triangle_zon_coff_aless_znone_nearest_flat_untextured,
              FB_triangle_zon_coff_aless_znone_nearest_flat_textured,
              FB_triangle_zon_coff_aless_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zon_coff_aless_znone_nearest_smooth_untextured,
              FB_triangle_zon_coff_aless_znone_nearest_smooth_textured,
              FB_triangle_zon_coff_aless_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_coff_aless_znone_mipmap_white_untextured,
              FB_triangle_zon_coff_aless_znone_mipmap_white_textured,
              FB_triangle_zon_coff_aless_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zon_coff_aless_znone_mipmap_flat_untextured,
              FB_triangle_zon_coff_aless_znone_mipmap_flat_textured,
              FB_triangle_zon_coff_aless_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_coff_aless_znone_mipmap_smooth_untextured,
              FB_triangle_zon_coff_aless_znone_mipmap_smooth_textured,
              FB_triangle_zon_coff_aless_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zon_coff_aless_zless_nearest_white_untextured,
              FB_triangle_zon_coff_aless_zless_nearest_white_textured,
              FB_triangle_zon_coff_aless_zless_nearest_white_perspective
            },
            {
              FB_triangle_zon_coff_aless_zless_nearest_flat_untextured,
              FB_triangle_zon_coff_aless_zless_nearest_flat_textured,
              FB_triangle_zon_coff_aless_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zon_coff_aless_zless_nearest_smooth_untextured,
              FB_triangle_zon_coff_aless_zless_nearest_smooth_textured,
              FB_triangle_zon_coff_aless_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_coff_aless_zless_mipmap_white_untextured,
              FB_triangle_zon_coff_aless_zless_mipmap_white_textured,
              FB_triangle_zon_coff_aless_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zon_coff_aless_zless_mipmap_flat_untextured,
              FB_triangle_zon_coff_aless_zless_mipmap_flat_textured,
              FB_triangle_zon_coff_aless_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_coff_aless_zless_mipmap_smooth_untextured,
              FB_triangle_zon_coff_aless_zless_mipmap_smooth_textured,
              FB_triangle_zon_coff_aless_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zon_coff_amore_znone_nearest_white_untextured,
              FB_triangle_zon_coff_amore_znone_nearest_white_textured,
              FB_triangle_zon_coff_amore_znone_nearest_white_perspective
            },
            {
              FB_triangle_zon_coff_amore_znone_nearest_flat_untextured,
              FB_triangle_zon_coff_amore_znone_nearest_flat_textured,
              FB_triangle_zon_coff_amore_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zon_coff_amore_znone_nearest_smooth_untextured,
              FB_triangle_zon_coff_amore_znone_nearest_smooth_textured,
              FB_triangle_zon_coff_amore_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_coff_amore_znone_mipmap_white_untextured,
              FB_triangle_zon_coff_amore_znone_mipmap_white_textured,
              FB_triangle_zon_coff_amore_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zon_coff_amore_znone_mipmap_flat_untextured,
              FB_triangle_zon_coff_amore_znone_mipmap_flat_textured,
              FB_triangle_zon_coff_amore_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_coff_amore_znone_mipmap_smooth_untextured,
              FB_triangle_zon_coff_amore_znone_mipmap_smooth_textured,
              FB_triangle_zon_coff_amore_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zon_coff_amore_zless_nearest_white_untextured,
              FB_triangle_zon_coff_amore_zless_nearest_white_textured,
              FB_triangle_zon_coff_amore_zless_nearest_white_perspective
            },
            {
              FB_triangle_zon_coff_amore_zless_nearest_flat_untextured,
              FB_triangle_zon_coff_amore_zless_nearest_flat_textured,
              FB_triangle_zon_coff_amore_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zon_coff_amore_zless_nearest_smooth_untextured,
              FB_triangle_zon_coff_amore_zless_nearest_smooth_textured,
              FB_triangle_zon_coff_amore_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_coff_amore_zless_mipmap_white_untextured,
              FB_triangle_zon_coff_amore_zless_mipmap_white_textured,
              FB_triangle_zon_coff_amore_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zon_coff_amore_zless_mipmap_flat_untextured,
              FB_triangle_zon_coff_amore_zless_mipmap_flat_textured,
              FB_triangle_zon_coff_amore_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_coff_amore_zless_mipmap_smooth_untextured,
              FB_triangle_zon_coff_amore_zless_mipmap_smooth_textured,
              FB_triangle_zon_coff_amore_zless_mipmap_smooth_perspective
            }
          }
        }
      }
    }
  },
  {
    {
      {
        {
          {
            {
              FB_triangle_zoff_cstore_anone_znone_nearest_white_untextured,
              FB_triangle_zoff_cstore_anone_znone_nearest_white_textured,
              FB_triangle_zoff_cstore_anone_znone_nearest_white_perspective
            },
            {
              FB_triangle_zoff_cstore_anone_znone_nearest_flat_untextured,
              FB_triangle_zoff_cstore_anone_znone_nearest_flat_textured,
              FB_triangle_zoff_cstore_anone_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_cstore_anone_znone_nearest_smooth_untextured,
              FB_triangle_zoff_cstore_anone_znone_nearest_smooth_textured,
              FB_triangle_zoff_cstore_anone_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_cstore_anone_znone_mipmap_white_untextured,
              FB_triangle_zoff_cstore_anone_znone_mipmap_white_textured,
              FB_triangle_zoff_cstore_anone_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_cstore_anone_znone_mipmap_flat_untextured,
              FB_triangle_zoff_cstore_anone_znone_mipmap_flat_textured,
              FB_triangle_zoff_cstore_anone_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_cstore_anone_znone_mipmap_smooth_untextured,
              FB_triangle_zoff_cstore_anone_znone_mipmap_smooth_textured,
              FB_triangle_zoff_cstore_anone_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zoff_cstore_anone_zless_nearest_white_untextured,
              FB_triangle_zoff_cstore_anone_zless_nearest_white_textured,
              FB_triangle_zoff_cstore_anone_zless_nearest_white_perspective
            },
            {
              FB_triangle_zoff_cstore_anone_zless_nearest_flat_untextured,
              FB_triangle_zoff_cstore_anone_zless_nearest_flat_textured,
              FB_triangle_zoff_cstore_anone_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_cstore_anone_zless_nearest_smooth_untextured,
              FB_triangle_zoff_cstore_anone_zless_nearest_smooth_textured,
              FB_triangle_zoff_cstore_anone_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_cstore_anone_zless_mipmap_white_untextured,
              FB_triangle_zoff_cstore_anone_zless_mipmap_white_textured,
              FB_triangle_zoff_cstore_anone_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_cstore_anone_zless_mipmap_flat_untextured,
              FB_triangle_zoff_cstore_anone_zless_mipmap_flat_textured,
              FB_triangle_zoff_cstore_anone_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_cstore_anone_zless_mipmap_smooth_untextured,
              FB_triangle_zoff_cstore_anone_zless_mipmap_smooth_textured,
              FB_triangle_zoff_cstore_anone_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zoff_cstore_aless_znone_nearest_white_untextured,
              FB_triangle_zoff_cstore_aless_znone_nearest_white_textured,
              FB_triangle_zoff_cstore_aless_znone_nearest_white_perspective
            },
            {
              FB_triangle_zoff_cstore_aless_znone_nearest_flat_untextured,
              FB_triangle_zoff_cstore_aless_znone_nearest_flat_textured,
              FB_triangle_zoff_cstore_aless_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_cstore_aless_znone_nearest_smooth_untextured,
              FB_triangle_zoff_cstore_aless_znone_nearest_smooth_textured,
              FB_triangle_zoff_cstore_aless_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_cstore_aless_znone_mipmap_white_untextured,
              FB_triangle_zoff_cstore_aless_znone_mipmap_white_textured,
              FB_triangle_zoff_cstore_aless_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_cstore_aless_znone_mipmap_flat_untextured,
              FB_triangle_zoff_cstore_aless_znone_mipmap_flat_textured,
              FB_triangle_zoff_cstore_aless_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_cstore_aless_znone_mipmap_smooth_untextured,
              FB_triangle_zoff_cstore_aless_znone_mipmap_smooth_textured,
              FB_triangle_zoff_cstore_aless_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zoff_cstore_aless_zless_nearest_white_untextured,
              FB_triangle_zoff_cstore_aless_zless_nearest_white_textured,
              FB_triangle_zoff_cstore_aless_zless_nearest_white_perspective
            },
            {
              FB_triangle_zoff_cstore_aless_zless_nearest_flat_untextured,
              FB_triangle_zoff_cstore_aless_zless_nearest_flat_textured,
              FB_triangle_zoff_cstore_aless_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_cstore_aless_zless_nearest_smooth_untextured,
              FB_triangle_zoff_cstore_aless_zless_nearest_smooth_textured,
              FB_triangle_zoff_cstore_aless_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_cstore_aless_zless_mipmap_white_untextured,
              FB_triangle_zoff_cstore_aless_zless_mipmap_white_textured,
              FB_triangle_zoff_cstore_aless_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_cstore_aless_zless_mipmap_flat_untextured,
              FB_triangle_zoff_cstore_aless_zless_mipmap_flat_textured,
              FB_triangle_zoff_cstore_aless_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_cstore_aless_zless_mipmap_smooth_untextured,
              FB_triangle_zoff_cstore_aless_zless_mipmap_smooth_textured,
              FB_triangle_zoff_cstore_aless_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zoff_cstore_amore_znone_nearest_white_untextured,
              FB_triangle_zoff_cstore_amore_znone_nearest_white_textured,
              FB_triangle_zoff_cstore_amore_znone_nearest_white_perspective
            },
            {
              FB_triangle_zoff_cstore_amore_znone_nearest_flat_untextured,
              FB_triangle_zoff_cstore_amore_znone_nearest_flat_textured,
              FB_triangle_zoff_cstore_amore_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_cstore_amore_znone_nearest_smooth_untextured,
              FB_triangle_zoff_cstore_amore_znone_nearest_smooth_textured,
              FB_triangle_zoff_cstore_amore_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_cstore_amore_znone_mipmap_white_untextured,
              FB_triangle_zoff_cstore_amore_znone_mipmap_white_textured,
              FB_triangle_zoff_cstore_amore_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_cstore_amore_znone_mipmap_flat_untextured,
              FB_triangle_zoff_cstore_amore_znone_mipmap_flat_textured,
              FB_triangle_zoff_cstore_amore_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_cstore_amore_znone_mipmap_smooth_untextured,
              FB_triangle_zoff_cstore_amore_znone_mipmap_smooth_textured,
              FB_triangle_zoff_cstore_amore_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zoff_cstore_amore_zless_nearest_white_untextured,
              FB_triangle_zoff_cstore_amore_zless_nearest_white_textured,
              FB_triangle_zoff_cstore_amore_zless_nearest_white_perspective
            },
            {
              FB_triangle_zoff_cstore_amore_zless_nearest_flat_untextured,
              FB_triangle_zoff_cstore_amore_zless_nearest_flat_textured,
              FB_triangle_zoff_cstore_amore_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_cstore_amore_zless_nearest_smooth_untextured,
              FB_triangle_zoff_cstore_amore_zless_nearest_smooth_textured,
              FB_triangle_zoff_cstore_amore_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_cstore_amore_zless_mipmap_white_untextured,
              FB_triangle_zoff_cstore_amore_zless_mipmap_white_textured,
              FB_triangle_zoff_cstore_amore_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_cstore_amore_zless_mipmap_flat_untextured,
              FB_triangle_zoff_cstore_amore_zless_mipmap_flat_textured,
              FB_triangle_zoff_cstore_amore_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_cstore_amore_zless_mipmap_smooth_untextured,
              FB_triangle_zoff_cstore_amore_zless_mipmap_smooth_textured,
              FB_triangle_zoff_cstore_amore_zless_mipmap_smooth_perspective
            }
          }
        }
      }
    },
    {
      {
        {
          {
            {
              FB_triangle_zoff_cblend_anone_znone_nearest_white_untextured,
              FB_triangle_zoff_cblend_anone_znone_nearest_white_textured,
              FB_triangle_zoff_cblend_anone_znone_nearest_white_perspective
            },
            {
              FB_triangle_zoff_cblend_anone_znone_nearest_flat_untextured,
              FB_triangle_zoff_cblend_anone_znone_nearest_flat_textured,
              FB_triangle_zoff_cblend_anone_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_cblend_anone_znone_nearest_smooth_untextured,
              FB_triangle_zoff_cblend_anone_znone_nearest_smooth_textured,
              FB_triangle_zoff_cblend_anone_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_cblend_anone_znone_mipmap_white_untextured,
              FB_triangle_zoff_cblend_anone_znone_mipmap_white_textured,
              FB_triangle_zoff_cblend_anone_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_cblend_anone_znone_mipmap_flat_untextured,
              FB_triangle_zoff_cblend_anone_znone_mipmap_flat_textured,
              FB_triangle_zoff_cblend_anone_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_cblend_anone_znone_mipmap_smooth_untextured,
              FB_triangle_zoff_cblend_anone_znone_mipmap_smooth_textured,
              FB_triangle_zoff_cblend_anone_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zoff_cblend_anone_zless_nearest_white_untextured,
              FB_triangle_zoff_cblend_anone_zless_nearest_white_textured,
              FB_triangle_zoff_cblend_anone_zless_nearest_white_perspective
            },
            {
              FB_triangle_zoff_cblend_anone_zless_nearest_flat_untextured,
              FB_triangle_zoff_cblend_anone_zless_nearest_flat_textured,
              FB_triangle_zoff_cblend_anone_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_cblend_anone_zless_nearest_smooth_untextured,
              FB_triangle_zoff_cblend_anone_zless_nearest_smooth_textured,
              FB_triangle_zoff_cblend_anone_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_cblend_anone_zless_mipmap_white_untextured,
              FB_triangle_zoff_cblend_anone_zless_mipmap_white_textured,
              FB_triangle_zoff_cblend_anone_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_cblend_anone_zless_mipmap_flat_untextured,
              FB_triangle_zoff_cblend_anone_zless_mipmap_flat_textured,
              FB_triangle_zoff_cblend_anone_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_cblend_anone_zless_mipmap_smooth_untextured,
              FB_triangle_zoff_cblend_anone_zless_mipmap_smooth_textured,
              FB_triangle_zoff_cblend_anone_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zoff_cblend_aless_znone_nearest_white_untextured,
              FB_triangle_zoff_cblend_aless_znone_nearest_white_textured,
              FB_triangle_zoff_cblend_aless_znone_nearest_white_perspective
            },
            {
              FB_triangle_zoff_cblend_aless_znone_nearest_flat_untextured,
              FB_triangle_zoff_cblend_aless_znone_nearest_flat_textured,
              FB_triangle_zoff_cblend_aless_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_cblend_aless_znone_nearest_smooth_untextured,
              FB_triangle_zoff_cblend_aless_znone_nearest_smooth_textured,
              FB_triangle_zoff_cblend_aless_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_cblend_aless_znone_mipmap_white_untextured,
              FB_triangle_zoff_cblend_aless_znone_mipmap_white_textured,
              FB_triangle_zoff_cblend_aless_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_cblend_aless_znone_mipmap_flat_untextured,
              FB_triangle_zoff_cblend_aless_znone_mipmap_flat_textured,
              FB_triangle_zoff_cblend_aless_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_cblend_aless_znone_mipmap_smooth_untextured,
              FB_triangle_zoff_cblend_aless_znone_mipmap_smooth_textured,
              FB_triangle_zoff_cblend_aless_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zoff_cblend_aless_zless_nearest_white_untextured,
              FB_triangle_zoff_cblend_aless_zless_nearest_white_textured,
              FB_triangle_zoff_cblend_aless_zless_nearest_white_perspective
            },
            {
              FB_triangle_zoff_cblend_aless_zless_nearest_flat_untextured,
              FB_triangle_zoff_cblend_aless_zless_nearest_flat_textured,
              FB_triangle_zoff_cblend_aless_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_cblend_aless_zless_nearest_smooth_untextured,
              FB_triangle_zoff_cblend_aless_zless_nearest_smooth_textured,
              FB_triangle_zoff_cblend_aless_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_cblend_aless_zless_mipmap_white_untextured,
              FB_triangle_zoff_cblend_aless_zless_mipmap_white_textured,
              FB_triangle_zoff_cblend_aless_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_cblend_aless_zless_mipmap_flat_untextured,
              FB_triangle_zoff_cblend_aless_zless_mipmap_flat_textured,
              FB_triangle_zoff_cblend_aless_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_cblend_aless_zless_mipmap_smooth_untextured,
              FB_triangle_zoff_cblend_aless_zless_mipmap_smooth_textured,
              FB_triangle_zoff_cblend_aless_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zoff_cblend_amore_znone_nearest_white_untextured,
              FB_triangle_zoff_cblend_amore_znone_nearest_white_textured,
              FB_triangle_zoff_cblend_amore_znone_nearest_white_perspective
            },
            {
              FB_triangle_zoff_cblend_amore_znone_nearest_flat_untextured,
              FB_triangle_zoff_cblend_amore_znone_nearest_flat_textured,
              FB_triangle_zoff_cblend_amore_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_cblend_amore_znone_nearest_smooth_untextured,
              FB_triangle_zoff_cblend_amore_znone_nearest_smooth_textured,
              FB_triangle_zoff_cblend_amore_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_cblend_amore_znone_mipmap_white_untextured,
              FB_triangle_zoff_cblend_amore_znone_mipmap_white_textured,
              FB_triangle_zoff_cblend_amore_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_cblend_amore_znone_mipmap_flat_untextured,
              FB_triangle_zoff_cblend_amore_znone_mipmap_flat_textured,
              FB_triangle_zoff_cblend_amore_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_cblend_amore_znone_mipmap_smooth_untextured,
              FB_triangle_zoff_cblend_amore_znone_mipmap_smooth_textured,
              FB_triangle_zoff_cblend_amore_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zoff_cblend_amore_zless_nearest_white_untextured,
              FB_triangle_zoff_cblend_amore_zless_nearest_white_textured,
              FB_triangle_zoff_cblend_amore_zless_nearest_white_perspective
            },
            {
              FB_triangle_zoff_cblend_amore_zless_nearest_flat_untextured,
              FB_triangle_zoff_cblend_amore_zless_nearest_flat_textured,
              FB_triangle_zoff_cblend_amore_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_cblend_amore_zless_nearest_smooth_untextured,
              FB_triangle_zoff_cblend_amore_zless_nearest_smooth_textured,
              FB_triangle_zoff_cblend_amore_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_cblend_amore_zless_mipmap_white_untextured,
              FB_triangle_zoff_cblend_amore_zless_mipmap_white_textured,
              FB_triangle_zoff_cblend_amore_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_cblend_amore_zless_mipmap_flat_untextured,
              FB_triangle_zoff_cblend_amore_zless_mipmap_flat_textured,
              FB_triangle_zoff_cblend_amore_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_cblend_amore_zless_mipmap_smooth_untextured,
              FB_triangle_zoff_cblend_amore_zless_mipmap_smooth_textured,
              FB_triangle_zoff_cblend_amore_zless_mipmap_smooth_perspective
            }
          }
        }
      }
    },
    {
      {
        {
          {
            {
              FB_triangle_zoff_cgeneral_anone_znone_nearest_white_untextured,
              FB_triangle_zoff_cgeneral_anone_znone_nearest_white_textured,
              FB_triangle_zoff_cgeneral_anone_znone_nearest_white_perspective
            },
            {
              FB_triangle_zoff_cgeneral_anone_znone_nearest_flat_untextured,
              FB_triangle_zoff_cgeneral_anone_znone_nearest_flat_textured,
              FB_triangle_zoff_cgeneral_anone_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_cgeneral_anone_znone_nearest_smooth_untextured,
              FB_triangle_zoff_cgeneral_anone_znone_nearest_smooth_textured,
              FB_triangle_zoff_cgeneral_anone_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_cgeneral_anone_znone_mipmap_white_untextured,
              FB_triangle_zoff_cgeneral_anone_znone_mipmap_white_textured,
              FB_triangle_zoff_cgeneral_anone_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_cgeneral_anone_znone_mipmap_flat_untextured,
              FB_triangle_zoff_cgeneral_anone_znone_mipmap_flat_textured,
              FB_triangle_zoff_cgeneral_anone_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_cgeneral_anone_znone_mipmap_smooth_untextured,
              FB_triangle_zoff_cgeneral_anone_znone_mipmap_smooth_textured,
              FB_triangle_zoff_cgeneral_anone_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zoff_cgeneral_anone_zless_nearest_white_untextured,
              FB_triangle_zoff_cgeneral_anone_zless_nearest_white_textured,
              FB_triangle_zoff_cgeneral_anone_zless_nearest_white_perspective
            },
            {
              FB_triangle_zoff_cgeneral_anone_zless_nearest_flat_untextured,
              FB_triangle_zoff_cgeneral_anone_zless_nearest_flat_textured,
              FB_triangle_zoff_cgeneral_anone_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_cgeneral_anone_zless_nearest_smooth_untextured,
              FB_triangle_zoff_cgeneral_anone_zless_nearest_smooth_textured,
              FB_triangle_zoff_cgeneral_anone_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_cgeneral_anone_zless_mipmap_white_untextured,
              FB_triangle_zoff_cgeneral_anone_zless_mipmap_white_textured,
              FB_triangle_zoff_cgeneral_anone_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_cgeneral_anone_zless_mipmap_flat_untextured,
              FB_triangle_zoff_cgeneral_anone_zless_mipmap_flat_textured,
              FB_triangle_zoff_cgeneral_anone_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_cgeneral_anone_zless_mipmap_smooth_untextured,
              FB_triangle_zoff_cgeneral_anone_zless_mipmap_smooth_textured,
              FB_triangle_zoff_cgeneral_anone_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zoff_cgeneral_aless_znone_nearest_white_untextured,
              FB_triangle_zoff_cgeneral_aless_znone_nearest_white_textured,
              FB_triangle_zoff_cgeneral_aless_znone_nearest_white_perspective
            },
            {
              FB_triangle_zoff_cgeneral_aless_znone_nearest_flat_untextured,
              FB_triangle_zoff_cgeneral_aless_znone_nearest_flat_textured,
              FB_triangle_zoff_cgeneral_aless_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_cgeneral_aless_znone_nearest_smooth_untextured,
              FB_triangle_zoff_cgeneral_aless_znone_nearest_smooth_textured,
              FB_triangle_zoff_cgeneral_aless_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_cgeneral_aless_znone_mipmap_white_untextured,
              FB_triangle_zoff_cgeneral_aless_znone_mipmap_white_textured,
              FB_triangle_zoff_cgeneral_aless_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_cgeneral_aless_znone_mipmap_flat_untextured,
              FB_triangle_zoff_cgeneral_aless_znone_mipmap_flat_textured,
              FB_triangle_zoff_cgeneral_aless_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_cgeneral_aless_znone_mipmap_smooth_untextured,
              FB_triangle_zoff_cgeneral_aless_znone_mipmap_smooth_textured,
              FB_triangle_zoff_cgeneral_aless_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zoff_cgeneral_aless_zless_nearest_white_untextured,
              FB_triangle_zoff_cgeneral_aless_zless_nearest_white_textured,
              FB_triangle_zoff_cgeneral_aless_zless_nearest_white_perspective
            },
            {
              FB_triangle_zoff_cgeneral_aless_zless_nearest_flat_untextured,
              FB_triangle_zoff_cgeneral_aless_zless_nearest_flat_textured,
              FB_triangle_zoff_cgeneral_aless_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_cgeneral_aless_zless_nearest_smooth_untextured,
              FB_triangle_zoff_cgeneral_aless_zless_nearest_smooth_textured,
              FB_triangle_zoff_cgeneral_aless_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_cgeneral_aless_zless_mipmap_white_untextured,
              FB_triangle_zoff_cgeneral_aless_zless_mipmap_white_textured,
              FB_triangle_zoff_cgeneral_aless_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_cgeneral_aless_zless_mipmap_flat_untextured,
              FB_triangle_zoff_cgeneral_aless_zless_mipmap_flat_textured,
              FB_triangle_zoff_cgeneral_aless_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_cgeneral_aless_zless_mipmap_smooth_untextured,
              FB_triangle_zoff_cgeneral_aless_zless_mipmap_smooth_textured,
              FB_triangle_zoff_cgeneral_aless_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zoff_cgeneral_amore_znone_nearest_white_untextured,
              FB_triangle_zoff_cgeneral_amore_znone_nearest_white_textured,
              FB_triangle_zoff_cgeneral_amore_znone_nearest_white_perspective
            },
            {
              FB_triangle_zoff_cgeneral_amore_znone_nearest_flat_untextured,
              FB_triangle_zoff_cgeneral_amore_znone_nearest_flat_textured,
              FB_triangle_zoff_cgeneral_amore_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_cgeneral_amore_znone_nearest_smooth_untextured,
              FB_triangle_zoff_cgeneral_amore_znone_nearest_smooth_textured,
              FB_triangle_zoff_cgeneral_amore_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_cgeneral_amore_znone_mipmap_white_untextured,
              FB_triangle_zoff_cgeneral_amore_znone_mipmap_white_textured,
              FB_triangle_zoff_cgeneral_amore_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_cgeneral_amore_znone_mipmap_flat_untextured,
              FB_triangle_zoff_cgeneral_amore_znone_mipmap_flat_textured,
              FB_triangle_zoff_cgeneral_amore_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_cgeneral_amore_znone_mipmap_smooth_untextured,
              FB_triangle_zoff_cgeneral_amore_znone_mipmap_smooth_textured,
              FB_triangle_zoff_cgeneral_amore_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zoff_cgeneral_amore_zless_nearest_white_untextured,
              FB_triangle_zoff_cgeneral_amore_zless_nearest_white_textured,
              FB_triangle_zoff_cgeneral_amore_zless_nearest_white_perspective
            },
            {
              FB_triangle_zoff_cgeneral_amore_zless_nearest_flat_untextured,
              FB_triangle_zoff_cgeneral_amore_zless_nearest_flat_textured,
              FB_triangle_zoff_cgeneral_amore_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_cgeneral_amore_zless_nearest_smooth_untextured,
              FB_triangle_zoff_cgeneral_amore_zless_nearest_smooth_textured,
              FB_triangle_zoff_cgeneral_amore_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_cgeneral_amore_zless_mipmap_white_untextured,
              FB_triangle_zoff_cgeneral_amore_zless_mipmap_white_textured,
              FB_triangle_zoff_cgeneral_amore_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_cgeneral_amore_zless_mipmap_flat_untextured,
              FB_triangle_zoff_cgeneral_amore_zless_mipmap_flat_textured,
              FB_triangle_zoff_cgeneral_amore_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_cgeneral_amore_zless_mipmap_smooth_untextured,
              FB_triangle_zoff_cgeneral_amore_zless_mipmap_smooth_textured,
              FB_triangle_zoff_cgeneral_amore_zless_mipmap_smooth_perspective
            }
          }
        }
      }
    },
    {
      {
        {
          {
            {
              FB_triangle_zoff_coff_anone_znone_nearest_white_untextured,
              FB_triangle_zoff_coff_anone_znone_nearest_white_textured,
              FB_triangle_zoff_coff_anone_znone_nearest_white_perspective
            },
            {
              FB_triangle_zoff_coff_anone_znone_nearest_flat_untextured,
              FB_triangle_zoff_coff_anone_znone_nearest_flat_textured,
              FB_triangle_zoff_coff_anone_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_coff_anone_znone_nearest_smooth_untextured,
              FB_triangle_zoff_coff_anone_znone_nearest_smooth_textured,
              FB_triangle_zoff_coff_anone_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_coff_anone_znone_mipmap_white_untextured,
              FB_triangle_zoff_coff_anone_znone_mipmap_white_textured,
              FB_triangle_zoff_coff_anone_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_coff_anone_znone_mipmap_flat_untextured,
              FB_triangle_zoff_coff_anone_znone_mipmap_flat_textured,
              FB_triangle_zoff_coff_anone_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_coff_anone_znone_mipmap_smooth_untextured,
              FB_triangle_zoff_coff_anone_znone_mipmap_smooth_textured,
              FB_triangle_zoff_coff_anone_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zoff_coff_anone_zless_nearest_white_untextured,
              FB_triangle_zoff_coff_anone_zless_nearest_white_textured,
              FB_triangle_zoff_coff_anone_zless_nearest_white_perspective
            },
            {
              FB_triangle_zoff_coff_anone_zless_nearest_flat_untextured,
              FB_triangle_zoff_coff_anone_zless_nearest_flat_textured,
              FB_triangle_zoff_coff_anone_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_coff_anone_zless_nearest_smooth_untextured,
              FB_triangle_zoff_coff_anone_zless_nearest_smooth_textured,
              FB_triangle_zoff_coff_anone_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_coff_anone_zless_mipmap_white_untextured,
              FB_triangle_zoff_coff_anone_zless_mipmap_white_textured,
              FB_triangle_zoff_coff_anone_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_coff_anone_zless_mipmap_flat_untextured,
              FB_triangle_zoff_coff_anone_zless_mipmap_flat_textured,
              FB_triangle_zoff_coff_anone_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_coff_anone_zless_mipmap_smooth_untextured,
              FB_triangle_zoff_coff_anone_zless_mipmap_smooth_textured,
              FB_triangle_zoff_coff_anone_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zoff_coff_aless_znone_nearest_white_untextured,
              FB_triangle_zoff_coff_aless_znone_nearest_white_textured,
              FB_triangle_zoff_coff_aless_znone_nearest_white_perspective
            },
            {
              FB_triangle_zoff_coff_aless_znone_nearest_flat_untextured,
              FB_triangle_zoff_coff_aless_znone_nearest_flat_textured,
              FB_triangle_zoff_coff_aless_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_coff_aless_znone_nearest_smooth_untextured,
              FB_triangle_zoff_coff_aless_znone_nearest_smooth_textured,
              FB_triangle_zoff_coff_aless_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_coff_aless_znone_mipmap_white_untextured,
              FB_triangle_zoff_coff_aless_znone_mipmap_white_textured,
              FB_triangle_zoff_coff_aless_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_coff_aless_znone_mipmap_flat_untextured,
              FB_triangle_zoff_coff_aless_znone_mipmap_flat_textured,
              FB_triangle_zoff_coff_aless_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_coff_aless_znone_mipmap_smooth_untextured,
              FB_triangle_zoff_coff_aless_znone_mipmap_smooth_textured,
              FB_triangle_zoff_coff_aless_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zoff_coff_aless_zless_nearest_white_untextured,
              FB_triangle_zoff_coff_aless_zless_nearest_white_textured,
              FB_triangle_zoff_coff_aless_zless_nearest_white_perspective
            },
            {
              FB_triangle_zoff_coff_aless_zless_nearest_flat_untextured,
              FB_triangle_zoff_coff_aless_zless_nearest_flat_textured,
              FB_triangle_zoff_coff_aless_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_coff_aless_zless_nearest_smooth_untextured,
              FB_triangle_zoff_coff_aless_zless_nearest_smooth_textured,
              FB_triangle_zoff_coff_aless_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_coff_aless_zless_mipmap_white_untextured,
              FB_triangle_zoff_coff_aless_zless_mipmap_white_textured,
              FB_triangle_zoff_coff_aless_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_coff_aless_zless_mipmap_flat_untextured,
              FB_triangle_zoff_coff_aless_zless_mipmap_flat_textured,
              FB_triangle_zoff_coff_aless_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_coff_aless_zless_mipmap_smooth_untextured,
              FB_triangle_zoff_coff_aless_zless_mipmap_smooth_textured,
              FB_triangle_zoff_coff_aless_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zoff_coff_amore_znone_nearest_white_untextured,
              FB_triangle_zoff_coff_amore_znone_nearest_white_textured,
              FB_triangle_zoff_coff_amore_znone_nearest_white_perspective
            },
            {
              FB_triangle_zoff_coff_amore_znone_nearest_flat_untextured,
              FB_triangle_zoff_coff_amore_znone_nearest_flat_textured,
              FB_triangle_zoff_coff_amore_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_coff_amore_znone_nearest_smooth_untextured,
              FB_triangle_zoff_coff_amore_znone_nearest_smooth_textured,
              FB_triangle_zoff_coff_amore_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_coff_amore_znone_mipmap_white_untextured,
              FB_triangle_zoff_coff_amore_znone_mipmap_white_textured,
              FB_triangle_zoff_coff_amore_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_coff_amore_znone_mipmap_flat_untextured,
              FB_triangle_zoff_coff_amore_znone_mipmap_flat_textured,
              FB_triangle_zoff_coff_amore_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_coff_amore_znone_mipmap_smooth_untextured,
              FB_triangle_zoff_coff_amore_znone_mipmap_smooth_textured,
              FB_triangle_zoff_coff_amore_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zoff_coff_amore_zless_nearest_white_untextured,
              FB_triangle_zoff_coff_amore_zless_nearest_white_textured,
              FB_triangle_zoff_coff_amore_zless_nearest_white_perspective
            },
            {
              FB_triangle_zoff_coff_amore_zless_nearest_flat_untextured,
              FB_triangle_zoff_coff_amore_zless_nearest_flat_textured,
              FB_triangle_zoff_coff_amore_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_coff_amore_zless_nearest_smooth_untextured,
              FB_triangle_zoff_coff_amore_zless_nearest_smooth_textured,
              FB_triangle_zoff_coff_amore_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_coff_amore_zless_mipmap_white_untextured,
              FB_triangle_zoff_coff_amore_zless_mipmap_white_textured,
              FB_triangle_zoff_coff_amore_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_coff_amore_zless_mipmap_flat_untextured,
              FB_triangle_zoff_coff_amore_zless_mipmap_flat_textured,
              FB_triangle_zoff_coff_amore_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_coff_amore_zless_mipmap_smooth_untextured,
              FB_triangle_zoff_coff_amore_zless_mipmap_smooth_textured,
              FB_triangle_zoff_coff_amore_zless_mipmap_smooth_perspective
            }
          }
        }
      }
    }
  }
};
