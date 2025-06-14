/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaPview.h
 * @author drose
 * @date 2003-03-11
 */

#ifndef MAYAPVIEW_H
#define MAYAPVIEW_H

#include "pandatoolbase.h"
#include "pandaFramework.h"

#include "pre_maya_include.h"
#include <maya/MArgList.h>
#include <maya/MPxCommand.h>
#include <maya/MObject.h>
#include "post_maya_include.h"

/**
 * This class serves as a plug-in to Maya to allow viewing the current Maya
 * selection as it will be converted to Panda.
 */
class MayaPview : public MPxCommand {
public:
  MayaPview();
  virtual MStatus doIt(const MArgList &args);

  static void *creator();

private:
  bool convert(const NodePath &parent, bool animate);
};

EXPCL_MISC MStatus initializePlugin(MObject obj);
EXPCL_MISC MStatus uninitializePlugin(MObject obj);


#endif
