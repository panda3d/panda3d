// Filename: mayaApi.h
// Created by:  drose (15Apr02)
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

#ifndef MAYAAPI_H
#define MAYAAPI_H

#include "pandatoolbase.h"
#include "distanceUnit.h"
#include "coordinateSystem.h"
#include "referenceCount.h"
#include "pointerTo.h"

class Filename;

////////////////////////////////////////////////////////////////////
//       Class : MayaApi
// Description : This class presents a wrapper around the global
//               Maya interface.  While the reference count is held,
//               it keeps the Maya interface open, and closes the
//               interface when the object destructs.
////////////////////////////////////////////////////////////////////
class MayaApi : public ReferenceCount {
protected:
  MayaApi(const string &program_name);
  MayaApi(const MayaApi &copy);
  void operator = (const MayaApi &copy);

public:
  ~MayaApi();

  static PT(MayaApi) open_api(string program_name = "");
  bool is_valid() const;

  bool read(const Filename &filename);
  bool write(const Filename &filename);
  bool clear();

  DistanceUnit get_units();
  CoordinateSystem get_coordinate_system();

private:
  bool _is_valid;
  bool _plug_in;
  static MayaApi *_global_api;
};

#endif
