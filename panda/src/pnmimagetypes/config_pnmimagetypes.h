// Filename: config_pnmimagetypes.h
// Created by:  drose (17Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_PNMIMAGETYPES_H
#define CONFIG_PNMIMAGETYPES_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"

NotifyCategoryDecl(pnmimage_sgi, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_alias, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_tiff, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_tga, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_img, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_soft, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_bmp, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_jpg, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_jpg2000, EXPCL_PANDA, EXPTP_PANDA);

extern int sgi_storage_type;
extern const string sgi_imagename;
extern const bool tga_rle;
extern const bool tga_colormap;
extern const bool tga_grayscale;

extern const int jpeg_quality;
extern const int jpeg_scale_num;
extern const int jpeg_scale_denom;

extern const int bmp_bpp;

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
