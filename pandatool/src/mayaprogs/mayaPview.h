// Filename: mayaPview.h
// Created by:  drose (11Mar03)
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

#ifndef MAYAPVIEW_H
#define MAYAPVIEW_H

#include "pandatoolbase.h"
#include "pandaFramework.h"

#include "pre_maya_include.h"
#include <maya/MArgList.h>
#include <maya/MPxCommand.h>
#include <maya/MObject.h>
#include "post_maya_include.h"

////////////////////////////////////////////////////////////////////
//       Class : MayaPview
// Description : This class serves as a plug-in to Maya to allow
//               viewing the current Maya selection as it will be
//               converted to Panda.
////////////////////////////////////////////////////////////////////
class MayaPview : public MPxCommand {
public:
  MayaPview();
  virtual MStatus doIt(const MArgList &);

  static void *creator();

private:
  bool convert(const NodePath &parent);
};

EXPCL_MISC MStatus initializePlugin(MObject obj);
EXPCL_MISC MStatus uninitializePlugin(MObject obj);


#endif
