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

ConfigVariableEnum<SGIStorageType> sgi_storage_type
("sgi-storage-type", SST_rle,
 PRC_DESC("Use either 'rle' or 'verbatim' to indicate how SGI (*.rgb) "
          "files are written."));
ConfigVariableString sgi_imagename
("sgi-imagename", "",
 PRC_DESC("This string is written to the header of an SGI (*.rgb) file.  "
          "It seems to have documentation purposes only."));

// TGA supports RLE compression, as well as colormapping and/or
// grayscale images.  Set these true to enable these features, if
// possible, or false to disable them.  Some programs (like xv) have
// difficulty reading these advanced TGA files.
ConfigVariableBool tga_rle
("tga-rle", false,
 PRC_DESC("Set this true to enable RLE compression when writing TGA files."));

ConfigVariableBool tga_colormap
("tga-colormap", false,
 PRC_DESC("Set this true to write colormapped TGA files."));

ConfigVariableBool tga_grayscale
("tga-grayscale", false,
 PRC_DESC("Set this true to enable writing grayscale TGA files."));

ConfigVariableEnum<IMGHeaderType> img_header_type
("img-header-type", IHT_short,
 PRC_DESC("IMG format is just a sequential string of r, g, b bytes.  However, "
          "it may or may not include a \"header\" which consists of the xsize "
          "and the ysize of the image, either as shorts or as longs.  Specify "
          "that with this variable, either 'short', 'long', or 'none' for "
          "no header at all (in which case you should also set img-size)."));

ConfigVariableInt img_size
("img-size", 0,
 PRC_DESC("If an IMG file without a header is loaded (e.g. img-header-type "
          "is set to 'none', this specifies the fixed x y size of the image."));
ConfigVariableInt jpeg_quality
("jpeg-quality", 95,
 PRC_DESC("Set this to the quality percentage for writing JPEG files.  95 is "
          "the highest useful value (values greater than 95 do not lead to "
          "significantly better quality, but do lead to significantly greater "
          "size)."));


// These control the scaling that is automatically performed on a JPEG
// file for decompression.  You might specify to scale down by a
// fraction, e.g. 1/8, by specifying jpeg_scale_num = 1 and
// jpeg_scale_denom = 8.  This will reduce decompression time
// correspondingly.  Attempting to use this to scale up, or to scale
// by any fraction other than an even power of two, may not be
// supported.
ConfigVariableInt jpeg_scale_num
("jpeg-scale-num", 1);
ConfigVariableInt jpeg_scale_denom
("jpeg-scale-denom", 1);

ConfigVariableInt bmp_bpp
("bmp-bpp", 0,
 PRC_DESC("This controls how many bits per pixel are written out for BMP "
          "files.  If this is zero, the default, the number of bits per pixel "
          "is based on the image."));

ostream &
operator << (ostream &out, SGIStorageType sst) {
  switch (sst) {
  case SST_rle:
    return out << "rle";
  case SST_verbatim:
    return out << "verbatim";
  }

  return out << "**invalid SGIStorageType(" << (int)sst << ")**";
}

istream &
operator >> (istream &in, SGIStorageType &sst) {
  string word;
  in >> word;

  if (cmp_nocase(word, "rle") == 0) {
    sst = SST_rle;
  } else if (cmp_nocase(word, "verbatim") == 0) {
    sst = SST_verbatim;
  } else {
    pnmimage_img_cat->error()
      << "Invalid SGIStorageType: " << word << "\n";
    sst = SST_verbatim;
  }

  return in;
}

ostream &
operator << (ostream &out, IMGHeaderType iht) {
  switch (iht) {
  case IHT_none:
    return out << "none";
  case IHT_short:
    return out << "short";
  case IHT_long:
    return out << "long";
  }

  return out << "**invalid IMGHeaderType(" << (int)iht << ")**";
}

istream &
operator >> (istream &in, IMGHeaderType &iht) {
  string word;
  in >> word;

  if (cmp_nocase(word, "none") == 0) {
    iht = IHT_none;
  } else if (cmp_nocase(word, "short") == 0) {
    iht = IHT_short;
  } else if (cmp_nocase(word, "long") == 0) {
    iht = IHT_long;
  } else {
    pnmimage_img_cat->error()
      << "Invalid IMGHeaderType: " << word << "\n";
    iht = IHT_none;
  }

  return in;
}

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
