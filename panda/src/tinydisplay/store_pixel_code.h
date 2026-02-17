/* This file is generated code--do not edit.  See store_pixel.py. */

#define FNAME(name) store_pixel_add_add_000
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_add_r00
#define FNAME_S(name) store_pixel_add_add_r00_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_R 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_add_0g0
#define FNAME_S(name) store_pixel_add_add_0g0_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_G 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_add_rg0
#define FNAME_S(name) store_pixel_add_add_rg0_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_R 1
#define HAVE_G 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_add_00b
#define FNAME_S(name) store_pixel_add_add_00b_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_add_r0b
#define FNAME_S(name) store_pixel_add_add_r0b_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_R 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_add_0gb
#define FNAME_S(name) store_pixel_add_add_0gb_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_G 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_add_rgb
#define FNAME_S(name) store_pixel_add_add_rgb_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_R 1
#define HAVE_G 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_min_000
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_min_r00
#define FNAME_S(name) store_pixel_add_min_r00_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_min_0g0
#define FNAME_S(name) store_pixel_add_min_0g0_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_G 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_min_rg0
#define FNAME_S(name) store_pixel_add_min_rg0_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_G 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_min_00b
#define FNAME_S(name) store_pixel_add_min_00b_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_min_r0b
#define FNAME_S(name) store_pixel_add_min_r0b_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_min_0gb
#define FNAME_S(name) store_pixel_add_min_0gb_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_G 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_min_rgb
#define FNAME_S(name) store_pixel_add_min_rgb_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_G 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_max_000
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_max_r00
#define FNAME_S(name) store_pixel_add_max_r00_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_max_0g0
#define FNAME_S(name) store_pixel_add_max_0g0_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_G 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_max_rg0
#define FNAME_S(name) store_pixel_add_max_rg0_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_G 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_max_00b
#define FNAME_S(name) store_pixel_add_max_00b_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_max_r0b
#define FNAME_S(name) store_pixel_add_max_r0b_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_max_0gb
#define FNAME_S(name) store_pixel_add_max_0gb_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_G 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_max_rgb
#define FNAME_S(name) store_pixel_add_max_rgb_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_G 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_off_000
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_off_r00
#define FNAME_S(name) store_pixel_add_off_r00_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#define HAVE_R 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_off_0g0
#define FNAME_S(name) store_pixel_add_off_0g0_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#define HAVE_G 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_off_rg0
#define FNAME_S(name) store_pixel_add_off_rg0_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#define HAVE_R 1
#define HAVE_G 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_off_00b
#define FNAME_S(name) store_pixel_add_off_00b_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#define HAVE_B 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_off_r0b
#define FNAME_S(name) store_pixel_add_off_r0b_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#define HAVE_R 1
#define HAVE_B 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_off_0gb
#define FNAME_S(name) store_pixel_add_off_0gb_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#define HAVE_G 1
#define HAVE_B 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_add_off_rgb
#define FNAME_S(name) store_pixel_add_off_rgb_s
#define MODE_RGB(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#define HAVE_R 1
#define HAVE_G 1
#define HAVE_B 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_add_000
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_add_r00
#define FNAME_S(name) store_pixel_min_add_r00_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_R 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_add_0g0
#define FNAME_S(name) store_pixel_min_add_0g0_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_G 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_add_rg0
#define FNAME_S(name) store_pixel_min_add_rg0_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_R 1
#define HAVE_G 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_add_00b
#define FNAME_S(name) store_pixel_min_add_00b_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_add_r0b
#define FNAME_S(name) store_pixel_min_add_r0b_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_R 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_add_0gb
#define FNAME_S(name) store_pixel_min_add_0gb_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_G 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_add_rgb
#define FNAME_S(name) store_pixel_min_add_rgb_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_R 1
#define HAVE_G 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_min_000
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_min_r00
#define FNAME_S(name) store_pixel_min_min_r00_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_min_0g0
#define FNAME_S(name) store_pixel_min_min_0g0_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_G 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_min_rg0
#define FNAME_S(name) store_pixel_min_min_rg0_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_G 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_min_00b
#define FNAME_S(name) store_pixel_min_min_00b_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_min_r0b
#define FNAME_S(name) store_pixel_min_min_r0b_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_min_0gb
#define FNAME_S(name) store_pixel_min_min_0gb_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_G 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_min_rgb
#define FNAME_S(name) store_pixel_min_min_rgb_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_G 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_max_000
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_max_r00
#define FNAME_S(name) store_pixel_min_max_r00_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_max_0g0
#define FNAME_S(name) store_pixel_min_max_0g0_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_G 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_max_rg0
#define FNAME_S(name) store_pixel_min_max_rg0_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_G 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_max_00b
#define FNAME_S(name) store_pixel_min_max_00b_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_max_r0b
#define FNAME_S(name) store_pixel_min_max_r0b_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_max_0gb
#define FNAME_S(name) store_pixel_min_max_0gb_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_G 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_max_rgb
#define FNAME_S(name) store_pixel_min_max_rgb_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_G 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_off_000
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_off_r00
#define FNAME_S(name) store_pixel_min_off_r00_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#define HAVE_R 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_off_0g0
#define FNAME_S(name) store_pixel_min_off_0g0_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#define HAVE_G 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_off_rg0
#define FNAME_S(name) store_pixel_min_off_rg0_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#define HAVE_R 1
#define HAVE_G 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_off_00b
#define FNAME_S(name) store_pixel_min_off_00b_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#define HAVE_B 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_off_r0b
#define FNAME_S(name) store_pixel_min_off_r0b_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#define HAVE_R 1
#define HAVE_B 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_off_0gb
#define FNAME_S(name) store_pixel_min_off_0gb_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#define HAVE_G 1
#define HAVE_B 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_min_off_rgb
#define FNAME_S(name) store_pixel_min_off_rgb_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#define HAVE_R 1
#define HAVE_G 1
#define HAVE_B 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_add_000
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_add_r00
#define FNAME_S(name) store_pixel_max_add_r00_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_R 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_add_0g0
#define FNAME_S(name) store_pixel_max_add_0g0_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_G 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_add_rg0
#define FNAME_S(name) store_pixel_max_add_rg0_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_R 1
#define HAVE_G 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_add_00b
#define FNAME_S(name) store_pixel_max_add_00b_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_add_r0b
#define FNAME_S(name) store_pixel_max_add_r0b_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_R 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_add_0gb
#define FNAME_S(name) store_pixel_max_add_0gb_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_G 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_add_rgb
#define FNAME_S(name) store_pixel_max_add_rgb_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) STORE_PIX_CLAMP(((unsigned int)c * c_opa >> 16) + ((unsigned int)fc * c_opb >> 16))
#define HAVE_R 1
#define HAVE_G 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_min_000
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_min_r00
#define FNAME_S(name) store_pixel_max_min_r00_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_min_0g0
#define FNAME_S(name) store_pixel_max_min_0g0_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_G 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_min_rg0
#define FNAME_S(name) store_pixel_max_min_rg0_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_G 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_min_00b
#define FNAME_S(name) store_pixel_max_min_00b_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_min_r0b
#define FNAME_S(name) store_pixel_max_min_r0b_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_min_0gb
#define FNAME_S(name) store_pixel_max_min_0gb_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_G 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_min_rgb
#define FNAME_S(name) store_pixel_max_min_rgb_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::min((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_G 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_max_000
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_max_r00
#define FNAME_S(name) store_pixel_max_max_r00_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_max_0g0
#define FNAME_S(name) store_pixel_max_max_0g0_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_G 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_max_rg0
#define FNAME_S(name) store_pixel_max_max_rg0_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_G 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_max_00b
#define FNAME_S(name) store_pixel_max_max_00b_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_max_r0b
#define FNAME_S(name) store_pixel_max_max_r0b_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_max_0gb
#define FNAME_S(name) store_pixel_max_max_0gb_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_G 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_max_rgb
#define FNAME_S(name) store_pixel_max_max_rgb_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define HAVE_R 1
#define HAVE_G 1
#define HAVE_B 1
#define HAVE_A 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_off_000
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_off_r00
#define FNAME_S(name) store_pixel_max_off_r00_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#define HAVE_R 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_off_0g0
#define FNAME_S(name) store_pixel_max_off_0g0_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#define HAVE_G 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_off_rg0
#define FNAME_S(name) store_pixel_max_off_rg0_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#define HAVE_R 1
#define HAVE_G 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_off_00b
#define FNAME_S(name) store_pixel_max_off_00b_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#define HAVE_B 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_off_r0b
#define FNAME_S(name) store_pixel_max_off_r0b_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#define HAVE_R 1
#define HAVE_B 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_off_0gb
#define FNAME_S(name) store_pixel_max_off_0gb_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#define HAVE_G 1
#define HAVE_B 1
#include "store_pixel.h"

#define FNAME(name) store_pixel_max_off_rgb
#define FNAME_S(name) store_pixel_max_off_rgb_s
#define MODE_RGB(c, c_opa, fc, c_opb) std::max((unsigned int)c, (unsigned int)fc)
#define MODE_ALPHA(c, c_opa, fc, c_opb) (unsigned int)fc
#define HAVE_R 1
#define HAVE_G 1
#define HAVE_B 1
#include "store_pixel.h"

const ZB_storePixelFunc store_pixel_funcs[3][4][8][2] = {
  {
    {
      {store_pixel_add_add_000, store_pixel_add_add_000},
      {store_pixel_add_add_r00, store_pixel_add_add_r00_s},
      {store_pixel_add_add_0g0, store_pixel_add_add_0g0_s},
      {store_pixel_add_add_rg0, store_pixel_add_add_rg0_s},
      {store_pixel_add_add_00b, store_pixel_add_add_00b_s},
      {store_pixel_add_add_r0b, store_pixel_add_add_r0b_s},
      {store_pixel_add_add_0gb, store_pixel_add_add_0gb_s},
      {store_pixel_add_add_rgb, store_pixel_add_add_rgb_s},
    },
    {
      {store_pixel_add_min_000, store_pixel_add_min_000},
      {store_pixel_add_min_r00, store_pixel_add_min_r00_s},
      {store_pixel_add_min_0g0, store_pixel_add_min_0g0_s},
      {store_pixel_add_min_rg0, store_pixel_add_min_rg0_s},
      {store_pixel_add_min_00b, store_pixel_add_min_00b_s},
      {store_pixel_add_min_r0b, store_pixel_add_min_r0b_s},
      {store_pixel_add_min_0gb, store_pixel_add_min_0gb_s},
      {store_pixel_add_min_rgb, store_pixel_add_min_rgb_s},
    },
    {
      {store_pixel_add_max_000, store_pixel_add_max_000},
      {store_pixel_add_max_r00, store_pixel_add_max_r00_s},
      {store_pixel_add_max_0g0, store_pixel_add_max_0g0_s},
      {store_pixel_add_max_rg0, store_pixel_add_max_rg0_s},
      {store_pixel_add_max_00b, store_pixel_add_max_00b_s},
      {store_pixel_add_max_r0b, store_pixel_add_max_r0b_s},
      {store_pixel_add_max_0gb, store_pixel_add_max_0gb_s},
      {store_pixel_add_max_rgb, store_pixel_add_max_rgb_s},
    },
    {
      {store_pixel_add_off_000, store_pixel_add_off_000},
      {store_pixel_add_off_r00, store_pixel_add_off_r00_s},
      {store_pixel_add_off_0g0, store_pixel_add_off_0g0_s},
      {store_pixel_add_off_rg0, store_pixel_add_off_rg0_s},
      {store_pixel_add_off_00b, store_pixel_add_off_00b_s},
      {store_pixel_add_off_r0b, store_pixel_add_off_r0b_s},
      {store_pixel_add_off_0gb, store_pixel_add_off_0gb_s},
      {store_pixel_add_off_rgb, store_pixel_add_off_rgb_s},
    },
  },
  {
    {
      {store_pixel_min_add_000, store_pixel_min_add_000},
      {store_pixel_min_add_r00, store_pixel_min_add_r00_s},
      {store_pixel_min_add_0g0, store_pixel_min_add_0g0_s},
      {store_pixel_min_add_rg0, store_pixel_min_add_rg0_s},
      {store_pixel_min_add_00b, store_pixel_min_add_00b_s},
      {store_pixel_min_add_r0b, store_pixel_min_add_r0b_s},
      {store_pixel_min_add_0gb, store_pixel_min_add_0gb_s},
      {store_pixel_min_add_rgb, store_pixel_min_add_rgb_s},
    },
    {
      {store_pixel_min_min_000, store_pixel_min_min_000},
      {store_pixel_min_min_r00, store_pixel_min_min_r00_s},
      {store_pixel_min_min_0g0, store_pixel_min_min_0g0_s},
      {store_pixel_min_min_rg0, store_pixel_min_min_rg0_s},
      {store_pixel_min_min_00b, store_pixel_min_min_00b_s},
      {store_pixel_min_min_r0b, store_pixel_min_min_r0b_s},
      {store_pixel_min_min_0gb, store_pixel_min_min_0gb_s},
      {store_pixel_min_min_rgb, store_pixel_min_min_rgb_s},
    },
    {
      {store_pixel_min_max_000, store_pixel_min_max_000},
      {store_pixel_min_max_r00, store_pixel_min_max_r00_s},
      {store_pixel_min_max_0g0, store_pixel_min_max_0g0_s},
      {store_pixel_min_max_rg0, store_pixel_min_max_rg0_s},
      {store_pixel_min_max_00b, store_pixel_min_max_00b_s},
      {store_pixel_min_max_r0b, store_pixel_min_max_r0b_s},
      {store_pixel_min_max_0gb, store_pixel_min_max_0gb_s},
      {store_pixel_min_max_rgb, store_pixel_min_max_rgb_s},
    },
    {
      {store_pixel_min_off_000, store_pixel_min_off_000},
      {store_pixel_min_off_r00, store_pixel_min_off_r00_s},
      {store_pixel_min_off_0g0, store_pixel_min_off_0g0_s},
      {store_pixel_min_off_rg0, store_pixel_min_off_rg0_s},
      {store_pixel_min_off_00b, store_pixel_min_off_00b_s},
      {store_pixel_min_off_r0b, store_pixel_min_off_r0b_s},
      {store_pixel_min_off_0gb, store_pixel_min_off_0gb_s},
      {store_pixel_min_off_rgb, store_pixel_min_off_rgb_s},
    },
  },
  {
    {
      {store_pixel_max_add_000, store_pixel_max_add_000},
      {store_pixel_max_add_r00, store_pixel_max_add_r00_s},
      {store_pixel_max_add_0g0, store_pixel_max_add_0g0_s},
      {store_pixel_max_add_rg0, store_pixel_max_add_rg0_s},
      {store_pixel_max_add_00b, store_pixel_max_add_00b_s},
      {store_pixel_max_add_r0b, store_pixel_max_add_r0b_s},
      {store_pixel_max_add_0gb, store_pixel_max_add_0gb_s},
      {store_pixel_max_add_rgb, store_pixel_max_add_rgb_s},
    },
    {
      {store_pixel_max_min_000, store_pixel_max_min_000},
      {store_pixel_max_min_r00, store_pixel_max_min_r00_s},
      {store_pixel_max_min_0g0, store_pixel_max_min_0g0_s},
      {store_pixel_max_min_rg0, store_pixel_max_min_rg0_s},
      {store_pixel_max_min_00b, store_pixel_max_min_00b_s},
      {store_pixel_max_min_r0b, store_pixel_max_min_r0b_s},
      {store_pixel_max_min_0gb, store_pixel_max_min_0gb_s},
      {store_pixel_max_min_rgb, store_pixel_max_min_rgb_s},
    },
    {
      {store_pixel_max_max_000, store_pixel_max_max_000},
      {store_pixel_max_max_r00, store_pixel_max_max_r00_s},
      {store_pixel_max_max_0g0, store_pixel_max_max_0g0_s},
      {store_pixel_max_max_rg0, store_pixel_max_max_rg0_s},
      {store_pixel_max_max_00b, store_pixel_max_max_00b_s},
      {store_pixel_max_max_r0b, store_pixel_max_max_r0b_s},
      {store_pixel_max_max_0gb, store_pixel_max_max_0gb_s},
      {store_pixel_max_max_rgb, store_pixel_max_max_rgb_s},
    },
    {
      {store_pixel_max_off_000, store_pixel_max_off_000},
      {store_pixel_max_off_r00, store_pixel_max_off_r00_s},
      {store_pixel_max_off_0g0, store_pixel_max_off_0g0_s},
      {store_pixel_max_off_rg0, store_pixel_max_off_rg0_s},
      {store_pixel_max_off_00b, store_pixel_max_off_00b_s},
      {store_pixel_max_off_r0b, store_pixel_max_off_r0b_s},
      {store_pixel_max_off_0gb, store_pixel_max_off_0gb_s},
      {store_pixel_max_off_rgb, store_pixel_max_off_rgb_s},
    },
  },
};

