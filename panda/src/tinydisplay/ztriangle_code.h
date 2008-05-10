/* This file is generated code--do not edit.  See ztriangle.py. */

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_noblend_anone_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_noblend_anone_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_noblend_anone_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_noblend_anone_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_noblend_aless_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_noblend_aless_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_noblend_aless_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_noblend_aless_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_noblend_amore_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_noblend_amore_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_noblend_amore_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_noblend_amore_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_blend_anone_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_blend_anone_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_blend_anone_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_blend_anone_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_blend_aless_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_blend_aless_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_blend_aless_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_blend_aless_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_blend_amore_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_blend_amore_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_blend_amore_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_blend_amore_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_nocolor_anone_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_nocolor_anone_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_nocolor_anone_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_nocolor_anone_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_nocolor_aless_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_nocolor_aless_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_nocolor_aless_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_nocolor_aless_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_nocolor_amore_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_nocolor_amore_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zon_nocolor_amore_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zon_nocolor_amore_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_noblend_anone_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_noblend_anone_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_noblend_anone_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_noblend_anone_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_noblend_aless_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_noblend_aless_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_noblend_aless_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_noblend_aless_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_noblend_amore_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_noblend_amore_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_noblend_amore_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_noblend_amore_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_blend_anone_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_blend_anone_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_blend_anone_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_blend_anone_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_blend_aless_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_blend_aless_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_blend_aless_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_blend_aless_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_blend_amore_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_blend_amore_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_blend_amore_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_blend_amore_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_nocolor_anone_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_nocolor_anone_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_nocolor_anone_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) 1
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_nocolor_anone_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_nocolor_aless_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_nocolor_aless_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_nocolor_aless_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_nocolor_aless_zless_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_nocolor_amore_znone_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_nocolor_amore_znone_mipmap_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST(texture_levels, s, t)
#define FNAME(name) FB_triangle_zoff_nocolor_amore_zless_nearest_ ## name
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb, a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((ZPOINT)(zpix) < (ZPOINT)(z))
#define CALC_MIPMAP_LEVEL DO_CALC_MIPMAP_LEVEL
#define INTERP_MIPMAP
#define ZB_LOOKUP_TEXTURE(texture_levels, s, t, level) ZB_LOOKUP_TEXTURE_NEAREST_MIPMAP(texture_levels, s, t, level)
#define FNAME(name) FB_triangle_zoff_nocolor_amore_zless_mipmap_ ## name
#include "ztriangle_two.h"

const ZB_fillTriangleFunc fill_tri_funcs[2][3][3][2][2][3][3] = {
  {
    {
      {
        {
          {
            {
              FB_triangle_zon_noblend_anone_znone_nearest_white_untextured,
              FB_triangle_zon_noblend_anone_znone_nearest_white_textured,
              FB_triangle_zon_noblend_anone_znone_nearest_white_perspective
            },
            {
              FB_triangle_zon_noblend_anone_znone_nearest_flat_untextured,
              FB_triangle_zon_noblend_anone_znone_nearest_flat_textured,
              FB_triangle_zon_noblend_anone_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zon_noblend_anone_znone_nearest_smooth_untextured,
              FB_triangle_zon_noblend_anone_znone_nearest_smooth_textured,
              FB_triangle_zon_noblend_anone_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_noblend_anone_znone_mipmap_white_untextured,
              FB_triangle_zon_noblend_anone_znone_mipmap_white_textured,
              FB_triangle_zon_noblend_anone_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zon_noblend_anone_znone_mipmap_flat_untextured,
              FB_triangle_zon_noblend_anone_znone_mipmap_flat_textured,
              FB_triangle_zon_noblend_anone_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_noblend_anone_znone_mipmap_smooth_untextured,
              FB_triangle_zon_noblend_anone_znone_mipmap_smooth_textured,
              FB_triangle_zon_noblend_anone_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zon_noblend_anone_zless_nearest_white_untextured,
              FB_triangle_zon_noblend_anone_zless_nearest_white_textured,
              FB_triangle_zon_noblend_anone_zless_nearest_white_perspective
            },
            {
              FB_triangle_zon_noblend_anone_zless_nearest_flat_untextured,
              FB_triangle_zon_noblend_anone_zless_nearest_flat_textured,
              FB_triangle_zon_noblend_anone_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zon_noblend_anone_zless_nearest_smooth_untextured,
              FB_triangle_zon_noblend_anone_zless_nearest_smooth_textured,
              FB_triangle_zon_noblend_anone_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_noblend_anone_zless_mipmap_white_untextured,
              FB_triangle_zon_noblend_anone_zless_mipmap_white_textured,
              FB_triangle_zon_noblend_anone_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zon_noblend_anone_zless_mipmap_flat_untextured,
              FB_triangle_zon_noblend_anone_zless_mipmap_flat_textured,
              FB_triangle_zon_noblend_anone_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_noblend_anone_zless_mipmap_smooth_untextured,
              FB_triangle_zon_noblend_anone_zless_mipmap_smooth_textured,
              FB_triangle_zon_noblend_anone_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zon_noblend_aless_znone_nearest_white_untextured,
              FB_triangle_zon_noblend_aless_znone_nearest_white_textured,
              FB_triangle_zon_noblend_aless_znone_nearest_white_perspective
            },
            {
              FB_triangle_zon_noblend_aless_znone_nearest_flat_untextured,
              FB_triangle_zon_noblend_aless_znone_nearest_flat_textured,
              FB_triangle_zon_noblend_aless_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zon_noblend_aless_znone_nearest_smooth_untextured,
              FB_triangle_zon_noblend_aless_znone_nearest_smooth_textured,
              FB_triangle_zon_noblend_aless_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_noblend_aless_znone_mipmap_white_untextured,
              FB_triangle_zon_noblend_aless_znone_mipmap_white_textured,
              FB_triangle_zon_noblend_aless_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zon_noblend_aless_znone_mipmap_flat_untextured,
              FB_triangle_zon_noblend_aless_znone_mipmap_flat_textured,
              FB_triangle_zon_noblend_aless_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_noblend_aless_znone_mipmap_smooth_untextured,
              FB_triangle_zon_noblend_aless_znone_mipmap_smooth_textured,
              FB_triangle_zon_noblend_aless_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zon_noblend_aless_zless_nearest_white_untextured,
              FB_triangle_zon_noblend_aless_zless_nearest_white_textured,
              FB_triangle_zon_noblend_aless_zless_nearest_white_perspective
            },
            {
              FB_triangle_zon_noblend_aless_zless_nearest_flat_untextured,
              FB_triangle_zon_noblend_aless_zless_nearest_flat_textured,
              FB_triangle_zon_noblend_aless_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zon_noblend_aless_zless_nearest_smooth_untextured,
              FB_triangle_zon_noblend_aless_zless_nearest_smooth_textured,
              FB_triangle_zon_noblend_aless_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_noblend_aless_zless_mipmap_white_untextured,
              FB_triangle_zon_noblend_aless_zless_mipmap_white_textured,
              FB_triangle_zon_noblend_aless_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zon_noblend_aless_zless_mipmap_flat_untextured,
              FB_triangle_zon_noblend_aless_zless_mipmap_flat_textured,
              FB_triangle_zon_noblend_aless_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_noblend_aless_zless_mipmap_smooth_untextured,
              FB_triangle_zon_noblend_aless_zless_mipmap_smooth_textured,
              FB_triangle_zon_noblend_aless_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zon_noblend_amore_znone_nearest_white_untextured,
              FB_triangle_zon_noblend_amore_znone_nearest_white_textured,
              FB_triangle_zon_noblend_amore_znone_nearest_white_perspective
            },
            {
              FB_triangle_zon_noblend_amore_znone_nearest_flat_untextured,
              FB_triangle_zon_noblend_amore_znone_nearest_flat_textured,
              FB_triangle_zon_noblend_amore_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zon_noblend_amore_znone_nearest_smooth_untextured,
              FB_triangle_zon_noblend_amore_znone_nearest_smooth_textured,
              FB_triangle_zon_noblend_amore_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_noblend_amore_znone_mipmap_white_untextured,
              FB_triangle_zon_noblend_amore_znone_mipmap_white_textured,
              FB_triangle_zon_noblend_amore_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zon_noblend_amore_znone_mipmap_flat_untextured,
              FB_triangle_zon_noblend_amore_znone_mipmap_flat_textured,
              FB_triangle_zon_noblend_amore_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_noblend_amore_znone_mipmap_smooth_untextured,
              FB_triangle_zon_noblend_amore_znone_mipmap_smooth_textured,
              FB_triangle_zon_noblend_amore_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zon_noblend_amore_zless_nearest_white_untextured,
              FB_triangle_zon_noblend_amore_zless_nearest_white_textured,
              FB_triangle_zon_noblend_amore_zless_nearest_white_perspective
            },
            {
              FB_triangle_zon_noblend_amore_zless_nearest_flat_untextured,
              FB_triangle_zon_noblend_amore_zless_nearest_flat_textured,
              FB_triangle_zon_noblend_amore_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zon_noblend_amore_zless_nearest_smooth_untextured,
              FB_triangle_zon_noblend_amore_zless_nearest_smooth_textured,
              FB_triangle_zon_noblend_amore_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_noblend_amore_zless_mipmap_white_untextured,
              FB_triangle_zon_noblend_amore_zless_mipmap_white_textured,
              FB_triangle_zon_noblend_amore_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zon_noblend_amore_zless_mipmap_flat_untextured,
              FB_triangle_zon_noblend_amore_zless_mipmap_flat_textured,
              FB_triangle_zon_noblend_amore_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_noblend_amore_zless_mipmap_smooth_untextured,
              FB_triangle_zon_noblend_amore_zless_mipmap_smooth_textured,
              FB_triangle_zon_noblend_amore_zless_mipmap_smooth_perspective
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
              FB_triangle_zon_blend_anone_znone_nearest_white_untextured,
              FB_triangle_zon_blend_anone_znone_nearest_white_textured,
              FB_triangle_zon_blend_anone_znone_nearest_white_perspective
            },
            {
              FB_triangle_zon_blend_anone_znone_nearest_flat_untextured,
              FB_triangle_zon_blend_anone_znone_nearest_flat_textured,
              FB_triangle_zon_blend_anone_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zon_blend_anone_znone_nearest_smooth_untextured,
              FB_triangle_zon_blend_anone_znone_nearest_smooth_textured,
              FB_triangle_zon_blend_anone_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_blend_anone_znone_mipmap_white_untextured,
              FB_triangle_zon_blend_anone_znone_mipmap_white_textured,
              FB_triangle_zon_blend_anone_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zon_blend_anone_znone_mipmap_flat_untextured,
              FB_triangle_zon_blend_anone_znone_mipmap_flat_textured,
              FB_triangle_zon_blend_anone_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_blend_anone_znone_mipmap_smooth_untextured,
              FB_triangle_zon_blend_anone_znone_mipmap_smooth_textured,
              FB_triangle_zon_blend_anone_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zon_blend_anone_zless_nearest_white_untextured,
              FB_triangle_zon_blend_anone_zless_nearest_white_textured,
              FB_triangle_zon_blend_anone_zless_nearest_white_perspective
            },
            {
              FB_triangle_zon_blend_anone_zless_nearest_flat_untextured,
              FB_triangle_zon_blend_anone_zless_nearest_flat_textured,
              FB_triangle_zon_blend_anone_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zon_blend_anone_zless_nearest_smooth_untextured,
              FB_triangle_zon_blend_anone_zless_nearest_smooth_textured,
              FB_triangle_zon_blend_anone_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_blend_anone_zless_mipmap_white_untextured,
              FB_triangle_zon_blend_anone_zless_mipmap_white_textured,
              FB_triangle_zon_blend_anone_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zon_blend_anone_zless_mipmap_flat_untextured,
              FB_triangle_zon_blend_anone_zless_mipmap_flat_textured,
              FB_triangle_zon_blend_anone_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_blend_anone_zless_mipmap_smooth_untextured,
              FB_triangle_zon_blend_anone_zless_mipmap_smooth_textured,
              FB_triangle_zon_blend_anone_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zon_blend_aless_znone_nearest_white_untextured,
              FB_triangle_zon_blend_aless_znone_nearest_white_textured,
              FB_triangle_zon_blend_aless_znone_nearest_white_perspective
            },
            {
              FB_triangle_zon_blend_aless_znone_nearest_flat_untextured,
              FB_triangle_zon_blend_aless_znone_nearest_flat_textured,
              FB_triangle_zon_blend_aless_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zon_blend_aless_znone_nearest_smooth_untextured,
              FB_triangle_zon_blend_aless_znone_nearest_smooth_textured,
              FB_triangle_zon_blend_aless_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_blend_aless_znone_mipmap_white_untextured,
              FB_triangle_zon_blend_aless_znone_mipmap_white_textured,
              FB_triangle_zon_blend_aless_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zon_blend_aless_znone_mipmap_flat_untextured,
              FB_triangle_zon_blend_aless_znone_mipmap_flat_textured,
              FB_triangle_zon_blend_aless_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_blend_aless_znone_mipmap_smooth_untextured,
              FB_triangle_zon_blend_aless_znone_mipmap_smooth_textured,
              FB_triangle_zon_blend_aless_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zon_blend_aless_zless_nearest_white_untextured,
              FB_triangle_zon_blend_aless_zless_nearest_white_textured,
              FB_triangle_zon_blend_aless_zless_nearest_white_perspective
            },
            {
              FB_triangle_zon_blend_aless_zless_nearest_flat_untextured,
              FB_triangle_zon_blend_aless_zless_nearest_flat_textured,
              FB_triangle_zon_blend_aless_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zon_blend_aless_zless_nearest_smooth_untextured,
              FB_triangle_zon_blend_aless_zless_nearest_smooth_textured,
              FB_triangle_zon_blend_aless_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_blend_aless_zless_mipmap_white_untextured,
              FB_triangle_zon_blend_aless_zless_mipmap_white_textured,
              FB_triangle_zon_blend_aless_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zon_blend_aless_zless_mipmap_flat_untextured,
              FB_triangle_zon_blend_aless_zless_mipmap_flat_textured,
              FB_triangle_zon_blend_aless_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_blend_aless_zless_mipmap_smooth_untextured,
              FB_triangle_zon_blend_aless_zless_mipmap_smooth_textured,
              FB_triangle_zon_blend_aless_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zon_blend_amore_znone_nearest_white_untextured,
              FB_triangle_zon_blend_amore_znone_nearest_white_textured,
              FB_triangle_zon_blend_amore_znone_nearest_white_perspective
            },
            {
              FB_triangle_zon_blend_amore_znone_nearest_flat_untextured,
              FB_triangle_zon_blend_amore_znone_nearest_flat_textured,
              FB_triangle_zon_blend_amore_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zon_blend_amore_znone_nearest_smooth_untextured,
              FB_triangle_zon_blend_amore_znone_nearest_smooth_textured,
              FB_triangle_zon_blend_amore_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_blend_amore_znone_mipmap_white_untextured,
              FB_triangle_zon_blend_amore_znone_mipmap_white_textured,
              FB_triangle_zon_blend_amore_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zon_blend_amore_znone_mipmap_flat_untextured,
              FB_triangle_zon_blend_amore_znone_mipmap_flat_textured,
              FB_triangle_zon_blend_amore_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_blend_amore_znone_mipmap_smooth_untextured,
              FB_triangle_zon_blend_amore_znone_mipmap_smooth_textured,
              FB_triangle_zon_blend_amore_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zon_blend_amore_zless_nearest_white_untextured,
              FB_triangle_zon_blend_amore_zless_nearest_white_textured,
              FB_triangle_zon_blend_amore_zless_nearest_white_perspective
            },
            {
              FB_triangle_zon_blend_amore_zless_nearest_flat_untextured,
              FB_triangle_zon_blend_amore_zless_nearest_flat_textured,
              FB_triangle_zon_blend_amore_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zon_blend_amore_zless_nearest_smooth_untextured,
              FB_triangle_zon_blend_amore_zless_nearest_smooth_textured,
              FB_triangle_zon_blend_amore_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_blend_amore_zless_mipmap_white_untextured,
              FB_triangle_zon_blend_amore_zless_mipmap_white_textured,
              FB_triangle_zon_blend_amore_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zon_blend_amore_zless_mipmap_flat_untextured,
              FB_triangle_zon_blend_amore_zless_mipmap_flat_textured,
              FB_triangle_zon_blend_amore_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_blend_amore_zless_mipmap_smooth_untextured,
              FB_triangle_zon_blend_amore_zless_mipmap_smooth_textured,
              FB_triangle_zon_blend_amore_zless_mipmap_smooth_perspective
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
              FB_triangle_zon_nocolor_anone_znone_nearest_white_untextured,
              FB_triangle_zon_nocolor_anone_znone_nearest_white_textured,
              FB_triangle_zon_nocolor_anone_znone_nearest_white_perspective
            },
            {
              FB_triangle_zon_nocolor_anone_znone_nearest_flat_untextured,
              FB_triangle_zon_nocolor_anone_znone_nearest_flat_textured,
              FB_triangle_zon_nocolor_anone_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zon_nocolor_anone_znone_nearest_smooth_untextured,
              FB_triangle_zon_nocolor_anone_znone_nearest_smooth_textured,
              FB_triangle_zon_nocolor_anone_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_nocolor_anone_znone_mipmap_white_untextured,
              FB_triangle_zon_nocolor_anone_znone_mipmap_white_textured,
              FB_triangle_zon_nocolor_anone_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zon_nocolor_anone_znone_mipmap_flat_untextured,
              FB_triangle_zon_nocolor_anone_znone_mipmap_flat_textured,
              FB_triangle_zon_nocolor_anone_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_nocolor_anone_znone_mipmap_smooth_untextured,
              FB_triangle_zon_nocolor_anone_znone_mipmap_smooth_textured,
              FB_triangle_zon_nocolor_anone_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zon_nocolor_anone_zless_nearest_white_untextured,
              FB_triangle_zon_nocolor_anone_zless_nearest_white_textured,
              FB_triangle_zon_nocolor_anone_zless_nearest_white_perspective
            },
            {
              FB_triangle_zon_nocolor_anone_zless_nearest_flat_untextured,
              FB_triangle_zon_nocolor_anone_zless_nearest_flat_textured,
              FB_triangle_zon_nocolor_anone_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zon_nocolor_anone_zless_nearest_smooth_untextured,
              FB_triangle_zon_nocolor_anone_zless_nearest_smooth_textured,
              FB_triangle_zon_nocolor_anone_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_nocolor_anone_zless_mipmap_white_untextured,
              FB_triangle_zon_nocolor_anone_zless_mipmap_white_textured,
              FB_triangle_zon_nocolor_anone_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zon_nocolor_anone_zless_mipmap_flat_untextured,
              FB_triangle_zon_nocolor_anone_zless_mipmap_flat_textured,
              FB_triangle_zon_nocolor_anone_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_nocolor_anone_zless_mipmap_smooth_untextured,
              FB_triangle_zon_nocolor_anone_zless_mipmap_smooth_textured,
              FB_triangle_zon_nocolor_anone_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zon_nocolor_aless_znone_nearest_white_untextured,
              FB_triangle_zon_nocolor_aless_znone_nearest_white_textured,
              FB_triangle_zon_nocolor_aless_znone_nearest_white_perspective
            },
            {
              FB_triangle_zon_nocolor_aless_znone_nearest_flat_untextured,
              FB_triangle_zon_nocolor_aless_znone_nearest_flat_textured,
              FB_triangle_zon_nocolor_aless_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zon_nocolor_aless_znone_nearest_smooth_untextured,
              FB_triangle_zon_nocolor_aless_znone_nearest_smooth_textured,
              FB_triangle_zon_nocolor_aless_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_nocolor_aless_znone_mipmap_white_untextured,
              FB_triangle_zon_nocolor_aless_znone_mipmap_white_textured,
              FB_triangle_zon_nocolor_aless_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zon_nocolor_aless_znone_mipmap_flat_untextured,
              FB_triangle_zon_nocolor_aless_znone_mipmap_flat_textured,
              FB_triangle_zon_nocolor_aless_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_nocolor_aless_znone_mipmap_smooth_untextured,
              FB_triangle_zon_nocolor_aless_znone_mipmap_smooth_textured,
              FB_triangle_zon_nocolor_aless_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zon_nocolor_aless_zless_nearest_white_untextured,
              FB_triangle_zon_nocolor_aless_zless_nearest_white_textured,
              FB_triangle_zon_nocolor_aless_zless_nearest_white_perspective
            },
            {
              FB_triangle_zon_nocolor_aless_zless_nearest_flat_untextured,
              FB_triangle_zon_nocolor_aless_zless_nearest_flat_textured,
              FB_triangle_zon_nocolor_aless_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zon_nocolor_aless_zless_nearest_smooth_untextured,
              FB_triangle_zon_nocolor_aless_zless_nearest_smooth_textured,
              FB_triangle_zon_nocolor_aless_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_nocolor_aless_zless_mipmap_white_untextured,
              FB_triangle_zon_nocolor_aless_zless_mipmap_white_textured,
              FB_triangle_zon_nocolor_aless_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zon_nocolor_aless_zless_mipmap_flat_untextured,
              FB_triangle_zon_nocolor_aless_zless_mipmap_flat_textured,
              FB_triangle_zon_nocolor_aless_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_nocolor_aless_zless_mipmap_smooth_untextured,
              FB_triangle_zon_nocolor_aless_zless_mipmap_smooth_textured,
              FB_triangle_zon_nocolor_aless_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zon_nocolor_amore_znone_nearest_white_untextured,
              FB_triangle_zon_nocolor_amore_znone_nearest_white_textured,
              FB_triangle_zon_nocolor_amore_znone_nearest_white_perspective
            },
            {
              FB_triangle_zon_nocolor_amore_znone_nearest_flat_untextured,
              FB_triangle_zon_nocolor_amore_znone_nearest_flat_textured,
              FB_triangle_zon_nocolor_amore_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zon_nocolor_amore_znone_nearest_smooth_untextured,
              FB_triangle_zon_nocolor_amore_znone_nearest_smooth_textured,
              FB_triangle_zon_nocolor_amore_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_nocolor_amore_znone_mipmap_white_untextured,
              FB_triangle_zon_nocolor_amore_znone_mipmap_white_textured,
              FB_triangle_zon_nocolor_amore_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zon_nocolor_amore_znone_mipmap_flat_untextured,
              FB_triangle_zon_nocolor_amore_znone_mipmap_flat_textured,
              FB_triangle_zon_nocolor_amore_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_nocolor_amore_znone_mipmap_smooth_untextured,
              FB_triangle_zon_nocolor_amore_znone_mipmap_smooth_textured,
              FB_triangle_zon_nocolor_amore_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zon_nocolor_amore_zless_nearest_white_untextured,
              FB_triangle_zon_nocolor_amore_zless_nearest_white_textured,
              FB_triangle_zon_nocolor_amore_zless_nearest_white_perspective
            },
            {
              FB_triangle_zon_nocolor_amore_zless_nearest_flat_untextured,
              FB_triangle_zon_nocolor_amore_zless_nearest_flat_textured,
              FB_triangle_zon_nocolor_amore_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zon_nocolor_amore_zless_nearest_smooth_untextured,
              FB_triangle_zon_nocolor_amore_zless_nearest_smooth_textured,
              FB_triangle_zon_nocolor_amore_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zon_nocolor_amore_zless_mipmap_white_untextured,
              FB_triangle_zon_nocolor_amore_zless_mipmap_white_textured,
              FB_triangle_zon_nocolor_amore_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zon_nocolor_amore_zless_mipmap_flat_untextured,
              FB_triangle_zon_nocolor_amore_zless_mipmap_flat_textured,
              FB_triangle_zon_nocolor_amore_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zon_nocolor_amore_zless_mipmap_smooth_untextured,
              FB_triangle_zon_nocolor_amore_zless_mipmap_smooth_textured,
              FB_triangle_zon_nocolor_amore_zless_mipmap_smooth_perspective
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
              FB_triangle_zoff_noblend_anone_znone_nearest_white_untextured,
              FB_triangle_zoff_noblend_anone_znone_nearest_white_textured,
              FB_triangle_zoff_noblend_anone_znone_nearest_white_perspective
            },
            {
              FB_triangle_zoff_noblend_anone_znone_nearest_flat_untextured,
              FB_triangle_zoff_noblend_anone_znone_nearest_flat_textured,
              FB_triangle_zoff_noblend_anone_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_noblend_anone_znone_nearest_smooth_untextured,
              FB_triangle_zoff_noblend_anone_znone_nearest_smooth_textured,
              FB_triangle_zoff_noblend_anone_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_noblend_anone_znone_mipmap_white_untextured,
              FB_triangle_zoff_noblend_anone_znone_mipmap_white_textured,
              FB_triangle_zoff_noblend_anone_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_noblend_anone_znone_mipmap_flat_untextured,
              FB_triangle_zoff_noblend_anone_znone_mipmap_flat_textured,
              FB_triangle_zoff_noblend_anone_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_noblend_anone_znone_mipmap_smooth_untextured,
              FB_triangle_zoff_noblend_anone_znone_mipmap_smooth_textured,
              FB_triangle_zoff_noblend_anone_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zoff_noblend_anone_zless_nearest_white_untextured,
              FB_triangle_zoff_noblend_anone_zless_nearest_white_textured,
              FB_triangle_zoff_noblend_anone_zless_nearest_white_perspective
            },
            {
              FB_triangle_zoff_noblend_anone_zless_nearest_flat_untextured,
              FB_triangle_zoff_noblend_anone_zless_nearest_flat_textured,
              FB_triangle_zoff_noblend_anone_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_noblend_anone_zless_nearest_smooth_untextured,
              FB_triangle_zoff_noblend_anone_zless_nearest_smooth_textured,
              FB_triangle_zoff_noblend_anone_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_noblend_anone_zless_mipmap_white_untextured,
              FB_triangle_zoff_noblend_anone_zless_mipmap_white_textured,
              FB_triangle_zoff_noblend_anone_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_noblend_anone_zless_mipmap_flat_untextured,
              FB_triangle_zoff_noblend_anone_zless_mipmap_flat_textured,
              FB_triangle_zoff_noblend_anone_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_noblend_anone_zless_mipmap_smooth_untextured,
              FB_triangle_zoff_noblend_anone_zless_mipmap_smooth_textured,
              FB_triangle_zoff_noblend_anone_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zoff_noblend_aless_znone_nearest_white_untextured,
              FB_triangle_zoff_noblend_aless_znone_nearest_white_textured,
              FB_triangle_zoff_noblend_aless_znone_nearest_white_perspective
            },
            {
              FB_triangle_zoff_noblend_aless_znone_nearest_flat_untextured,
              FB_triangle_zoff_noblend_aless_znone_nearest_flat_textured,
              FB_triangle_zoff_noblend_aless_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_noblend_aless_znone_nearest_smooth_untextured,
              FB_triangle_zoff_noblend_aless_znone_nearest_smooth_textured,
              FB_triangle_zoff_noblend_aless_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_noblend_aless_znone_mipmap_white_untextured,
              FB_triangle_zoff_noblend_aless_znone_mipmap_white_textured,
              FB_triangle_zoff_noblend_aless_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_noblend_aless_znone_mipmap_flat_untextured,
              FB_triangle_zoff_noblend_aless_znone_mipmap_flat_textured,
              FB_triangle_zoff_noblend_aless_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_noblend_aless_znone_mipmap_smooth_untextured,
              FB_triangle_zoff_noblend_aless_znone_mipmap_smooth_textured,
              FB_triangle_zoff_noblend_aless_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zoff_noblend_aless_zless_nearest_white_untextured,
              FB_triangle_zoff_noblend_aless_zless_nearest_white_textured,
              FB_triangle_zoff_noblend_aless_zless_nearest_white_perspective
            },
            {
              FB_triangle_zoff_noblend_aless_zless_nearest_flat_untextured,
              FB_triangle_zoff_noblend_aless_zless_nearest_flat_textured,
              FB_triangle_zoff_noblend_aless_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_noblend_aless_zless_nearest_smooth_untextured,
              FB_triangle_zoff_noblend_aless_zless_nearest_smooth_textured,
              FB_triangle_zoff_noblend_aless_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_noblend_aless_zless_mipmap_white_untextured,
              FB_triangle_zoff_noblend_aless_zless_mipmap_white_textured,
              FB_triangle_zoff_noblend_aless_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_noblend_aless_zless_mipmap_flat_untextured,
              FB_triangle_zoff_noblend_aless_zless_mipmap_flat_textured,
              FB_triangle_zoff_noblend_aless_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_noblend_aless_zless_mipmap_smooth_untextured,
              FB_triangle_zoff_noblend_aless_zless_mipmap_smooth_textured,
              FB_triangle_zoff_noblend_aless_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zoff_noblend_amore_znone_nearest_white_untextured,
              FB_triangle_zoff_noblend_amore_znone_nearest_white_textured,
              FB_triangle_zoff_noblend_amore_znone_nearest_white_perspective
            },
            {
              FB_triangle_zoff_noblend_amore_znone_nearest_flat_untextured,
              FB_triangle_zoff_noblend_amore_znone_nearest_flat_textured,
              FB_triangle_zoff_noblend_amore_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_noblend_amore_znone_nearest_smooth_untextured,
              FB_triangle_zoff_noblend_amore_znone_nearest_smooth_textured,
              FB_triangle_zoff_noblend_amore_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_noblend_amore_znone_mipmap_white_untextured,
              FB_triangle_zoff_noblend_amore_znone_mipmap_white_textured,
              FB_triangle_zoff_noblend_amore_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_noblend_amore_znone_mipmap_flat_untextured,
              FB_triangle_zoff_noblend_amore_znone_mipmap_flat_textured,
              FB_triangle_zoff_noblend_amore_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_noblend_amore_znone_mipmap_smooth_untextured,
              FB_triangle_zoff_noblend_amore_znone_mipmap_smooth_textured,
              FB_triangle_zoff_noblend_amore_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zoff_noblend_amore_zless_nearest_white_untextured,
              FB_triangle_zoff_noblend_amore_zless_nearest_white_textured,
              FB_triangle_zoff_noblend_amore_zless_nearest_white_perspective
            },
            {
              FB_triangle_zoff_noblend_amore_zless_nearest_flat_untextured,
              FB_triangle_zoff_noblend_amore_zless_nearest_flat_textured,
              FB_triangle_zoff_noblend_amore_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_noblend_amore_zless_nearest_smooth_untextured,
              FB_triangle_zoff_noblend_amore_zless_nearest_smooth_textured,
              FB_triangle_zoff_noblend_amore_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_noblend_amore_zless_mipmap_white_untextured,
              FB_triangle_zoff_noblend_amore_zless_mipmap_white_textured,
              FB_triangle_zoff_noblend_amore_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_noblend_amore_zless_mipmap_flat_untextured,
              FB_triangle_zoff_noblend_amore_zless_mipmap_flat_textured,
              FB_triangle_zoff_noblend_amore_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_noblend_amore_zless_mipmap_smooth_untextured,
              FB_triangle_zoff_noblend_amore_zless_mipmap_smooth_textured,
              FB_triangle_zoff_noblend_amore_zless_mipmap_smooth_perspective
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
              FB_triangle_zoff_blend_anone_znone_nearest_white_untextured,
              FB_triangle_zoff_blend_anone_znone_nearest_white_textured,
              FB_triangle_zoff_blend_anone_znone_nearest_white_perspective
            },
            {
              FB_triangle_zoff_blend_anone_znone_nearest_flat_untextured,
              FB_triangle_zoff_blend_anone_znone_nearest_flat_textured,
              FB_triangle_zoff_blend_anone_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_blend_anone_znone_nearest_smooth_untextured,
              FB_triangle_zoff_blend_anone_znone_nearest_smooth_textured,
              FB_triangle_zoff_blend_anone_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_blend_anone_znone_mipmap_white_untextured,
              FB_triangle_zoff_blend_anone_znone_mipmap_white_textured,
              FB_triangle_zoff_blend_anone_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_blend_anone_znone_mipmap_flat_untextured,
              FB_triangle_zoff_blend_anone_znone_mipmap_flat_textured,
              FB_triangle_zoff_blend_anone_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_blend_anone_znone_mipmap_smooth_untextured,
              FB_triangle_zoff_blend_anone_znone_mipmap_smooth_textured,
              FB_triangle_zoff_blend_anone_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zoff_blend_anone_zless_nearest_white_untextured,
              FB_triangle_zoff_blend_anone_zless_nearest_white_textured,
              FB_triangle_zoff_blend_anone_zless_nearest_white_perspective
            },
            {
              FB_triangle_zoff_blend_anone_zless_nearest_flat_untextured,
              FB_triangle_zoff_blend_anone_zless_nearest_flat_textured,
              FB_triangle_zoff_blend_anone_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_blend_anone_zless_nearest_smooth_untextured,
              FB_triangle_zoff_blend_anone_zless_nearest_smooth_textured,
              FB_triangle_zoff_blend_anone_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_blend_anone_zless_mipmap_white_untextured,
              FB_triangle_zoff_blend_anone_zless_mipmap_white_textured,
              FB_triangle_zoff_blend_anone_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_blend_anone_zless_mipmap_flat_untextured,
              FB_triangle_zoff_blend_anone_zless_mipmap_flat_textured,
              FB_triangle_zoff_blend_anone_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_blend_anone_zless_mipmap_smooth_untextured,
              FB_triangle_zoff_blend_anone_zless_mipmap_smooth_textured,
              FB_triangle_zoff_blend_anone_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zoff_blend_aless_znone_nearest_white_untextured,
              FB_triangle_zoff_blend_aless_znone_nearest_white_textured,
              FB_triangle_zoff_blend_aless_znone_nearest_white_perspective
            },
            {
              FB_triangle_zoff_blend_aless_znone_nearest_flat_untextured,
              FB_triangle_zoff_blend_aless_znone_nearest_flat_textured,
              FB_triangle_zoff_blend_aless_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_blend_aless_znone_nearest_smooth_untextured,
              FB_triangle_zoff_blend_aless_znone_nearest_smooth_textured,
              FB_triangle_zoff_blend_aless_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_blend_aless_znone_mipmap_white_untextured,
              FB_triangle_zoff_blend_aless_znone_mipmap_white_textured,
              FB_triangle_zoff_blend_aless_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_blend_aless_znone_mipmap_flat_untextured,
              FB_triangle_zoff_blend_aless_znone_mipmap_flat_textured,
              FB_triangle_zoff_blend_aless_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_blend_aless_znone_mipmap_smooth_untextured,
              FB_triangle_zoff_blend_aless_znone_mipmap_smooth_textured,
              FB_triangle_zoff_blend_aless_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zoff_blend_aless_zless_nearest_white_untextured,
              FB_triangle_zoff_blend_aless_zless_nearest_white_textured,
              FB_triangle_zoff_blend_aless_zless_nearest_white_perspective
            },
            {
              FB_triangle_zoff_blend_aless_zless_nearest_flat_untextured,
              FB_triangle_zoff_blend_aless_zless_nearest_flat_textured,
              FB_triangle_zoff_blend_aless_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_blend_aless_zless_nearest_smooth_untextured,
              FB_triangle_zoff_blend_aless_zless_nearest_smooth_textured,
              FB_triangle_zoff_blend_aless_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_blend_aless_zless_mipmap_white_untextured,
              FB_triangle_zoff_blend_aless_zless_mipmap_white_textured,
              FB_triangle_zoff_blend_aless_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_blend_aless_zless_mipmap_flat_untextured,
              FB_triangle_zoff_blend_aless_zless_mipmap_flat_textured,
              FB_triangle_zoff_blend_aless_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_blend_aless_zless_mipmap_smooth_untextured,
              FB_triangle_zoff_blend_aless_zless_mipmap_smooth_textured,
              FB_triangle_zoff_blend_aless_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zoff_blend_amore_znone_nearest_white_untextured,
              FB_triangle_zoff_blend_amore_znone_nearest_white_textured,
              FB_triangle_zoff_blend_amore_znone_nearest_white_perspective
            },
            {
              FB_triangle_zoff_blend_amore_znone_nearest_flat_untextured,
              FB_triangle_zoff_blend_amore_znone_nearest_flat_textured,
              FB_triangle_zoff_blend_amore_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_blend_amore_znone_nearest_smooth_untextured,
              FB_triangle_zoff_blend_amore_znone_nearest_smooth_textured,
              FB_triangle_zoff_blend_amore_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_blend_amore_znone_mipmap_white_untextured,
              FB_triangle_zoff_blend_amore_znone_mipmap_white_textured,
              FB_triangle_zoff_blend_amore_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_blend_amore_znone_mipmap_flat_untextured,
              FB_triangle_zoff_blend_amore_znone_mipmap_flat_textured,
              FB_triangle_zoff_blend_amore_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_blend_amore_znone_mipmap_smooth_untextured,
              FB_triangle_zoff_blend_amore_znone_mipmap_smooth_textured,
              FB_triangle_zoff_blend_amore_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zoff_blend_amore_zless_nearest_white_untextured,
              FB_triangle_zoff_blend_amore_zless_nearest_white_textured,
              FB_triangle_zoff_blend_amore_zless_nearest_white_perspective
            },
            {
              FB_triangle_zoff_blend_amore_zless_nearest_flat_untextured,
              FB_triangle_zoff_blend_amore_zless_nearest_flat_textured,
              FB_triangle_zoff_blend_amore_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_blend_amore_zless_nearest_smooth_untextured,
              FB_triangle_zoff_blend_amore_zless_nearest_smooth_textured,
              FB_triangle_zoff_blend_amore_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_blend_amore_zless_mipmap_white_untextured,
              FB_triangle_zoff_blend_amore_zless_mipmap_white_textured,
              FB_triangle_zoff_blend_amore_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_blend_amore_zless_mipmap_flat_untextured,
              FB_triangle_zoff_blend_amore_zless_mipmap_flat_textured,
              FB_triangle_zoff_blend_amore_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_blend_amore_zless_mipmap_smooth_untextured,
              FB_triangle_zoff_blend_amore_zless_mipmap_smooth_textured,
              FB_triangle_zoff_blend_amore_zless_mipmap_smooth_perspective
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
              FB_triangle_zoff_nocolor_anone_znone_nearest_white_untextured,
              FB_triangle_zoff_nocolor_anone_znone_nearest_white_textured,
              FB_triangle_zoff_nocolor_anone_znone_nearest_white_perspective
            },
            {
              FB_triangle_zoff_nocolor_anone_znone_nearest_flat_untextured,
              FB_triangle_zoff_nocolor_anone_znone_nearest_flat_textured,
              FB_triangle_zoff_nocolor_anone_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_nocolor_anone_znone_nearest_smooth_untextured,
              FB_triangle_zoff_nocolor_anone_znone_nearest_smooth_textured,
              FB_triangle_zoff_nocolor_anone_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_nocolor_anone_znone_mipmap_white_untextured,
              FB_triangle_zoff_nocolor_anone_znone_mipmap_white_textured,
              FB_triangle_zoff_nocolor_anone_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_nocolor_anone_znone_mipmap_flat_untextured,
              FB_triangle_zoff_nocolor_anone_znone_mipmap_flat_textured,
              FB_triangle_zoff_nocolor_anone_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_nocolor_anone_znone_mipmap_smooth_untextured,
              FB_triangle_zoff_nocolor_anone_znone_mipmap_smooth_textured,
              FB_triangle_zoff_nocolor_anone_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zoff_nocolor_anone_zless_nearest_white_untextured,
              FB_triangle_zoff_nocolor_anone_zless_nearest_white_textured,
              FB_triangle_zoff_nocolor_anone_zless_nearest_white_perspective
            },
            {
              FB_triangle_zoff_nocolor_anone_zless_nearest_flat_untextured,
              FB_triangle_zoff_nocolor_anone_zless_nearest_flat_textured,
              FB_triangle_zoff_nocolor_anone_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_nocolor_anone_zless_nearest_smooth_untextured,
              FB_triangle_zoff_nocolor_anone_zless_nearest_smooth_textured,
              FB_triangle_zoff_nocolor_anone_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_nocolor_anone_zless_mipmap_white_untextured,
              FB_triangle_zoff_nocolor_anone_zless_mipmap_white_textured,
              FB_triangle_zoff_nocolor_anone_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_nocolor_anone_zless_mipmap_flat_untextured,
              FB_triangle_zoff_nocolor_anone_zless_mipmap_flat_textured,
              FB_triangle_zoff_nocolor_anone_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_nocolor_anone_zless_mipmap_smooth_untextured,
              FB_triangle_zoff_nocolor_anone_zless_mipmap_smooth_textured,
              FB_triangle_zoff_nocolor_anone_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zoff_nocolor_aless_znone_nearest_white_untextured,
              FB_triangle_zoff_nocolor_aless_znone_nearest_white_textured,
              FB_triangle_zoff_nocolor_aless_znone_nearest_white_perspective
            },
            {
              FB_triangle_zoff_nocolor_aless_znone_nearest_flat_untextured,
              FB_triangle_zoff_nocolor_aless_znone_nearest_flat_textured,
              FB_triangle_zoff_nocolor_aless_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_nocolor_aless_znone_nearest_smooth_untextured,
              FB_triangle_zoff_nocolor_aless_znone_nearest_smooth_textured,
              FB_triangle_zoff_nocolor_aless_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_nocolor_aless_znone_mipmap_white_untextured,
              FB_triangle_zoff_nocolor_aless_znone_mipmap_white_textured,
              FB_triangle_zoff_nocolor_aless_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_nocolor_aless_znone_mipmap_flat_untextured,
              FB_triangle_zoff_nocolor_aless_znone_mipmap_flat_textured,
              FB_triangle_zoff_nocolor_aless_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_nocolor_aless_znone_mipmap_smooth_untextured,
              FB_triangle_zoff_nocolor_aless_znone_mipmap_smooth_textured,
              FB_triangle_zoff_nocolor_aless_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zoff_nocolor_aless_zless_nearest_white_untextured,
              FB_triangle_zoff_nocolor_aless_zless_nearest_white_textured,
              FB_triangle_zoff_nocolor_aless_zless_nearest_white_perspective
            },
            {
              FB_triangle_zoff_nocolor_aless_zless_nearest_flat_untextured,
              FB_triangle_zoff_nocolor_aless_zless_nearest_flat_textured,
              FB_triangle_zoff_nocolor_aless_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_nocolor_aless_zless_nearest_smooth_untextured,
              FB_triangle_zoff_nocolor_aless_zless_nearest_smooth_textured,
              FB_triangle_zoff_nocolor_aless_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_nocolor_aless_zless_mipmap_white_untextured,
              FB_triangle_zoff_nocolor_aless_zless_mipmap_white_textured,
              FB_triangle_zoff_nocolor_aless_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_nocolor_aless_zless_mipmap_flat_untextured,
              FB_triangle_zoff_nocolor_aless_zless_mipmap_flat_textured,
              FB_triangle_zoff_nocolor_aless_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_nocolor_aless_zless_mipmap_smooth_untextured,
              FB_triangle_zoff_nocolor_aless_zless_mipmap_smooth_textured,
              FB_triangle_zoff_nocolor_aless_zless_mipmap_smooth_perspective
            }
          }
        }
      },
      {
        {
          {
            {
              FB_triangle_zoff_nocolor_amore_znone_nearest_white_untextured,
              FB_triangle_zoff_nocolor_amore_znone_nearest_white_textured,
              FB_triangle_zoff_nocolor_amore_znone_nearest_white_perspective
            },
            {
              FB_triangle_zoff_nocolor_amore_znone_nearest_flat_untextured,
              FB_triangle_zoff_nocolor_amore_znone_nearest_flat_textured,
              FB_triangle_zoff_nocolor_amore_znone_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_nocolor_amore_znone_nearest_smooth_untextured,
              FB_triangle_zoff_nocolor_amore_znone_nearest_smooth_textured,
              FB_triangle_zoff_nocolor_amore_znone_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_nocolor_amore_znone_mipmap_white_untextured,
              FB_triangle_zoff_nocolor_amore_znone_mipmap_white_textured,
              FB_triangle_zoff_nocolor_amore_znone_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_nocolor_amore_znone_mipmap_flat_untextured,
              FB_triangle_zoff_nocolor_amore_znone_mipmap_flat_textured,
              FB_triangle_zoff_nocolor_amore_znone_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_nocolor_amore_znone_mipmap_smooth_untextured,
              FB_triangle_zoff_nocolor_amore_znone_mipmap_smooth_textured,
              FB_triangle_zoff_nocolor_amore_znone_mipmap_smooth_perspective
            }
          }
        },
        {
          {
            {
              FB_triangle_zoff_nocolor_amore_zless_nearest_white_untextured,
              FB_triangle_zoff_nocolor_amore_zless_nearest_white_textured,
              FB_triangle_zoff_nocolor_amore_zless_nearest_white_perspective
            },
            {
              FB_triangle_zoff_nocolor_amore_zless_nearest_flat_untextured,
              FB_triangle_zoff_nocolor_amore_zless_nearest_flat_textured,
              FB_triangle_zoff_nocolor_amore_zless_nearest_flat_perspective
            },
            {
              FB_triangle_zoff_nocolor_amore_zless_nearest_smooth_untextured,
              FB_triangle_zoff_nocolor_amore_zless_nearest_smooth_textured,
              FB_triangle_zoff_nocolor_amore_zless_nearest_smooth_perspective
            }
          },
          {
            {
              FB_triangle_zoff_nocolor_amore_zless_mipmap_white_untextured,
              FB_triangle_zoff_nocolor_amore_zless_mipmap_white_textured,
              FB_triangle_zoff_nocolor_amore_zless_mipmap_white_perspective
            },
            {
              FB_triangle_zoff_nocolor_amore_zless_mipmap_flat_untextured,
              FB_triangle_zoff_nocolor_amore_zless_mipmap_flat_textured,
              FB_triangle_zoff_nocolor_amore_zless_mipmap_flat_perspective
            },
            {
              FB_triangle_zoff_nocolor_amore_zless_mipmap_smooth_untextured,
              FB_triangle_zoff_nocolor_amore_zless_mipmap_smooth_textured,
              FB_triangle_zoff_nocolor_amore_zless_mipmap_smooth_perspective
            }
          }
        }
      }
    }
  }
};
