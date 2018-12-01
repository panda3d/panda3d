/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_pnmimagetypes.cxx
 * @author drose
 * @date 2000-06-17
 */

#include "config_pnmimagetypes.h"
#include "pnmFileTypeSGI.h"
#include "pnmFileTypeTGA.h"
#include "pnmFileTypeIMG.h"
#include "pnmFileTypeSoftImage.h"
#include "pnmFileTypeBMP.h"
#include "pnmFileTypeJPG.h"
#include "pnmFileTypePNG.h"
#include "pnmFileTypePNM.h"
#include "pnmFileTypePfm.h"
#include "pnmFileTypeTIFF.h"
#include "pnmFileTypeEXR.h"
#include "pnmFileTypeStbImage.h"
#include "sgi.h"

#include "config_pnmimage.h"
#include "pnmFileTypeRegistry.h"
#include "string_utils.h"
#include "dconfig.h"
#include "pandaSystem.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_PNMIMAGETYPES)
  #error Buildsystem error: BUILDING_PANDA_PNMIMAGETYPES not defined
#endif

using std::istream;
using std::ostream;
using std::string;

Configure(config_pnmimagetypes);
NotifyCategoryDefName(pnmimage_sgi, "sgi", pnmimage_cat);
NotifyCategoryDefName(pnmimage_tga, "tga", pnmimage_cat);
NotifyCategoryDefName(pnmimage_img, "img", pnmimage_cat);
NotifyCategoryDefName(pnmimage_soft, "soft", pnmimage_cat);
NotifyCategoryDefName(pnmimage_bmp, "bmp", pnmimage_cat);
NotifyCategoryDefName(pnmimage_jpg, "jpg", pnmimage_cat);
NotifyCategoryDefName(pnmimage_png, "png", pnmimage_cat);
NotifyCategoryDefName(pnmimage_pnm, "pnm", pnmimage_cat);
NotifyCategoryDefName(pnmimage_tiff, "tiff", pnmimage_cat);
NotifyCategoryDefName(pnmimage_exr, "exr", pnmimage_cat);

ConfigVariableEnum<SGIStorageType> sgi_storage_type
("sgi-storage-type", SST_rle,
 PRC_DESC("Use either 'rle' or 'verbatim' to indicate how SGI (*.rgb) "
          "files are written."));
ConfigVariableString sgi_imagename
("sgi-imagename", "",
 PRC_DESC("This string is written to the header of an SGI (*.rgb) file.  "
          "It seems to have documentation purposes only."));

// TGA supports RLE compression, as well as colormapping andor grayscale
// images.  Set these true to enable these features, if possible, or false to
// disable them.  Some programs (like xv) have difficulty reading these
// advanced TGA files.
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

ConfigVariableInt png_compression_level
("png-compression-level", 6,
 PRC_DESC("Set this to the desired compression level for writing PNG images.  "
          "Valid values are 0 (no compression), or 1 (compression, best "
          "speed) to 9 (best compression).  Default is 6.  PNG compression is "
          "lossless."));

ConfigVariableBool png_palette
("png-palette", true,
 PRC_DESC("Set this true to allow writing palette-based PNG images when "
          "possible."));

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

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libpnmimagetypes() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  init_libpnmimage();

  PNMFileTypeRegistry *tr = PNMFileTypeRegistry::get_global_ptr();

#ifdef HAVE_SGI_RGB
  PNMFileTypeSGI::init_type();
  PNMFileTypeSGI::register_with_read_factory();
  tr->register_type(new PNMFileTypeSGI);
#endif

#ifdef HAVE_TGA
  PNMFileTypeTGA::init_type();
  PNMFileTypeTGA::register_with_read_factory();
  tr->register_type(new PNMFileTypeTGA);
#endif

#ifdef HAVE_IMG
  PNMFileTypeIMG::init_type();
  PNMFileTypeIMG::register_with_read_factory();
  tr->register_type(new PNMFileTypeIMG);
#endif

#ifdef HAVE_SOFTIMAGE_PIC
  PNMFileTypeSoftImage::init_type();
  PNMFileTypeSoftImage::register_with_read_factory();
  tr->register_type(new PNMFileTypeSoftImage);
#endif  // HAVE_SOFTIMAGE_PIC

#ifdef HAVE_BMP
  PNMFileTypeBMP::init_type();
  PNMFileTypeBMP::register_with_read_factory();
  tr->register_type(new PNMFileTypeBMP);
#endif

#ifdef HAVE_PNM
  PNMFileTypePNM::init_type();
  PNMFileTypePNM::register_with_read_factory();
  tr->register_type(new PNMFileTypePNM);
#endif

  PNMFileTypePfm::init_type();
  PNMFileTypePfm::register_with_read_factory();
  tr->register_type(new PNMFileTypePfm);

#ifdef HAVE_JPEG
  PNMFileTypeJPG::init_type();
  PNMFileTypeJPG::register_with_read_factory();
  tr->register_type(new PNMFileTypeJPG);
#endif

#ifdef HAVE_PNG
  PNMFileTypePNG::init_type();
  PNMFileTypePNG::register_with_read_factory();
  tr->register_type(new PNMFileTypePNG);
#endif

#ifdef HAVE_TIFF
  PNMFileTypeTIFF::init_type();
  PNMFileTypeTIFF::register_with_read_factory();
  tr->register_type(new PNMFileTypeTIFF);
#endif

#ifdef HAVE_OPENEXR
  PNMFileTypeEXR::init_type();
  PNMFileTypeEXR::register_with_read_factory();
  tr->register_type(new PNMFileTypeEXR);
#endif

#ifdef HAVE_STB_IMAGE
  PNMFileTypeStbImage::init_type();
  PNMFileTypeStbImage::register_with_read_factory();
  tr->register_type(new PNMFileTypeStbImage);
#endif

  // And register with the PandaSystem.
  PandaSystem *ps = PandaSystem::get_global_ptr();
  (void)ps; // Suppress unused variable warning

#ifdef HAVE_JPEG
  ps->add_system("libjpeg");
#endif
#ifdef HAVE_PNG
  ps->add_system("libpng");
#endif
#ifdef HAVE_TIFF
  ps->add_system("libtiff");
#endif
#ifdef HAVE_OPENEXR
  ps->add_system("openexr");
#endif
}
