/* This file is generated code--do not edit.  See store_pixel.py. */

#define FNAME(name) store_pixel_zero_zero
#define OP_A(f, i) 0
#define OP_B(f, i) 0
#include "store_pixel.h"

#define FNAME(name) store_pixel_zero_one
#define OP_A(f, i) 0
#define OP_B(f, i) 0x10000
#include "store_pixel.h"

#define FNAME(name) store_pixel_zero_icolor
#define OP_A(f, i) 0
#define OP_B(f, i) i
#include "store_pixel.h"

#define FNAME(name) store_pixel_zero_micolor
#define OP_A(f, i) 0
#define OP_B(f, i) (0xffff - i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_zero_fcolor
#define OP_A(f, i) 0
#define OP_B(f, i) f
#include "store_pixel.h"

#define FNAME(name) store_pixel_zero_mfcolor
#define OP_A(f, i) 0
#define OP_B(f, i) (0xffff - f)
#include "store_pixel.h"

#define FNAME(name) store_pixel_zero_ialpha
#define OP_A(f, i) 0
#define OP_B(f, i) a
#include "store_pixel.h"

#define FNAME(name) store_pixel_zero_mialpha
#define OP_A(f, i) 0
#define OP_B(f, i) (0xffff - a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_zero_falpha
#define OP_A(f, i) 0
#define OP_B(f, i) fa
#include "store_pixel.h"

#define FNAME(name) store_pixel_zero_mfalpha
#define OP_A(f, i) 0
#define OP_B(f, i) (0xffff - fa)
#include "store_pixel.h"

#define FNAME(name) store_pixel_zero_ccolor
#define OP_A(f, i) 0
#define OP_B(f, i) zb->blend_ ## i
#include "store_pixel.h"

#define FNAME(name) store_pixel_zero_mccolor
#define OP_A(f, i) 0
#define OP_B(f, i) (0xffff - zb->blend_ ## i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_zero_calpha
#define OP_A(f, i) 0
#define OP_B(f, i) zb->blend_a
#include "store_pixel.h"

#define FNAME(name) store_pixel_zero_mcalpha
#define OP_A(f, i) 0
#define OP_B(f, i) (0xffff - zb->blend_a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_one_zero
#define OP_A(f, i) 0x10000
#define OP_B(f, i) 0
#include "store_pixel.h"

#define FNAME(name) store_pixel_one_one
#define OP_A(f, i) 0x10000
#define OP_B(f, i) 0x10000
#include "store_pixel.h"

#define FNAME(name) store_pixel_one_icolor
#define OP_A(f, i) 0x10000
#define OP_B(f, i) i
#include "store_pixel.h"

#define FNAME(name) store_pixel_one_micolor
#define OP_A(f, i) 0x10000
#define OP_B(f, i) (0xffff - i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_one_fcolor
#define OP_A(f, i) 0x10000
#define OP_B(f, i) f
#include "store_pixel.h"

#define FNAME(name) store_pixel_one_mfcolor
#define OP_A(f, i) 0x10000
#define OP_B(f, i) (0xffff - f)
#include "store_pixel.h"

#define FNAME(name) store_pixel_one_ialpha
#define OP_A(f, i) 0x10000
#define OP_B(f, i) a
#include "store_pixel.h"

#define FNAME(name) store_pixel_one_mialpha
#define OP_A(f, i) 0x10000
#define OP_B(f, i) (0xffff - a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_one_falpha
#define OP_A(f, i) 0x10000
#define OP_B(f, i) fa
#include "store_pixel.h"

#define FNAME(name) store_pixel_one_mfalpha
#define OP_A(f, i) 0x10000
#define OP_B(f, i) (0xffff - fa)
#include "store_pixel.h"

#define FNAME(name) store_pixel_one_ccolor
#define OP_A(f, i) 0x10000
#define OP_B(f, i) zb->blend_ ## i
#include "store_pixel.h"

#define FNAME(name) store_pixel_one_mccolor
#define OP_A(f, i) 0x10000
#define OP_B(f, i) (0xffff - zb->blend_ ## i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_one_calpha
#define OP_A(f, i) 0x10000
#define OP_B(f, i) zb->blend_a
#include "store_pixel.h"

#define FNAME(name) store_pixel_one_mcalpha
#define OP_A(f, i) 0x10000
#define OP_B(f, i) (0xffff - zb->blend_a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_icolor_zero
#define OP_A(f, i) i
#define OP_B(f, i) 0
#include "store_pixel.h"

#define FNAME(name) store_pixel_icolor_one
#define OP_A(f, i) i
#define OP_B(f, i) 0x10000
#include "store_pixel.h"

#define FNAME(name) store_pixel_icolor_icolor
#define OP_A(f, i) i
#define OP_B(f, i) i
#include "store_pixel.h"

#define FNAME(name) store_pixel_icolor_micolor
#define OP_A(f, i) i
#define OP_B(f, i) (0xffff - i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_icolor_fcolor
#define OP_A(f, i) i
#define OP_B(f, i) f
#include "store_pixel.h"

#define FNAME(name) store_pixel_icolor_mfcolor
#define OP_A(f, i) i
#define OP_B(f, i) (0xffff - f)
#include "store_pixel.h"

#define FNAME(name) store_pixel_icolor_ialpha
#define OP_A(f, i) i
#define OP_B(f, i) a
#include "store_pixel.h"

#define FNAME(name) store_pixel_icolor_mialpha
#define OP_A(f, i) i
#define OP_B(f, i) (0xffff - a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_icolor_falpha
#define OP_A(f, i) i
#define OP_B(f, i) fa
#include "store_pixel.h"

#define FNAME(name) store_pixel_icolor_mfalpha
#define OP_A(f, i) i
#define OP_B(f, i) (0xffff - fa)
#include "store_pixel.h"

#define FNAME(name) store_pixel_icolor_ccolor
#define OP_A(f, i) i
#define OP_B(f, i) zb->blend_ ## i
#include "store_pixel.h"

#define FNAME(name) store_pixel_icolor_mccolor
#define OP_A(f, i) i
#define OP_B(f, i) (0xffff - zb->blend_ ## i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_icolor_calpha
#define OP_A(f, i) i
#define OP_B(f, i) zb->blend_a
#include "store_pixel.h"

#define FNAME(name) store_pixel_icolor_mcalpha
#define OP_A(f, i) i
#define OP_B(f, i) (0xffff - zb->blend_a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_micolor_zero
#define OP_A(f, i) (0xffff - i)
#define OP_B(f, i) 0
#include "store_pixel.h"

#define FNAME(name) store_pixel_micolor_one
#define OP_A(f, i) (0xffff - i)
#define OP_B(f, i) 0x10000
#include "store_pixel.h"

#define FNAME(name) store_pixel_micolor_icolor
#define OP_A(f, i) (0xffff - i)
#define OP_B(f, i) i
#include "store_pixel.h"

#define FNAME(name) store_pixel_micolor_micolor
#define OP_A(f, i) (0xffff - i)
#define OP_B(f, i) (0xffff - i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_micolor_fcolor
#define OP_A(f, i) (0xffff - i)
#define OP_B(f, i) f
#include "store_pixel.h"

#define FNAME(name) store_pixel_micolor_mfcolor
#define OP_A(f, i) (0xffff - i)
#define OP_B(f, i) (0xffff - f)
#include "store_pixel.h"

#define FNAME(name) store_pixel_micolor_ialpha
#define OP_A(f, i) (0xffff - i)
#define OP_B(f, i) a
#include "store_pixel.h"

#define FNAME(name) store_pixel_micolor_mialpha
#define OP_A(f, i) (0xffff - i)
#define OP_B(f, i) (0xffff - a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_micolor_falpha
#define OP_A(f, i) (0xffff - i)
#define OP_B(f, i) fa
#include "store_pixel.h"

#define FNAME(name) store_pixel_micolor_mfalpha
#define OP_A(f, i) (0xffff - i)
#define OP_B(f, i) (0xffff - fa)
#include "store_pixel.h"

#define FNAME(name) store_pixel_micolor_ccolor
#define OP_A(f, i) (0xffff - i)
#define OP_B(f, i) zb->blend_ ## i
#include "store_pixel.h"

#define FNAME(name) store_pixel_micolor_mccolor
#define OP_A(f, i) (0xffff - i)
#define OP_B(f, i) (0xffff - zb->blend_ ## i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_micolor_calpha
#define OP_A(f, i) (0xffff - i)
#define OP_B(f, i) zb->blend_a
#include "store_pixel.h"

#define FNAME(name) store_pixel_micolor_mcalpha
#define OP_A(f, i) (0xffff - i)
#define OP_B(f, i) (0xffff - zb->blend_a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_fcolor_zero
#define OP_A(f, i) f
#define OP_B(f, i) 0
#include "store_pixel.h"

#define FNAME(name) store_pixel_fcolor_one
#define OP_A(f, i) f
#define OP_B(f, i) 0x10000
#include "store_pixel.h"

#define FNAME(name) store_pixel_fcolor_icolor
#define OP_A(f, i) f
#define OP_B(f, i) i
#include "store_pixel.h"

#define FNAME(name) store_pixel_fcolor_micolor
#define OP_A(f, i) f
#define OP_B(f, i) (0xffff - i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_fcolor_fcolor
#define OP_A(f, i) f
#define OP_B(f, i) f
#include "store_pixel.h"

#define FNAME(name) store_pixel_fcolor_mfcolor
#define OP_A(f, i) f
#define OP_B(f, i) (0xffff - f)
#include "store_pixel.h"

#define FNAME(name) store_pixel_fcolor_ialpha
#define OP_A(f, i) f
#define OP_B(f, i) a
#include "store_pixel.h"

#define FNAME(name) store_pixel_fcolor_mialpha
#define OP_A(f, i) f
#define OP_B(f, i) (0xffff - a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_fcolor_falpha
#define OP_A(f, i) f
#define OP_B(f, i) fa
#include "store_pixel.h"

#define FNAME(name) store_pixel_fcolor_mfalpha
#define OP_A(f, i) f
#define OP_B(f, i) (0xffff - fa)
#include "store_pixel.h"

#define FNAME(name) store_pixel_fcolor_ccolor
#define OP_A(f, i) f
#define OP_B(f, i) zb->blend_ ## i
#include "store_pixel.h"

#define FNAME(name) store_pixel_fcolor_mccolor
#define OP_A(f, i) f
#define OP_B(f, i) (0xffff - zb->blend_ ## i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_fcolor_calpha
#define OP_A(f, i) f
#define OP_B(f, i) zb->blend_a
#include "store_pixel.h"

#define FNAME(name) store_pixel_fcolor_mcalpha
#define OP_A(f, i) f
#define OP_B(f, i) (0xffff - zb->blend_a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfcolor_zero
#define OP_A(f, i) (0xffff - f)
#define OP_B(f, i) 0
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfcolor_one
#define OP_A(f, i) (0xffff - f)
#define OP_B(f, i) 0x10000
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfcolor_icolor
#define OP_A(f, i) (0xffff - f)
#define OP_B(f, i) i
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfcolor_micolor
#define OP_A(f, i) (0xffff - f)
#define OP_B(f, i) (0xffff - i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfcolor_fcolor
#define OP_A(f, i) (0xffff - f)
#define OP_B(f, i) f
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfcolor_mfcolor
#define OP_A(f, i) (0xffff - f)
#define OP_B(f, i) (0xffff - f)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfcolor_ialpha
#define OP_A(f, i) (0xffff - f)
#define OP_B(f, i) a
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfcolor_mialpha
#define OP_A(f, i) (0xffff - f)
#define OP_B(f, i) (0xffff - a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfcolor_falpha
#define OP_A(f, i) (0xffff - f)
#define OP_B(f, i) fa
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfcolor_mfalpha
#define OP_A(f, i) (0xffff - f)
#define OP_B(f, i) (0xffff - fa)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfcolor_ccolor
#define OP_A(f, i) (0xffff - f)
#define OP_B(f, i) zb->blend_ ## i
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfcolor_mccolor
#define OP_A(f, i) (0xffff - f)
#define OP_B(f, i) (0xffff - zb->blend_ ## i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfcolor_calpha
#define OP_A(f, i) (0xffff - f)
#define OP_B(f, i) zb->blend_a
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfcolor_mcalpha
#define OP_A(f, i) (0xffff - f)
#define OP_B(f, i) (0xffff - zb->blend_a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_ialpha_zero
#define OP_A(f, i) a
#define OP_B(f, i) 0
#include "store_pixel.h"

#define FNAME(name) store_pixel_ialpha_one
#define OP_A(f, i) a
#define OP_B(f, i) 0x10000
#include "store_pixel.h"

#define FNAME(name) store_pixel_ialpha_icolor
#define OP_A(f, i) a
#define OP_B(f, i) i
#include "store_pixel.h"

#define FNAME(name) store_pixel_ialpha_micolor
#define OP_A(f, i) a
#define OP_B(f, i) (0xffff - i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_ialpha_fcolor
#define OP_A(f, i) a
#define OP_B(f, i) f
#include "store_pixel.h"

#define FNAME(name) store_pixel_ialpha_mfcolor
#define OP_A(f, i) a
#define OP_B(f, i) (0xffff - f)
#include "store_pixel.h"

#define FNAME(name) store_pixel_ialpha_ialpha
#define OP_A(f, i) a
#define OP_B(f, i) a
#include "store_pixel.h"

#define FNAME(name) store_pixel_ialpha_mialpha
#define OP_A(f, i) a
#define OP_B(f, i) (0xffff - a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_ialpha_falpha
#define OP_A(f, i) a
#define OP_B(f, i) fa
#include "store_pixel.h"

#define FNAME(name) store_pixel_ialpha_mfalpha
#define OP_A(f, i) a
#define OP_B(f, i) (0xffff - fa)
#include "store_pixel.h"

#define FNAME(name) store_pixel_ialpha_ccolor
#define OP_A(f, i) a
#define OP_B(f, i) zb->blend_ ## i
#include "store_pixel.h"

#define FNAME(name) store_pixel_ialpha_mccolor
#define OP_A(f, i) a
#define OP_B(f, i) (0xffff - zb->blend_ ## i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_ialpha_calpha
#define OP_A(f, i) a
#define OP_B(f, i) zb->blend_a
#include "store_pixel.h"

#define FNAME(name) store_pixel_ialpha_mcalpha
#define OP_A(f, i) a
#define OP_B(f, i) (0xffff - zb->blend_a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mialpha_zero
#define OP_A(f, i) (0xffff - a)
#define OP_B(f, i) 0
#include "store_pixel.h"

#define FNAME(name) store_pixel_mialpha_one
#define OP_A(f, i) (0xffff - a)
#define OP_B(f, i) 0x10000
#include "store_pixel.h"

#define FNAME(name) store_pixel_mialpha_icolor
#define OP_A(f, i) (0xffff - a)
#define OP_B(f, i) i
#include "store_pixel.h"

#define FNAME(name) store_pixel_mialpha_micolor
#define OP_A(f, i) (0xffff - a)
#define OP_B(f, i) (0xffff - i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mialpha_fcolor
#define OP_A(f, i) (0xffff - a)
#define OP_B(f, i) f
#include "store_pixel.h"

#define FNAME(name) store_pixel_mialpha_mfcolor
#define OP_A(f, i) (0xffff - a)
#define OP_B(f, i) (0xffff - f)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mialpha_ialpha
#define OP_A(f, i) (0xffff - a)
#define OP_B(f, i) a
#include "store_pixel.h"

#define FNAME(name) store_pixel_mialpha_mialpha
#define OP_A(f, i) (0xffff - a)
#define OP_B(f, i) (0xffff - a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mialpha_falpha
#define OP_A(f, i) (0xffff - a)
#define OP_B(f, i) fa
#include "store_pixel.h"

#define FNAME(name) store_pixel_mialpha_mfalpha
#define OP_A(f, i) (0xffff - a)
#define OP_B(f, i) (0xffff - fa)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mialpha_ccolor
#define OP_A(f, i) (0xffff - a)
#define OP_B(f, i) zb->blend_ ## i
#include "store_pixel.h"

#define FNAME(name) store_pixel_mialpha_mccolor
#define OP_A(f, i) (0xffff - a)
#define OP_B(f, i) (0xffff - zb->blend_ ## i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mialpha_calpha
#define OP_A(f, i) (0xffff - a)
#define OP_B(f, i) zb->blend_a
#include "store_pixel.h"

#define FNAME(name) store_pixel_mialpha_mcalpha
#define OP_A(f, i) (0xffff - a)
#define OP_B(f, i) (0xffff - zb->blend_a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_falpha_zero
#define OP_A(f, i) fa
#define OP_B(f, i) 0
#include "store_pixel.h"

#define FNAME(name) store_pixel_falpha_one
#define OP_A(f, i) fa
#define OP_B(f, i) 0x10000
#include "store_pixel.h"

#define FNAME(name) store_pixel_falpha_icolor
#define OP_A(f, i) fa
#define OP_B(f, i) i
#include "store_pixel.h"

#define FNAME(name) store_pixel_falpha_micolor
#define OP_A(f, i) fa
#define OP_B(f, i) (0xffff - i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_falpha_fcolor
#define OP_A(f, i) fa
#define OP_B(f, i) f
#include "store_pixel.h"

#define FNAME(name) store_pixel_falpha_mfcolor
#define OP_A(f, i) fa
#define OP_B(f, i) (0xffff - f)
#include "store_pixel.h"

#define FNAME(name) store_pixel_falpha_ialpha
#define OP_A(f, i) fa
#define OP_B(f, i) a
#include "store_pixel.h"

#define FNAME(name) store_pixel_falpha_mialpha
#define OP_A(f, i) fa
#define OP_B(f, i) (0xffff - a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_falpha_falpha
#define OP_A(f, i) fa
#define OP_B(f, i) fa
#include "store_pixel.h"

#define FNAME(name) store_pixel_falpha_mfalpha
#define OP_A(f, i) fa
#define OP_B(f, i) (0xffff - fa)
#include "store_pixel.h"

#define FNAME(name) store_pixel_falpha_ccolor
#define OP_A(f, i) fa
#define OP_B(f, i) zb->blend_ ## i
#include "store_pixel.h"

#define FNAME(name) store_pixel_falpha_mccolor
#define OP_A(f, i) fa
#define OP_B(f, i) (0xffff - zb->blend_ ## i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_falpha_calpha
#define OP_A(f, i) fa
#define OP_B(f, i) zb->blend_a
#include "store_pixel.h"

#define FNAME(name) store_pixel_falpha_mcalpha
#define OP_A(f, i) fa
#define OP_B(f, i) (0xffff - zb->blend_a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfalpha_zero
#define OP_A(f, i) (0xffff - fa)
#define OP_B(f, i) 0
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfalpha_one
#define OP_A(f, i) (0xffff - fa)
#define OP_B(f, i) 0x10000
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfalpha_icolor
#define OP_A(f, i) (0xffff - fa)
#define OP_B(f, i) i
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfalpha_micolor
#define OP_A(f, i) (0xffff - fa)
#define OP_B(f, i) (0xffff - i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfalpha_fcolor
#define OP_A(f, i) (0xffff - fa)
#define OP_B(f, i) f
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfalpha_mfcolor
#define OP_A(f, i) (0xffff - fa)
#define OP_B(f, i) (0xffff - f)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfalpha_ialpha
#define OP_A(f, i) (0xffff - fa)
#define OP_B(f, i) a
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfalpha_mialpha
#define OP_A(f, i) (0xffff - fa)
#define OP_B(f, i) (0xffff - a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfalpha_falpha
#define OP_A(f, i) (0xffff - fa)
#define OP_B(f, i) fa
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfalpha_mfalpha
#define OP_A(f, i) (0xffff - fa)
#define OP_B(f, i) (0xffff - fa)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfalpha_ccolor
#define OP_A(f, i) (0xffff - fa)
#define OP_B(f, i) zb->blend_ ## i
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfalpha_mccolor
#define OP_A(f, i) (0xffff - fa)
#define OP_B(f, i) (0xffff - zb->blend_ ## i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfalpha_calpha
#define OP_A(f, i) (0xffff - fa)
#define OP_B(f, i) zb->blend_a
#include "store_pixel.h"

#define FNAME(name) store_pixel_mfalpha_mcalpha
#define OP_A(f, i) (0xffff - fa)
#define OP_B(f, i) (0xffff - zb->blend_a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_ccolor_zero
#define OP_A(f, i) zb->blend_ ## i
#define OP_B(f, i) 0
#include "store_pixel.h"

#define FNAME(name) store_pixel_ccolor_one
#define OP_A(f, i) zb->blend_ ## i
#define OP_B(f, i) 0x10000
#include "store_pixel.h"

#define FNAME(name) store_pixel_ccolor_icolor
#define OP_A(f, i) zb->blend_ ## i
#define OP_B(f, i) i
#include "store_pixel.h"

#define FNAME(name) store_pixel_ccolor_micolor
#define OP_A(f, i) zb->blend_ ## i
#define OP_B(f, i) (0xffff - i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_ccolor_fcolor
#define OP_A(f, i) zb->blend_ ## i
#define OP_B(f, i) f
#include "store_pixel.h"

#define FNAME(name) store_pixel_ccolor_mfcolor
#define OP_A(f, i) zb->blend_ ## i
#define OP_B(f, i) (0xffff - f)
#include "store_pixel.h"

#define FNAME(name) store_pixel_ccolor_ialpha
#define OP_A(f, i) zb->blend_ ## i
#define OP_B(f, i) a
#include "store_pixel.h"

#define FNAME(name) store_pixel_ccolor_mialpha
#define OP_A(f, i) zb->blend_ ## i
#define OP_B(f, i) (0xffff - a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_ccolor_falpha
#define OP_A(f, i) zb->blend_ ## i
#define OP_B(f, i) fa
#include "store_pixel.h"

#define FNAME(name) store_pixel_ccolor_mfalpha
#define OP_A(f, i) zb->blend_ ## i
#define OP_B(f, i) (0xffff - fa)
#include "store_pixel.h"

#define FNAME(name) store_pixel_ccolor_ccolor
#define OP_A(f, i) zb->blend_ ## i
#define OP_B(f, i) zb->blend_ ## i
#include "store_pixel.h"

#define FNAME(name) store_pixel_ccolor_mccolor
#define OP_A(f, i) zb->blend_ ## i
#define OP_B(f, i) (0xffff - zb->blend_ ## i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_ccolor_calpha
#define OP_A(f, i) zb->blend_ ## i
#define OP_B(f, i) zb->blend_a
#include "store_pixel.h"

#define FNAME(name) store_pixel_ccolor_mcalpha
#define OP_A(f, i) zb->blend_ ## i
#define OP_B(f, i) (0xffff - zb->blend_a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mccolor_zero
#define OP_A(f, i) (0xffff - zb->blend_ ## i)
#define OP_B(f, i) 0
#include "store_pixel.h"

#define FNAME(name) store_pixel_mccolor_one
#define OP_A(f, i) (0xffff - zb->blend_ ## i)
#define OP_B(f, i) 0x10000
#include "store_pixel.h"

#define FNAME(name) store_pixel_mccolor_icolor
#define OP_A(f, i) (0xffff - zb->blend_ ## i)
#define OP_B(f, i) i
#include "store_pixel.h"

#define FNAME(name) store_pixel_mccolor_micolor
#define OP_A(f, i) (0xffff - zb->blend_ ## i)
#define OP_B(f, i) (0xffff - i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mccolor_fcolor
#define OP_A(f, i) (0xffff - zb->blend_ ## i)
#define OP_B(f, i) f
#include "store_pixel.h"

#define FNAME(name) store_pixel_mccolor_mfcolor
#define OP_A(f, i) (0xffff - zb->blend_ ## i)
#define OP_B(f, i) (0xffff - f)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mccolor_ialpha
#define OP_A(f, i) (0xffff - zb->blend_ ## i)
#define OP_B(f, i) a
#include "store_pixel.h"

#define FNAME(name) store_pixel_mccolor_mialpha
#define OP_A(f, i) (0xffff - zb->blend_ ## i)
#define OP_B(f, i) (0xffff - a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mccolor_falpha
#define OP_A(f, i) (0xffff - zb->blend_ ## i)
#define OP_B(f, i) fa
#include "store_pixel.h"

#define FNAME(name) store_pixel_mccolor_mfalpha
#define OP_A(f, i) (0xffff - zb->blend_ ## i)
#define OP_B(f, i) (0xffff - fa)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mccolor_ccolor
#define OP_A(f, i) (0xffff - zb->blend_ ## i)
#define OP_B(f, i) zb->blend_ ## i
#include "store_pixel.h"

#define FNAME(name) store_pixel_mccolor_mccolor
#define OP_A(f, i) (0xffff - zb->blend_ ## i)
#define OP_B(f, i) (0xffff - zb->blend_ ## i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mccolor_calpha
#define OP_A(f, i) (0xffff - zb->blend_ ## i)
#define OP_B(f, i) zb->blend_a
#include "store_pixel.h"

#define FNAME(name) store_pixel_mccolor_mcalpha
#define OP_A(f, i) (0xffff - zb->blend_ ## i)
#define OP_B(f, i) (0xffff - zb->blend_a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_calpha_zero
#define OP_A(f, i) zb->blend_a
#define OP_B(f, i) 0
#include "store_pixel.h"

#define FNAME(name) store_pixel_calpha_one
#define OP_A(f, i) zb->blend_a
#define OP_B(f, i) 0x10000
#include "store_pixel.h"

#define FNAME(name) store_pixel_calpha_icolor
#define OP_A(f, i) zb->blend_a
#define OP_B(f, i) i
#include "store_pixel.h"

#define FNAME(name) store_pixel_calpha_micolor
#define OP_A(f, i) zb->blend_a
#define OP_B(f, i) (0xffff - i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_calpha_fcolor
#define OP_A(f, i) zb->blend_a
#define OP_B(f, i) f
#include "store_pixel.h"

#define FNAME(name) store_pixel_calpha_mfcolor
#define OP_A(f, i) zb->blend_a
#define OP_B(f, i) (0xffff - f)
#include "store_pixel.h"

#define FNAME(name) store_pixel_calpha_ialpha
#define OP_A(f, i) zb->blend_a
#define OP_B(f, i) a
#include "store_pixel.h"

#define FNAME(name) store_pixel_calpha_mialpha
#define OP_A(f, i) zb->blend_a
#define OP_B(f, i) (0xffff - a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_calpha_falpha
#define OP_A(f, i) zb->blend_a
#define OP_B(f, i) fa
#include "store_pixel.h"

#define FNAME(name) store_pixel_calpha_mfalpha
#define OP_A(f, i) zb->blend_a
#define OP_B(f, i) (0xffff - fa)
#include "store_pixel.h"

#define FNAME(name) store_pixel_calpha_ccolor
#define OP_A(f, i) zb->blend_a
#define OP_B(f, i) zb->blend_ ## i
#include "store_pixel.h"

#define FNAME(name) store_pixel_calpha_mccolor
#define OP_A(f, i) zb->blend_a
#define OP_B(f, i) (0xffff - zb->blend_ ## i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_calpha_calpha
#define OP_A(f, i) zb->blend_a
#define OP_B(f, i) zb->blend_a
#include "store_pixel.h"

#define FNAME(name) store_pixel_calpha_mcalpha
#define OP_A(f, i) zb->blend_a
#define OP_B(f, i) (0xffff - zb->blend_a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mcalpha_zero
#define OP_A(f, i) (0xffff - zb->blend_a)
#define OP_B(f, i) 0
#include "store_pixel.h"

#define FNAME(name) store_pixel_mcalpha_one
#define OP_A(f, i) (0xffff - zb->blend_a)
#define OP_B(f, i) 0x10000
#include "store_pixel.h"

#define FNAME(name) store_pixel_mcalpha_icolor
#define OP_A(f, i) (0xffff - zb->blend_a)
#define OP_B(f, i) i
#include "store_pixel.h"

#define FNAME(name) store_pixel_mcalpha_micolor
#define OP_A(f, i) (0xffff - zb->blend_a)
#define OP_B(f, i) (0xffff - i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mcalpha_fcolor
#define OP_A(f, i) (0xffff - zb->blend_a)
#define OP_B(f, i) f
#include "store_pixel.h"

#define FNAME(name) store_pixel_mcalpha_mfcolor
#define OP_A(f, i) (0xffff - zb->blend_a)
#define OP_B(f, i) (0xffff - f)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mcalpha_ialpha
#define OP_A(f, i) (0xffff - zb->blend_a)
#define OP_B(f, i) a
#include "store_pixel.h"

#define FNAME(name) store_pixel_mcalpha_mialpha
#define OP_A(f, i) (0xffff - zb->blend_a)
#define OP_B(f, i) (0xffff - a)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mcalpha_falpha
#define OP_A(f, i) (0xffff - zb->blend_a)
#define OP_B(f, i) fa
#include "store_pixel.h"

#define FNAME(name) store_pixel_mcalpha_mfalpha
#define OP_A(f, i) (0xffff - zb->blend_a)
#define OP_B(f, i) (0xffff - fa)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mcalpha_ccolor
#define OP_A(f, i) (0xffff - zb->blend_a)
#define OP_B(f, i) zb->blend_ ## i
#include "store_pixel.h"

#define FNAME(name) store_pixel_mcalpha_mccolor
#define OP_A(f, i) (0xffff - zb->blend_a)
#define OP_B(f, i) (0xffff - zb->blend_ ## i)
#include "store_pixel.h"

#define FNAME(name) store_pixel_mcalpha_calpha
#define OP_A(f, i) (0xffff - zb->blend_a)
#define OP_B(f, i) zb->blend_a
#include "store_pixel.h"

#define FNAME(name) store_pixel_mcalpha_mcalpha
#define OP_A(f, i) (0xffff - zb->blend_a)
#define OP_B(f, i) (0xffff - zb->blend_a)
#include "store_pixel.h"

const ZB_storePixelFunc store_pixel_funcs[14][14] = {
  {
    store_pixel_zero_zero,
    store_pixel_zero_one,
    store_pixel_zero_icolor,
    store_pixel_zero_micolor,
    store_pixel_zero_fcolor,
    store_pixel_zero_mfcolor,
    store_pixel_zero_ialpha,
    store_pixel_zero_mialpha,
    store_pixel_zero_falpha,
    store_pixel_zero_mfalpha,
    store_pixel_zero_ccolor,
    store_pixel_zero_mccolor,
    store_pixel_zero_calpha,
    store_pixel_zero_mcalpha,
  },
  {
    store_pixel_one_zero,
    store_pixel_one_one,
    store_pixel_one_icolor,
    store_pixel_one_micolor,
    store_pixel_one_fcolor,
    store_pixel_one_mfcolor,
    store_pixel_one_ialpha,
    store_pixel_one_mialpha,
    store_pixel_one_falpha,
    store_pixel_one_mfalpha,
    store_pixel_one_ccolor,
    store_pixel_one_mccolor,
    store_pixel_one_calpha,
    store_pixel_one_mcalpha,
  },
  {
    store_pixel_icolor_zero,
    store_pixel_icolor_one,
    store_pixel_icolor_icolor,
    store_pixel_icolor_micolor,
    store_pixel_icolor_fcolor,
    store_pixel_icolor_mfcolor,
    store_pixel_icolor_ialpha,
    store_pixel_icolor_mialpha,
    store_pixel_icolor_falpha,
    store_pixel_icolor_mfalpha,
    store_pixel_icolor_ccolor,
    store_pixel_icolor_mccolor,
    store_pixel_icolor_calpha,
    store_pixel_icolor_mcalpha,
  },
  {
    store_pixel_micolor_zero,
    store_pixel_micolor_one,
    store_pixel_micolor_icolor,
    store_pixel_micolor_micolor,
    store_pixel_micolor_fcolor,
    store_pixel_micolor_mfcolor,
    store_pixel_micolor_ialpha,
    store_pixel_micolor_mialpha,
    store_pixel_micolor_falpha,
    store_pixel_micolor_mfalpha,
    store_pixel_micolor_ccolor,
    store_pixel_micolor_mccolor,
    store_pixel_micolor_calpha,
    store_pixel_micolor_mcalpha,
  },
  {
    store_pixel_fcolor_zero,
    store_pixel_fcolor_one,
    store_pixel_fcolor_icolor,
    store_pixel_fcolor_micolor,
    store_pixel_fcolor_fcolor,
    store_pixel_fcolor_mfcolor,
    store_pixel_fcolor_ialpha,
    store_pixel_fcolor_mialpha,
    store_pixel_fcolor_falpha,
    store_pixel_fcolor_mfalpha,
    store_pixel_fcolor_ccolor,
    store_pixel_fcolor_mccolor,
    store_pixel_fcolor_calpha,
    store_pixel_fcolor_mcalpha,
  },
  {
    store_pixel_mfcolor_zero,
    store_pixel_mfcolor_one,
    store_pixel_mfcolor_icolor,
    store_pixel_mfcolor_micolor,
    store_pixel_mfcolor_fcolor,
    store_pixel_mfcolor_mfcolor,
    store_pixel_mfcolor_ialpha,
    store_pixel_mfcolor_mialpha,
    store_pixel_mfcolor_falpha,
    store_pixel_mfcolor_mfalpha,
    store_pixel_mfcolor_ccolor,
    store_pixel_mfcolor_mccolor,
    store_pixel_mfcolor_calpha,
    store_pixel_mfcolor_mcalpha,
  },
  {
    store_pixel_ialpha_zero,
    store_pixel_ialpha_one,
    store_pixel_ialpha_icolor,
    store_pixel_ialpha_micolor,
    store_pixel_ialpha_fcolor,
    store_pixel_ialpha_mfcolor,
    store_pixel_ialpha_ialpha,
    store_pixel_ialpha_mialpha,
    store_pixel_ialpha_falpha,
    store_pixel_ialpha_mfalpha,
    store_pixel_ialpha_ccolor,
    store_pixel_ialpha_mccolor,
    store_pixel_ialpha_calpha,
    store_pixel_ialpha_mcalpha,
  },
  {
    store_pixel_mialpha_zero,
    store_pixel_mialpha_one,
    store_pixel_mialpha_icolor,
    store_pixel_mialpha_micolor,
    store_pixel_mialpha_fcolor,
    store_pixel_mialpha_mfcolor,
    store_pixel_mialpha_ialpha,
    store_pixel_mialpha_mialpha,
    store_pixel_mialpha_falpha,
    store_pixel_mialpha_mfalpha,
    store_pixel_mialpha_ccolor,
    store_pixel_mialpha_mccolor,
    store_pixel_mialpha_calpha,
    store_pixel_mialpha_mcalpha,
  },
  {
    store_pixel_falpha_zero,
    store_pixel_falpha_one,
    store_pixel_falpha_icolor,
    store_pixel_falpha_micolor,
    store_pixel_falpha_fcolor,
    store_pixel_falpha_mfcolor,
    store_pixel_falpha_ialpha,
    store_pixel_falpha_mialpha,
    store_pixel_falpha_falpha,
    store_pixel_falpha_mfalpha,
    store_pixel_falpha_ccolor,
    store_pixel_falpha_mccolor,
    store_pixel_falpha_calpha,
    store_pixel_falpha_mcalpha,
  },
  {
    store_pixel_mfalpha_zero,
    store_pixel_mfalpha_one,
    store_pixel_mfalpha_icolor,
    store_pixel_mfalpha_micolor,
    store_pixel_mfalpha_fcolor,
    store_pixel_mfalpha_mfcolor,
    store_pixel_mfalpha_ialpha,
    store_pixel_mfalpha_mialpha,
    store_pixel_mfalpha_falpha,
    store_pixel_mfalpha_mfalpha,
    store_pixel_mfalpha_ccolor,
    store_pixel_mfalpha_mccolor,
    store_pixel_mfalpha_calpha,
    store_pixel_mfalpha_mcalpha,
  },
  {
    store_pixel_ccolor_zero,
    store_pixel_ccolor_one,
    store_pixel_ccolor_icolor,
    store_pixel_ccolor_micolor,
    store_pixel_ccolor_fcolor,
    store_pixel_ccolor_mfcolor,
    store_pixel_ccolor_ialpha,
    store_pixel_ccolor_mialpha,
    store_pixel_ccolor_falpha,
    store_pixel_ccolor_mfalpha,
    store_pixel_ccolor_ccolor,
    store_pixel_ccolor_mccolor,
    store_pixel_ccolor_calpha,
    store_pixel_ccolor_mcalpha,
  },
  {
    store_pixel_mccolor_zero,
    store_pixel_mccolor_one,
    store_pixel_mccolor_icolor,
    store_pixel_mccolor_micolor,
    store_pixel_mccolor_fcolor,
    store_pixel_mccolor_mfcolor,
    store_pixel_mccolor_ialpha,
    store_pixel_mccolor_mialpha,
    store_pixel_mccolor_falpha,
    store_pixel_mccolor_mfalpha,
    store_pixel_mccolor_ccolor,
    store_pixel_mccolor_mccolor,
    store_pixel_mccolor_calpha,
    store_pixel_mccolor_mcalpha,
  },
  {
    store_pixel_calpha_zero,
    store_pixel_calpha_one,
    store_pixel_calpha_icolor,
    store_pixel_calpha_micolor,
    store_pixel_calpha_fcolor,
    store_pixel_calpha_mfcolor,
    store_pixel_calpha_ialpha,
    store_pixel_calpha_mialpha,
    store_pixel_calpha_falpha,
    store_pixel_calpha_mfalpha,
    store_pixel_calpha_ccolor,
    store_pixel_calpha_mccolor,
    store_pixel_calpha_calpha,
    store_pixel_calpha_mcalpha,
  },
  {
    store_pixel_mcalpha_zero,
    store_pixel_mcalpha_one,
    store_pixel_mcalpha_icolor,
    store_pixel_mcalpha_micolor,
    store_pixel_mcalpha_fcolor,
    store_pixel_mcalpha_mfcolor,
    store_pixel_mcalpha_ialpha,
    store_pixel_mcalpha_mialpha,
    store_pixel_mcalpha_falpha,
    store_pixel_mcalpha_mfalpha,
    store_pixel_mcalpha_ccolor,
    store_pixel_mcalpha_mccolor,
    store_pixel_mcalpha_calpha,
    store_pixel_mcalpha_mcalpha,
  },
};
