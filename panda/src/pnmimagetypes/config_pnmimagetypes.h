// Filename: config_pnmimagetypes.h
// Created by:  drose (17Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_PNMIMAGETYPES_H
#define CONFIG_PNMIMAGETYPES_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>

NotifyCategoryDecl(pnmimage_pnm, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_sgi, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_alias, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_radiance, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_tiff, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_yuv, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_img, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_soft, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_bmp, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_jpg, EXPCL_PANDA, EXPTP_PANDA);

extern int sgi_storage_type;
extern const string sgi_imagename;
extern const double radiance_gamma_correction;
extern const int radiance_brightness_adjustment;
extern const int yuv_xsize;
extern const int yuv_ysize;

extern const int jpeg_quality;
extern const int jpeg_scale_num;
extern const int jpeg_scale_denom;

enum IMGHeaderType {
  IHT_none,
  IHT_short,
  IHT_long,
};

extern IMGHeaderType img_header_type;
extern const int img_xsize;
extern const int img_ysize;

extern EXPCL_PANDA void init_libpnmimagetypes();

#endif
