// Filename: config_pnmimagetypes.cxx
// Created by:  drose (17Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "config_pnmimagetypes.h"
#include "pnmFileTypeSGI.h"
#include "pnmFileTypeAlias.h"
#include "pnmFileTypeTGA.h"
#include "pnmFileTypeIMG.h"
#include "pnmFileTypeSoftImage.h"
#include "pnmFileTypeBMP.h"
#include "pnmFileTypeJPG.h"
#include "pnmFileTypePNG.h"
#include "pnmFileTypeTIFF.h"
#include "sgi.h"

#include "config_pnmimage.h"
#include "pnmFileTypeRegistry.h"
#include "string_utils.h"
#include "dconfig.h"

Configure(config_pnmimagetypes);
NotifyCategoryDefName(pnmimage_sgi, "sgi", pnmimage_cat);
NotifyCategoryDefName(pnmimage_alias, "alias", pnmimage_cat);
NotifyCategoryDefName(pnmimage_tga, "tga", pnmimage_cat);
NotifyCategoryDefName(pnmimage_img, "img", pnmimage_cat);
NotifyCategoryDefName(pnmimage_soft, "soft", pnmimage_cat);
NotifyCategoryDefName(pnmimage_bmp, "bmp", pnmimage_cat);
NotifyCategoryDefName(pnmimage_jpg, "jpg", pnmimage_cat);
NotifyCategoryDefName(pnmimage_png, "png", pnmimage_cat);
NotifyCategoryDefName(pnmimage_tiff, "tiff", pnmimage_cat);

int sgi_storage_type = STORAGE_RLE;
const string sgi_imagename = config_pnmimagetypes.GetString("sgi-imagename", "");

// TGA supports RLE compression, as well as colormapping and/or
// grayscale images.  Set these true to enable these features, if
// possible, or false to disable them.  Some programs (like xv) have
// difficulty reading these advanced TGA files.
const bool tga_rle = config_pnmimagetypes.GetBool("tga-rle", false);
const bool tga_colormap = config_pnmimagetypes.GetBool("tga-colormap", false);
const bool tga_grayscale = config_pnmimagetypes.GetBool("tga-grayscale", false);

// IMG format is just a sequential string of r, g, b bytes.  However,
// it may or may not include a "header" which consists of the xsize
// and the ysize of the image, either as shorts or as longs.
IMGHeaderType img_header_type;
// The following are only used if header_type is 'none'.
const int img_xsize = config_pnmimagetypes.GetInt("img-xsize", 0);
const int img_ysize = config_pnmimagetypes.GetInt("img-ysize", 0);

// Set this to the quality percentage for writing JPEG files.  95 is
// the highest useful value (values greater than 95 do not lead to
// significantly better quality, but do lead to significantly greater
// size).
const int jpeg_quality = config_pnmimagetypes.GetInt("jpeg-quality", 95);

// These control the scaling that is automatically performed on a JPEG
// file for decompression.  You might specify to scale down by a
// fraction, e.g. 1/8, by specifying jpeg_scale_num = 1 and
// jpeg_scale_denom = 8.  This will reduce decompression time
// correspondingly.  Attempting to use this to scale up, or to scale
// by any fraction other than an even power of two, may not be
// supported.
const int jpeg_scale_num = config_pnmimagetypes.GetInt("jpeg-scale-num", 1);
const int jpeg_scale_denom = config_pnmimagetypes.GetInt("jpeg-scale-denom", 1);

// This controls how many bits per pixel are written out for BMP
// files.  If this is zero, the default, the number of bits per pixel
// is based on the image.
const int bmp_bpp = config_pnmimagetypes.GetInt("bmp-bpp", 0);

ConfigureFn(config_pnmimagetypes) {
  init_libpnmimagetypes();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libpnmimagetypes
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libpnmimagetypes() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  init_libpnmimage();
  PNMFileTypeSGI::init_type();
  PNMFileTypeAlias::init_type();
  PNMFileTypeTGA::init_type();
  PNMFileTypeIMG::init_type();
  PNMFileTypeSoftImage::init_type();
  PNMFileTypeBMP::init_type();
#ifdef HAVE_JPEG
  PNMFileTypeJPG::init_type();
#endif
#ifdef HAVE_PNG
  PNMFileTypePNG::init_type();
#endif
#ifdef HAVE_TIFF
  PNMFileTypeTIFF::init_type();
#endif

  string sgi_storage_type_str =
    config_pnmimagetypes.GetString("sgi-storage-type", "rle");
  if (cmp_nocase(sgi_storage_type_str, "rle") == 0) {
    sgi_storage_type = STORAGE_RLE;
  } else if (cmp_nocase(sgi_storage_type_str, "verbatim") == 0) {
    sgi_storage_type = STORAGE_VERBATIM;
  } else {
    pnmimage_sgi_cat->error()
      << "Invalid sgi-storage-type: " << sgi_storage_type_str << "\n";
  }

  string img_header_type_str =
    config_pnmimagetypes.GetString("img-header-type", "long");
  if (cmp_nocase(img_header_type_str, "none") == 0) {
    img_header_type = IHT_none;
  } else if (cmp_nocase(img_header_type_str, "short") == 0) {
    img_header_type = IHT_short;
  } else if (cmp_nocase(img_header_type_str, "long") == 0) {
    img_header_type = IHT_long;
  } else {
    pnmimage_img_cat->error()
      << "Invalid img-header-type: " << img_header_type_str << "\n";
  }

  // Register each type with the PNMFileTypeRegistry.
  PNMFileTypeRegistry *tr = PNMFileTypeRegistry::get_ptr();

  tr->register_type(new PNMFileTypeSGI);
  tr->register_type(new PNMFileTypeAlias);
  tr->register_type(new PNMFileTypeTGA);
  tr->register_type(new PNMFileTypeIMG);
  tr->register_type(new PNMFileTypeSoftImage);
  tr->register_type(new PNMFileTypeBMP);
#ifdef HAVE_JPEG
  tr->register_type(new PNMFileTypeJPG);
#endif
#ifdef HAVE_PNG
  tr->register_type(new PNMFileTypePNG);
#endif
#ifdef HAVE_TIFF
  tr->register_type(new PNMFileTypeTIFF);
#endif

  // Also register with the Bam reader.
  PNMFileTypeSGI::register_with_read_factory();
  PNMFileTypeAlias::register_with_read_factory();
  PNMFileTypeTGA::register_with_read_factory();
  PNMFileTypeIMG::register_with_read_factory();
  PNMFileTypeSoftImage::register_with_read_factory();
  PNMFileTypeBMP::register_with_read_factory();
#ifdef HAVE_JPEG
  PNMFileTypeJPG::register_with_read_factory();
#endif
#ifdef HAVE_PNG
  PNMFileTypePNG::register_with_read_factory();
#endif
#ifdef HAVE_TIFF
  PNMFileTypeTIFF::register_with_read_factory();
#endif
}
