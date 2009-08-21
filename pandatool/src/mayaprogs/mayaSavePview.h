// Filename: mayaSavePview.h
// Created by:  drose (27Oct03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef MAYASAVEPVIEW_H
#define MAYASAVEPVIEW_H

// We don't want to include pre_maya_include.h here, since that would
// necessitate linking with Pandatool's libmaya.dll, which would in
// turn bring in a lot of stuff from panda that we don't really need.
// Instead, we'll just define the Maya symbols we require here.

// Maya will try to typedef bool unless this symbol is defined.
#ifndef _BOOL
#define _BOOL 1
#endif

#ifdef __MACH__
#undef _BOOL
#include "maya/OpenMayaMac.h"
#endif

// Even though we don't include any Panda headers, it's safe to
// include this one, since it only defines some macros that we need to
// make this program platform-independent.
#include "dtool_config.h"

#ifdef PHAVE_IOSTREAM
// This will ask Maya 5.0 or better to use the new <iostream> library
// instead of the old <iostream.h> library.
#define REQUIRE_IOSTREAM
#endif  // PHAVE_IOSTREAM

#include <maya/MArgList.h>
#include <maya/MPxCommand.h>
#include <maya/MObject.h>

////////////////////////////////////////////////////////////////////
//       Class : MayaSavePview
// Description : This class serves as a plug-in to Maya to save the
//               scene and view it using the external pview program,
//               rather than linking in any part of Panda to a Maya
//               plugin.
//
//               Since it does not link with any Panda code, and hence
//               is a very lean plugin, it is less likely than
//               MayaPview to cause interoperability problems within
//               Maya.  However, it does force a save-to-disk and a
//               spawning of a separate executable, including a
//               complete reloading of all of the Maya libraries, so
//               it is quite a bit slower to execute.  And the
//               potential for interactive control is substantially
//               reduced.
////////////////////////////////////////////////////////////////////
class MayaSavePview : public MPxCommand {
public:
  MayaSavePview();
  virtual MStatus doIt(const MArgList &args);

  static void *creator();
};

// Since we don't include any of the Panda headers (other than
// dtool_config.h), we have to define this macro ourselves, to tell
// Windows to export the following functions from the DLL.
#ifdef WIN32_VC
  #define EXPCL_MISC __declspec(dllexport)
#else
  #define EXPCL_MISC
#endif

EXPCL_MISC MStatus initializePlugin(MObject obj);
EXPCL_MISC MStatus uninitializePlugin(MObject obj);


#endif
