// Filename: config_pnmimagetypes.h
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

#ifndef CONFIG_PNMIMAGETYPES_H
#define CONFIG_PNMIMAGETYPES_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableInt.h"
#include "configVariableString.h"
#include "configVariableBool.h"
#include "configVariableEnum.h"
#include "sgi.h"

NotifyCategoryDecl(pnmimage_sgi, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_alias, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_tiff, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_tga, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_img, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_soft, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_bmp, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_jpg, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(pnmimage_png, EXPCL_PANDA, EXPTP_PANDA);

enum SGIStorageType {
  SST_rle = STORAGE_RLE,
  SST_verbatim = STORAGE_VERBATIM,
};

EXPCL_PANDA ostream &operator << (ostream &out, SGIStorageType sst);
EXPCL_PANDA istream &operator >> (istream &in, SGIStorageType &sst);

extern ConfigVariableEnum<SGIStorageType> sgi_storage_type;
extern ConfigVariableString sgi_imagename;
extern ConfigVariableBool tga_rle;
extern ConfigVariableBool tga_colormap;
extern ConfigVariableBool tga_grayscale;

extern ConfigVariableInt jpeg_quality;
extern ConfigVariableInt jpeg_scale_num;
extern ConfigVariableInt jpeg_scale_denom;

extern ConfigVariableInt bmp_bpp;

enum IMGHeaderType {
  IHT_none,
  IHT_short,
  IHT_long,
};

EXPCL_PANDA ostream &operator << (ostream &out, IMGHeaderType iht);
EXPCL_PANDA istream &operator >> (istream &in, IMGHeaderType &iht);

extern ConfigVariableEnum<IMGHeaderType> img_header_type;
extern ConfigVariableInt img_size;

extern EXPCL_PANDA void init_libpnmimagetypes();

#endif
