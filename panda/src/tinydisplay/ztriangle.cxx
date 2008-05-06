#include <stdlib.h>
#include <stdio.h>
#include "zbuffer.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb,a) 1
#define ZCMP(zpix, z) 1
#define FNAME(name) name ## _xx_zon_noblend_anone_znone
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb,a) 1
#define ZCMP(zpix, z) ((zpix) < (z))
#define FNAME(name) name ## _xx_zon_noblend_anone_zless
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb,a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define FNAME(name) name ## _xx_zon_noblend_aless_znone
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb,a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((zpix) < (z))
#define FNAME(name) name ## _xx_zon_noblend_aless_zless
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb,a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define FNAME(name) name ## _xx_zon_noblend_amore_znone
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb,a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((zpix) < (z))
#define FNAME(name) name ## _xx_zon_noblend_amore_zless
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb,a) 1
#define ZCMP(zpix, z) 1
#define FNAME(name) name ## _xx_zon_blend_anone_znone
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb,a) 1
#define ZCMP(zpix, z) ((zpix) < (z))
#define FNAME(name) name ## _xx_zon_blend_anone_zless
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb,a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define FNAME(name) name ## _xx_zon_blend_aless_znone
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb,a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((zpix) < (z))
#define FNAME(name) name ## _xx_zon_blend_aless_zless
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb,a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define FNAME(name) name ## _xx_zon_blend_amore_znone
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb,a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((zpix) < (z))
#define FNAME(name) name ## _xx_zon_blend_amore_zless
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb,a) 1
#define ZCMP(zpix, z) 1
#define FNAME(name) name ## _xx_zon_nocolor_anone_znone
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb,a) 1
#define ZCMP(zpix, z) ((zpix) < (z))
#define FNAME(name) name ## _xx_zon_nocolor_anone_zless
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb,a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define FNAME(name) name ## _xx_zon_nocolor_aless_znone
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb,a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((zpix) < (z))
#define FNAME(name) name ## _xx_zon_nocolor_aless_zless
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb,a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define FNAME(name) name ## _xx_zon_nocolor_amore_znone
#include "ztriangle_two.h"

#define STORE_Z(zpix, z) (zpix) = (z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb,a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((zpix) < (z))
#define FNAME(name) name ## _xx_zon_nocolor_amore_zless
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb,a) 1
#define ZCMP(zpix, z) 1
#define FNAME(name) name ## _xx_zoff_noblend_anone_znone
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb,a) 1
#define ZCMP(zpix, z) ((zpix) < (z))
#define FNAME(name) name ## _xx_zoff_noblend_anone_zless
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb,a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define FNAME(name) name ## _xx_zoff_noblend_aless_znone
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb,a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((zpix) < (z))
#define FNAME(name) name ## _xx_zoff_noblend_aless_zless
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb,a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define FNAME(name) name ## _xx_zoff_noblend_amore_znone
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = (rgb)
#define ACMP(zb,a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((zpix) < (z))
#define FNAME(name) name ## _xx_zoff_noblend_amore_zless
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb,a) 1
#define ZCMP(zpix, z) 1
#define FNAME(name) name ## _xx_zoff_blend_anone_znone
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb,a) 1
#define ZCMP(zpix, z) ((zpix) < (z))
#define FNAME(name) name ## _xx_zoff_blend_anone_zless
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb,a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define FNAME(name) name ## _xx_zoff_blend_aless_znone
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb,a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((zpix) < (z))
#define FNAME(name) name ## _xx_zoff_blend_aless_zless
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb,a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define FNAME(name) name ## _xx_zoff_blend_amore_znone
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a) (pix) = PIXEL_BLEND_RGB(pix, r, g, b, a)
#define ACMP(zb,a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((zpix) < (z))
#define FNAME(name) name ## _xx_zoff_blend_amore_zless
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb,a) 1
#define ZCMP(zpix, z) 1
#define FNAME(name) name ## _xx_zoff_nocolor_anone_znone
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb,a) 1
#define ZCMP(zpix, z) ((zpix) < (z))
#define FNAME(name) name ## _xx_zoff_nocolor_anone_zless
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb,a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define FNAME(name) name ## _xx_zoff_nocolor_aless_znone
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb,a) (((unsigned int)(a)) < (zb)->reference_alpha)
#define ZCMP(zpix, z) ((zpix) < (z))
#define FNAME(name) name ## _xx_zoff_nocolor_aless_zless
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb,a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) 1
#define FNAME(name) name ## _xx_zoff_nocolor_amore_znone
#include "ztriangle_two.h"

#define STORE_Z(zpix, z)
#define STORE_PIX(pix, rgb, r, g, b, a)
#define ACMP(zb,a) (((unsigned int)(a)) > (zb)->reference_alpha)
#define ZCMP(zpix, z) ((zpix) < (z))
#define FNAME(name) name ## _xx_zoff_nocolor_amore_zless
#include "ztriangle_two.h"
