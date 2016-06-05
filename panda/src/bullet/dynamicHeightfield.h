/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dynamicHeightfield.h
 * @author wolfgangp
 * @date 2016-05-23
 */

#ifndef DYNAMIC_HEIGHTFIELD_H
#define DYNAMIC_HEIGHTFIELD_H

#include "pandabase.h"
#include "pointerTo.h"
#include "pandaNode.h"
#include "luse.h"
#include "pfmFile.h"
#include "shaderTerrainMesh.h"
#include "bulletHeightfieldShape.h"

/**
 * This class holds a heightfield and propagates changes to it to classes which
 * manage visualisation, collision, etc.
 */
class EXPCL_PANDABULLET DynamicHeightfield : public PfmFile {

PUBLISHED:
  INLINE DynamicHeightfield();
  INLINE ~DynamicHeightfield();

  void set_listener(ShaderTerrainMesh *stm);
  void set_listener(BulletHeightfieldShape *bhfs);

  INLINE void make_update_region(int lower_x, int upper_x, int lower_y, int upper_y);
  INLINE void make_update_region(float lower_x, float upper_x, float lower_y, float upper_y);
  INLINE void update();

  BLOCKING int pull_spot(const LPoint4f &delta, float xc, float yc, float xr, float yr, float exponent);
  void copy_sub_image(const PfmFile &copy, int xto, int yto,
                      int xfrom = 0, int yfrom = 0,
                      int x_size = -1, int y_size = -1);
  void add_sub_image(const PfmFile &copy, int xto, int yto,
                     int xfrom = 0, int yfrom = 0,
                     int x_size = -1, int y_size = -1,
                     float pixel_scale = 1.0);
  void mult_sub_image(const PfmFile &copy, int xto, int yto,
                      int xfrom = 0, int yfrom = 0,
                      int x_size = -1, int y_size = -1,
                      float pixel_scale = 1.0);
  void divide_sub_image(const PfmFile &copy, int xto, int yto,
                        int xfrom = 0, int yfrom = 0,
                        int x_size = -1, int y_size = -1,
                        float pixel_scale = 1.0);

private:
  PT(ShaderTerrainMesh) _stm_ptr;
  PT(BulletHeightfieldShape) _bhfs_ptr;
  LVector4i _region_corners;

};

#include "dynamicHeightfield.I"

#endif //DYNAMIC_HEIGHTFIELD_H
