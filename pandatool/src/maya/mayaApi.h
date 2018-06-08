/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaApi.h
 * @author drose
 * @date 2002-04-15
 */

#ifndef MAYAAPI_H
#define MAYAAPI_H

#include "pandatoolbase.h"
#include "distanceUnit.h"
#include "coordinateSystem.h"
#include "referenceCount.h"
#include "pointerTo.h"

class Filename;

/**
 * This class presents a wrapper around the global Maya interface.  While the
 * reference count is held, it keeps the Maya interface open, and closes the
 * interface when the object destructs.
 */
class MayaApi : public ReferenceCount {
protected:
  MayaApi(const std::string &program_name, bool view_license = false, bool revertdir = true);
  MayaApi(const MayaApi &copy);
  void operator = (const MayaApi &copy);

public:
  ~MayaApi();

  static PT(MayaApi) open_api(std::string program_name = "", bool view_license = false, bool revertdir = true);
  bool is_valid() const;

  bool read(const Filename &filename);
  bool write(const Filename &filename);
  bool clear();

  DistanceUnit get_units();
  void set_units(DistanceUnit unit);
  CoordinateSystem get_coordinate_system();

private:
  bool _is_valid;
  bool _plug_in;
  Filename _cwd;
  static MayaApi *_global_api;
};

#endif
