// Filename: eggToBam.h
// Created by:  drose (28Jun00)
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

#ifndef EGGTOBAM_H
#define EGGTOBAM_H

#include "pandatoolbase.h"

#include "eggToSomething.h"

////////////////////////////////////////////////////////////////////
//       Class : EggToBam
// Description :
////////////////////////////////////////////////////////////////////
class EggToBam : public EggToSomething {
public:
  EggToBam();

  void run();

protected:
  virtual bool handle_args(Args &args);

private:
  bool _has_egg_flatten;
  int _egg_flatten;
  bool _egg_suppress_hidden;
  bool _ls;
  bool _has_compression_quality;
  int _compression_quality;
  bool _compression_off;
};

#endif
