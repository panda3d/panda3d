// Filename: config_pnmimagetypes.cxx
// Created by:  drose (17Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "config_pnmimagetypes.h"
#include "pnmFileTypePNM.h"
#include "pnmFileTypeSGI.h"
#include "pnmFileTypeAlias.h"
#include "pnmFileTypeRadiance.h"
#include "pnmFileTypeTIFF.h"
#include "pnmFileTypeYUV.h"
#include "pnmFileTypeIMG.h"
#include "pnmFileTypeSoftImage.h"
#include "pnmFileTypeBMP.h"
#ifdef HAVE_JPEG
  #include "pnmFileTypeJPG.h"
#endif
#include "sgi.h"

#include <config_pnmimage.h>
#include <pnmFileTypeRegistry.h>
#include <string_utils.h>
#include <dconfig.h>

Configure(config_pnmimagetypes);
NotifyCategoryDef(pnmimage_pnm, pnmimage_cat);
NotifyCategoryDef(pnmimage_sgi, pnmimage_cat);
NotifyCategoryDef(pnmimage_alias, pnmimage_cat);
NotifyCategoryDef(pnmimage_radiance, pnmimage_cat);
NotifyCategoryDef(pnmimage_tiff, pnmimage_cat);
NotifyCategoryDef(pnmimage_yuv, pnmimage_cat);
NotifyCategoryDef(pnmimage_img, pnmimage_cat);
NotifyCategoryDef(pnmimage_soft, pnmimage_cat);
NotifyCategoryDef(pnmimage_bmp, pnmimage_cat);
NotifyCategoryDef(pnmimage_jpg, pnmimage_cat);

int sgi_storage_type = STORAGE_RLE;
const string sgi_imagename = config_pnmimagetypes.GetString("sgi-imagename", "");
const double radiance_gamma_correction = config_pnmimagetypes.GetDouble("radiance-gamma-correction", 2.2);
const int radiance_brightness_adjustment = config_pnmimagetypes.GetInt("radiance-brightness-adjustment", 0);

// YUV format doesn't include an image size specification, so the
// image size must be specified externally.  The defaults here are
// likely candidates, since this is the Abekas native size; the ysize
// is automatically adjusted down to account for a short file.
const int yuv_xsize = config_pnmimagetypes.GetInt("yuv-xsize", 720);
const int yuv_ysize = config_pnmimagetypes.GetInt("yuv-ysize", 486);

// IMG format is just a sequential string of r, g, b bytes.  However,
// it may or may not include a "header" which consists of the xsize
// and the ysize of the image, either as shorts or as longs.
IMGHeaderType img_header_type;
// The following are only used if header_type is 'none'.
const int img_xsize = config_pnmimagetypes.GetInt("img-xsize", 0);
const int img_ysize = config_pnmimagetypes.GetInt("img-ysize", 0);

ConfigureFn(config_pnmimagetypes) {
  PNMFileTypePNM::init_type();
  PNMFileTypeSGI::init_type();
  PNMFileTypeAlias::init_type();
  PNMFileTypeRadiance::init_type();
  PNMFileTypeTIFF::init_type();
  PNMFileTypeYUV::init_type();
  PNMFileTypeIMG::init_type();
  PNMFileTypeSoftImage::init_type();
  PNMFileTypeBMP::init_type();
#ifdef HAVE_JPEG
  PNMFileTypeJPG::init_type();
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

  PNMFileTypeRegistry *tr = PNMFileTypeRegistry::get_ptr();

  tr->register_type(new PNMFileTypePNM);
  tr->register_type(new PNMFileTypeSGI);
  tr->register_type(new PNMFileTypeAlias);
  tr->register_type(new PNMFileTypeRadiance);
  tr->register_type(new PNMFileTypeTIFF);
  tr->register_type(new PNMFileTypeYUV);
  tr->register_type(new PNMFileTypeIMG);
  tr->register_type(new PNMFileTypeSoftImage);
  tr->register_type(new PNMFileTypeBMP);
#ifdef HAVE_JPEG
  tr->register_type(new PNMFileTypeJPG);
#endif
}
